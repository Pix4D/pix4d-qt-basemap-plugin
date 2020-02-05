#pragma once

#include <QtLocation/QGeoServiceProvider>

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

class NoGeoTiledMap : public QGeoTiledMap
{
    using QGeoTiledMap::QGeoTiledMap;

    QSGNode* updateSceneGraph(QSGNode*, QQuickWindow* window) override;
};

class GeoTiledMappingManagerEngine : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

public:
    GeoTiledMappingManagerEngine(const QVariantMap& parameters,
                                 QGeoServiceProvider::Error* error, QString* errorString);
    QGeoMap* createMap();

private:
    bool m_noMapTiles{false};
};