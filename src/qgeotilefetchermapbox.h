// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOTILEFETCHERMAPBOX_H
#define QGEOTILEFETCHERMAPBOX_H

#include <QHash>
#include <QList>
#include <qlist.h>
#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QUrlQuery>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngine;
class QNetworkAccessManager;

class QGeoTileFetcherMapbox : public QGeoTileFetcher
{
    Q_OBJECT

public:
    // The list of map style names:
    static constexpr const char* PIX4D_STREETS = "streets";
    static constexpr const char* PIX4D_SATELLITE = "satellite";
    static constexpr const char* PIX4D_CUSTOM = "custom";
    static constexpr const char* PIX4D_NONE = "none";

public:
    QGeoTileFetcherMapbox(int scaleFactor, bool enableLogging, const QString& customBasemapUrl, QGeoTiledMappingManagerEngine *parent);

    void setUserAgent(const QByteArray &userAgent);
    void setMapIds(const QList<QString> &mapIds);
    void setFormat(const QString &format);
    void setAdditionalParameters(const QVariantMap& parameters);

    // Highest zoom level at which the tile server actually serves tiles.
    // Requests above this level get their URL transparently remapped to the
    // corresponding ancestor at this zoom, so the cache fills with usable
    // tiles instead of 404s and QGeoFileTileCacheMapbox::get() can stretch
    // them. -1 (the default) disables remapping.
    void setMaximumZoomLevel(int maxZoom);

private Q_SLOTS:
    // Forwards the bytes of a remapped reply to the engine under the
    // ancestor's spec (the URL we actually hit) rather than the high-zoom
    // spec the reply nominally represents.
    void onAncestorReady(const QGeoTileSpec &ancestor, const QByteArray &bytes, const QString &format);

private:
    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) override;

    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_format;
    QString m_replyFormat;
    QList<QString> m_mapIds;
    int m_scaleFactor;
    bool m_enableLogging{false};
    QString m_customBasemapUrl;
    QUrlQuery m_query;
    int m_maximumZoomLevel{-1};

    // Per-ancestor list of high-zoom specs that piggy-back on the same
    // ancestor fetch. The first request for a given ancestor creates the
    // network reply (the "leader") and clears m_requested for itself via
    // the reply's own finished path; the siblings parked here are unblocked
    // through a synthetic empty-payload tileFinished() emission when the
    // leader settles. Prevents 16-100 duplicate fetches per viewport change.
    QHash<QGeoTileSpec, QList<QGeoTileSpec>> m_pendingHighZoomForAncestor;
};

QT_END_NAMESPACE

#endif // QGEOTILEFETCHERMAPBOX_H
