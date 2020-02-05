#include "GeoTileFetcher.h"
#include "GeoMapReply.h"

#include <QtMath>
#include <QDebug>
#include <QRegularExpression>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

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

GeoTileFetcher::GeoTileFetcher(int scaleFactor, bool enableLogging, const QString& customBasemapUrl, QGeoTiledMappingManagerEngine* parent)
      : QGeoTileFetcher(parent)
      , m_networkManager(new QNetworkAccessManager(this))
      , m_userAgent("Qt Location based application")
      , m_format("png")
      , m_replyFormat("png")
      , m_accessToken("")
      , m_enableLogging(enableLogging)
      , m_customBasemapUrl(customBasemapUrl)
{
    m_scaleFactor = qBound(1, scaleFactor, 2);
}

void GeoTileFetcher::setMapIds(const QVector<QString>& mapIds)
{
    m_mapIds = mapIds;
}

void GeoTileFetcher::setFormat(const QString& format)
{
    m_format = format;

    if (m_format == "png" || m_format == "png32" || m_format == "png64" || m_format == "png128" || m_format == "png256")
    {
        m_replyFormat = "png";
    }
    else if (m_format == "jpg70" || m_format == "jpg80" || m_format == "jpg90")
    {
        m_replyFormat = "jpg";
    }
    else if (m_enableLogging)
    {
        qWarning() << "GeoTileFetcher: Unknown map format " << m_format;
    }
}

void GeoTileFetcher::setAccessToken(const QString& accessToken)
{
    m_accessToken = accessToken;
}

QGeoTiledMapReply* GeoTileFetcher::getTileImage(const QGeoTileSpec& spec)
{
    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    if (m_customBasemapUrl.isEmpty())
    {
        request.setUrl(
            QUrl(QStringLiteral("https://api.tiles.mapbox.com/v4/")
                + ((spec.mapId() >= m_mapIds.size()) ? QStringLiteral("mapbox.streets") : m_mapIds[spec.mapId() - 1])
                + QLatin1Char('/')
                + QString::number(spec.zoom())
                + QLatin1Char('/')
                + QString::number(spec.x())
                + QLatin1Char('/')
                + QString::number(spec.y())
                + ((m_scaleFactor > 1) ? (QLatin1Char('@') + QString::number(m_scaleFactor) + QLatin1String("x."))
                    : QLatin1String("."))
                + m_format
                + QLatin1Char('?')
                + QStringLiteral("access_token=")
                + m_accessToken));
    }
    else
    {
        const QString x = QString::number(spec.x());
        const QString y = QString::number(spec.y());
        const QString z = QString::number(spec.zoom());
        QString q, r, bbox, invY, wmsVersion;
        QStringList subdomains;

        auto basemapUrl = m_customBasemapUrl;
        basemapUrl = basemapUrl.replace("{x}", x);
        basemapUrl = basemapUrl.replace("{y}", y);
        basemapUrl = basemapUrl.replace("{z}", z);
        basemapUrl = basemapUrl.replace("{s}", "{sabc}");

        if (basemapUrl.contains("{-y}"))
        {
            invY = QString::number((1 << spec.zoom()) - spec.y() - 1);
            basemapUrl = basemapUrl.replace("{-y}", invY);
        }

        if (basemapUrl.contains("{q}"))
        {
            q = tileToQuad(spec);
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
            bbox = bbox4326ToString(spec, wmsVersion.toDouble() < 1.3);
            basemapUrl = basemapUrl.replace("{bbox4326}", bbox);
        }
        else if (basemapUrl.contains("{bbox4326_lonlat}"))
        {
            bbox = bbox4326ToString(spec, true);
            basemapUrl = basemapUrl.replace("{bbox4326_lonlat}", bbox);
        }
        else if (basemapUrl.contains("{bbox4326_latlon}"))
        {
            bbox = bbox4326ToString(spec, false);
            basemapUrl = basemapUrl.replace("{bbox4326_latlon}", bbox);
        }
        else if (basemapUrl.contains("{bbox3857}"))
        {
            bbox = bbox3857ToString(spec);
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
                subdomains.push_back(replaceSubdomain(spec, value));
                basemapUrl.replace(startIndex, count, subdomains.back());
                startIndex = basemapUrl.indexOf(QLatin1String("{s"));
            }
            else
            {
                if (m_enableLogging)
                {
                    qCritical() << "Basemap custom URL invalid {";
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
            qInfo() << "Basemap tile requested" << urlDetails;
        }

        request.setUrl(QUrl(basemapUrl));
    }

    return new GeoMapReply(m_networkManager->get(request), spec, m_replyFormat, m_enableLogging);
}