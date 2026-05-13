// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOTILEDMAPPINGMANAGERENGINEMAPBOX_H
#define QGEOTILEDMAPPINGMANAGERENGINEMAPBOX_H

#include <QtLocation/QGeoServiceProvider>

#include <QHash>
#include <QSet>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>
#include <QtLocation/private/qgeotilespec_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineMapbox : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

public:
    QGeoTiledMappingManagerEngineMapbox(const QVariantMap &parameters,
                                        QGeoServiceProvider::Error *error, QString *errorString);
    ~QGeoTiledMappingManagerEngineMapbox();

    QGeoMap *createMap() override;

    // Intercepts tile requests above maximum_zoom_level and reroutes them to
    // the corresponding ancestor tile at maximum_zoom_level, coalescing every
    // high-zoom sibling that shares the same ancestor into a single network
    // request. The high-zoom specs themselves are tracked in m_overzoomTiles
    // so engineTileFinished()/engineTileError() can fan completion back out
    // to each requester via QGeoTileRequestManager::tileFetched().
    void updateTileRequests(QGeoTiledMap *map,
                            const QSet<QGeoTileSpec> &tilesAdded,
                            const QSet<QGeoTileSpec> &tilesRemoved) override;

protected Q_SLOTS:
    void engineTileFinished(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format) override;
    void engineTileError(const QGeoTileSpec &spec, const QString &errorString) override;

private:
    QGeoTileSpec ancestorForTile(const QGeoTileSpec &tile) const;

    QString m_cacheDirectory;
    bool m_noMapTiles{false};
    bool m_enableLogging{false};
    int m_maximumZoomLevel{-1};
    // ancestor -> map -> set of high-zoom followers waiting for that ancestor.
    QHash<QGeoTileSpec, QHash<QGeoTiledMap *, QSet<QGeoTileSpec>>> m_overzoomTiles;
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPPINGMANAGERENGINEMAPBOX_H
