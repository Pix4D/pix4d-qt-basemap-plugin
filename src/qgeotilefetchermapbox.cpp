// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeotilefetchermapbox.h"
#include "qgeomapreplymapbox.h"
#include "qmapboxcommon.h"

#include <QtMath>
#include <QRegularExpression>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QDebug>

namespace
{
    QString tileToQuad(const QGeoTileSpec& spec)
    {
        QString quad;
        const int x = spec.x();
        const int y = spec.y();
        for (int i = spec.zoom(); i > 0; i--)
        {
            int digit = 0;
            int mask = 1 << (i - 1);
            if ((x & mask) != 0)
            {
                digit += 1;
            }
            if ((y & mask) != 0)
            {
                digit += 2;
            }
            quad = quad + QString::number(digit);
        }
        return quad;
    }

    QString replaceSubdomain(const QGeoTileSpec& spec, const QString& subdomain)
    {
        if (!subdomain.isEmpty())
        {
            const int hash = spec.x() + spec.y();
            return subdomain[hash % subdomain.length()];
        }
        return subdomain;
    }

    struct BboxCoordinates
    {
        BboxCoordinates(const QGeoTileSpec& spec)
        {
            auto calculateLongitude = [](double x)
            {
                return 180.0 * (x * 2.0 - 1.0);
            };
            auto calculateLatitude = [](double y)
            {
                return 180.0 / M_PI * (2.0 * atan(exp((1.0 - 2.0 * y) * M_PI)) - M_PI / 2.0);
            };

            const double n = 1 << spec.zoom();
            SWLatitude = calculateLatitude((spec.y() + 1) / n);
            SWLongitude = calculateLongitude(spec.x() / n);
            NELatitude = calculateLatitude(spec.y() / n);
            NELongitude = calculateLongitude((spec.x() + 1) / n);
        }

        double SWLatitude{0.0};
        double SWLongitude{0.0};
        double NELatitude{0.0};
        double NELongitude{0.0};
    };

    QString bbox3857ToString(const QGeoTileSpec& spec)
    {
        // Convert from 4326 -degrees lat,lng- to 3857 -in meters-
        // http://www.ogp.org.uk/pubs/373-07-2.pdf
        auto latlonTo3857 = [](double lat, double lng)
        {
            const double a = 6378137.0; // radius of sphere
            return std::pair<double, double>
            (
                // Easting and northing in meters
                a * lng * M_PI / 180.0,
                a * std::log(std::tan(M_PI / 4.0 + (lat / 2.0) * M_PI / 180.0))
            );
        };

        BboxCoordinates coords(spec);
        const auto SW3857 = latlonTo3857(coords.SWLatitude, coords.SWLongitude);
        const auto NE3857 = latlonTo3857(coords.NELatitude, coords.NELongitude);
        const int precision = 12;
        return QString("%1,%2,%3,%4")
            .arg(SW3857.first, 0, 'f', precision)
            .arg(SW3857.second, 0, 'f', precision)
            .arg(NE3857.first, 0, 'f', precision)
            .arg(NE3857.second, 0, 'f', precision);
    }

    QString bbox4326ToString(const QGeoTileSpec& spec, bool longitudeFirst)
    {
        BboxCoordinates coords(spec);
        const int precision = 12;
        return QString("%1,%2,%3,%4")
            .arg(longitudeFirst ? coords.SWLongitude : coords.SWLatitude, 0, 'f', precision)
            .arg(longitudeFirst ? coords.SWLatitude : coords.SWLongitude, 0, 'f', precision)
            .arg(longitudeFirst ? coords.NELongitude : coords.NELatitude, 0, 'f', precision)
            .arg(longitudeFirst ? coords.NELatitude : coords.NELongitude, 0, 'f', precision);
    }
}


QT_BEGIN_NAMESPACE

QGeoTileFetcherMapbox::QGeoTileFetcherMapbox(int scaleFactor, bool enableLogging, const QString& customBasemapUrl, QGeoTiledMappingManagerEngine *parent)
:   QGeoTileFetcher(parent), m_networkManager(new QNetworkAccessManager(this)),
    m_userAgent(mapboxDefaultUserAgent),
    m_format("png"),
    m_replyFormat("png"),
    m_enableLogging(enableLogging),
    m_customBasemapUrl(customBasemapUrl)
{
    m_scaleFactor = qBound(1, scaleFactor, 2);
}

void QGeoTileFetcherMapbox::setUserAgent(const QByteArray &userAgent)
{
    m_userAgent = userAgent;
}

void QGeoTileFetcherMapbox::setMapIds(const QList<QString> &mapIds)
{
    m_mapIds = mapIds;
}

void QGeoTileFetcherMapbox::setMaximumZoomLevel(int maxZoom)
{
    m_maximumZoomLevel = maxZoom;
}

void QGeoTileFetcherMapbox::setFormat(const QString &format)
{
    m_format = format;

    if (m_format == "png" || m_format == "png32" || m_format == "png64" || m_format == "png128" || m_format == "png256")
        m_replyFormat = "png";
    else if (m_format == "jpg70" || m_format == "jpg80" || m_format == "jpg90")
        m_replyFormat = "jpg";
    else
        qWarning() << "QGeoTileFetcher: Unknown map format " << m_format;
}

void QGeoTileFetcherMapbox::setAdditionalParameters(const QVariantMap& parameters)
{
    std::for_each(parameters.constKeyValueBegin(), parameters.constKeyValueEnd(), [this](const auto& pair){
        if (std::none_of(NON_QUERY_PARAMETER_KEYS.cbegin(), NON_QUERY_PARAMETER_KEYS.cend(), [this, pair](const QString& key){
                return pair.first == key;
            }))
        {
            m_query.addQueryItem(pair.first, pair.second.toString());
        }
    });
}

void QGeoTileFetcherMapbox::onAncestorReady(const QGeoTileSpec &ancestor,
                                            const QByteArray &bytes,
                                            const QString &format)
{
    // Re-emit the fetcher's tileFinished signal with the *ancestor* spec so
    // that QGeoTiledMappingManagerEngine::engineTileFinished() inserts the
    // bytes into the tile cache under the ancestor's filename, where
    // QGeoFileTileCacheMapbox::get() will find them when stretching for
    // higher-zoom requests in the same area.
    emit tileFinished(ancestor, bytes, format);
}

QGeoTiledMapReply *QGeoTileFetcherMapbox::getTileImage(const QGeoTileSpec &spec)
{
    // If the request is above the API maximum zoom, fetch the maxZoom ancestor
    // URL instead. The reply still represents `spec` from Qt's point of view,
    // but the bytes (delivered via ancestorReady) get cached under the
    // ancestor's spec so QGeoFileTileCacheMapbox::get() can substitute them.
    QGeoTileSpec urlSpec = spec;
    bool remap = false;
    if (m_maximumZoomLevel > 0 && spec.zoom() > m_maximumZoomLevel)
    {
        const int delta = spec.zoom() - m_maximumZoomLevel;
        const int denom = 1 << delta;
        urlSpec = QGeoTileSpec(spec.plugin(),
                               spec.mapId(),
                               m_maximumZoomLevel,
                               spec.x() / denom,
                               spec.y() / denom,
                               spec.version());
        remap = true;
    }

    // Deduplicate: if some other high-zoom request is already fetching this
    // ancestor, park `spec` as a follower and wait for the leader to settle.
    // Without this, a 4x4-zoom-level overzoom would fire ~64 identical URL
    // requests per viewport change.
    if (remap && m_pendingHighZoomForAncestor.contains(urlSpec))
    {
        m_pendingHighZoomForAncestor[urlSpec].append(spec);
        return nullptr;
    }

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl tileUrl;
    const QString x = QString::number(urlSpec.x());
    const QString y = QString::number(urlSpec.y());
    const QString z = QString::number(urlSpec.zoom());
    QString q, r, bbox, invY, wmsVersion;
    QStringList subdomains;

    QString basemapUrl;
    const auto mapId = urlSpec.mapId() < m_mapIds.size() ? m_mapIds[urlSpec.mapId()] : "";
    if (mapId == PIX4D_SATELLITE)
    {
        basemapUrl = MAPTILER_SATELLITE_URL;
    }
    else if (mapId == PIX4D_STREETS)
    {
        basemapUrl = MAPTILER_STREETS_URL;
    }
    else if ((mapId == PIX4D_CUSTOM) && !m_customBasemapUrl.isEmpty())
    {
        basemapUrl = m_customBasemapUrl;
    }
    else
    {
        if (m_enableLogging)
        {
            qInfo() << "QGeoTileFetcher: The selected basemap is NONE or the basemap URL is empty. Selected basemap type is " << mapId;
        }
        return nullptr;
    }

    basemapUrl = basemapUrl.replace("{x}", x);
    basemapUrl = basemapUrl.replace("{y}", y);
    basemapUrl = basemapUrl.replace("{z}", z);
    basemapUrl = basemapUrl.replace("{s}", "{sabc}");

    if (basemapUrl.contains("{-y}"))
    {
        invY = QString::number((1 << urlSpec.zoom()) - urlSpec.y() - 1);
        basemapUrl = basemapUrl.replace("{-y}", invY);
    }

    if (basemapUrl.contains("{q}"))
    {
        q = tileToQuad(urlSpec);
        basemapUrl = basemapUrl.replace("{q}", q);
    }

    if (basemapUrl.contains("{r}"))
    {
        r = m_scaleFactor > 1 ? QLatin1Char('@') + QString::number(m_scaleFactor) + QLatin1String("x") : "";
        basemapUrl = basemapUrl.replace("{r}", r);
    }

    if (basemapUrl.contains("{bbox4326}"))
    {
        // Version 1.3 and higher of WMS urls uses latitude first
        const int versionIndex = basemapUrl.toLower().indexOf("version=");
        wmsVersion = versionIndex >= 0 ? basemapUrl.mid(versionIndex + 8, 3) : "0.0";
        bbox = bbox4326ToString(urlSpec, wmsVersion.toDouble() < 1.3);
        basemapUrl = basemapUrl.replace("{bbox4326}", bbox);
    }
    else if (basemapUrl.contains("{bbox4326_lonlat}"))
    {
        bbox = bbox4326ToString(urlSpec, true);
        basemapUrl = basemapUrl.replace("{bbox4326_lonlat}", bbox);
    }
    else if (basemapUrl.contains("{bbox4326_latlon}"))
    {
        bbox = bbox4326ToString(urlSpec, false);
        basemapUrl = basemapUrl.replace("{bbox4326_latlon}", bbox);
    }
    else if (basemapUrl.contains("{bbox3857}"))
    {
        bbox = bbox3857ToString(urlSpec);
        basemapUrl = basemapUrl.replace("{bbox3857}", bbox);
    }

    int startIndex = basemapUrl.indexOf(QLatin1String("{s"));
    while (startIndex != -1)
    {
        const int endIndex = basemapUrl.indexOf(QLatin1Char('}'), startIndex);
        if (endIndex != -1)
        {
            const int count = endIndex - startIndex + 1;
            const auto value = basemapUrl.mid(startIndex + 2, std::max(count - 3, 0));
            subdomains.push_back(replaceSubdomain(urlSpec, value));
            basemapUrl.replace(startIndex, count, subdomains.back());
            startIndex = basemapUrl.indexOf(QLatin1String("{s"));
        }
        else
        {
            if (m_enableLogging)
            {
                qCritical() << "QGeoTileFetcher: Basemap custom URL invalid {";
            }
            break;
        }
    }

    if (m_enableLogging)
    {
        const QString provider = QUrl(basemapUrl).host();
        QString urlDetails = provider +
            " {x}=" + x +
            " {y}=" + y +
            " {z}=" + z +
            (!q.isEmpty() ? " {q}=" + q : "") +
            (!r.isEmpty() ? " {r}=" + r : "") +
            (!invY.isEmpty() ? " {-y}=" + invY : "") +
            (!subdomains.isEmpty() ? " {s}=" + subdomains.join(", ") : "") +
            (!bbox.isEmpty() ? " {bbox}=" + bbox : "") +
            (!wmsVersion.isEmpty() ? " with version " + wmsVersion : "");
        qInfo() << "QGeoTileFetcher: Basemap tile requested" << urlDetails;
    }

    tileUrl = QUrl(basemapUrl);
    
    if (!m_query.isEmpty())
        tileUrl.setQuery(m_query);

    request.setUrl(tileUrl);
    auto *reply = new QGeoMapReplyMapbox(m_networkManager, request, spec, m_replyFormat, m_enableLogging);

    if (remap)
    {
        reply->setAncestorRemap(urlSpec);
        connect(reply, &QGeoMapReplyMapbox::ancestorReady,
                this, &QGeoTileFetcherMapbox::onAncestorReady);

        // Mark the ancestor in flight; the bucket also holds any followers.
        m_pendingHighZoomForAncestor.insert(urlSpec, QList<QGeoTileSpec>());

        // When the leader reply finishes, wake every parked follower so its
        // high-zoom spec gets cleared from QGeoTileRequestManager::m_requested
        // and the next render cycle picks up the freshly-cached ancestor via
        // QGeoFileTileCacheMapbox::get()'s substitution path.
        const QGeoTileSpec ancestor = urlSpec;
        connect(reply, &QGeoTiledMapReply::finished, this,
                [this, ancestor, reply]() {
                    const auto followers = m_pendingHighZoomForAncestor.take(ancestor);
                    if (followers.isEmpty())
                        return;
                    if (reply->error() == QGeoTiledMapReply::NoError)
                    {
                        const QString format = reply->mapImageFormat();
                        for (const auto &s : followers)
                            emit tileFinished(s, QByteArray(), format);
                    }
                    else
                    {
                        const QString err = reply->errorString();
                        for (const auto &s : followers)
                            emit tileError(s, err);
                    }
                });
        // Safety net: if the leader is destroyed without firing finished()
        // (cancel paths), at least drop the bookkeeping so a later sibling
        // request can issue its own leader instead of waiting forever.
        connect(reply, &QObject::destroyed, this, [this, ancestor]() {
            m_pendingHighZoomForAncestor.remove(ancestor);
        });
    }

    return reply;
}

QT_END_NAMESPACE
