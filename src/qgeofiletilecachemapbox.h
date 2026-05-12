// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOFILETILECACHEMAPBOX_H
#define QGEOFILETILECACHEMAPBOX_H

#include <QtLocation/private/qgeofiletilecache_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QMap>

QT_BEGIN_NAMESPACE

class QGeoFileTileCacheMapbox : public QGeoFileTileCache
{
    Q_OBJECT
public:
    QGeoFileTileCacheMapbox(const QList<QGeoMapType> &mapTypes, int scaleFactor, bool enableLogging, const QString &directory = QString(), QObject *parent = nullptr);
    ~QGeoFileTileCacheMapbox();

    // Highest zoom level at which the tile server actually serves tiles.
    // get() substitutes an ancestor tile for any spec past this level so the
    // basemap stays visible while overzooming.
    void setMaximumZoomLevel(int maxZoom);

    QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec) override;

protected:
    // Treat empty payloads as bogus. QGeoTileFetcherMapbox emits an empty
    // tileFinished() for the original high-zoom spec after a remapped
    // ancestor download succeeds, just to clear m_requested in the request
    // manager. Without this override the empty payload would also create a
    // zero-byte file on disk for every overzoomed tile.
    bool isTileBogus(const QByteArray &bytes) const override;
    QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const override;
    QGeoTileSpec filenameToTileSpec(const QString &filename) const override;

    bool m_enableLogging{false};
    bool m_hasCacheDirectory{false};
    QList<QGeoMapType> m_mapTypes;
    QMap<QString, int> m_mapNameToId;
    int m_scaleFactor;
    int m_maximumZoomLevel{-1};
};

QT_END_NAMESPACE

#endif // QGEOFILETILECACHEMAPBOX_H
