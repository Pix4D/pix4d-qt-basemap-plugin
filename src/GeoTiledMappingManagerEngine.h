#pragma once

#include <QtLocation/QGeoServiceProvider>

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

class NoGeoTiledMap : public QGeoTiledMap
{
    using QGeoTiledMap::QGeoTiledMap;

    QSGNode* updateSceneGraph(QSGNode*, QQuickWindow* window) override;
};

class CustomGeoTiledMap: public QGeoTiledMap {
public:
    CustomGeoTiledMap(GeoTiledMappingManagerEngine *engine, QObject *parent);
    virtual ~CustomGeoTiledMap() = default;
    QString copyrightsStyleSheet() const override;
    virtual void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles) override;
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