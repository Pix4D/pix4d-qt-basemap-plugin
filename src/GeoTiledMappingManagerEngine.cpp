#include "GeoTiledMappingManagerEngine.h"
#include "GeoTileFetcher.h"
#include "GeoFileTileCache.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>

namespace
{
    static const QString PLUGIN_NAME(QStringLiteral("basemap_pix4d"));

    bool getParameter(const QVariantMap& parameters, const QString& key)
    {
        return parameters.contains(key) && parameters.value(key).toString().toLower() == "true";
    }

    bool getParameter(const QVariantMap& parameters, const QString& key, QString& value)
    {
        if (parameters.contains(key))
        {
            const auto keyValue = parameters.value(key).toString();
            if (keyValue.toLower() != "false") // Empty strings are passed in as 'false'
            {
                value = keyValue;
                return true;
            }
        }
        return false;
    }

    bool getParameter(const QVariantMap& parameters, const QString& key, int& value)
    {
        if (parameters.contains(key))
        {
            value = parameters.value(key).toInt();
            return true;
        }
        return false;
    }
}

GeoTiledMappingManagerEngine::GeoTiledMappingManagerEngine(const QVariantMap& parameters,
                                                           QGeoServiceProvider::Error* error,
                                                           QString* errorString)
      : QGeoTiledMappingManagerEngine()
{
    const QByteArray pluginName = PLUGIN_NAME.toUtf8();
    const bool enableLogging = getParameter(parameters, "enable_logging");

    int maximumZoomLevel = 25;
    getParameter(parameters, "maximum_zoom_level", maximumZoomLevel);

    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(maximumZoomLevel);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(80);
    cameraCaps.setMinimumFieldOfView(20.0);
    cameraCaps.setMaximumFieldOfView(120.0);
    cameraCaps.setOverzoomEnabled(true);
    setCameraCapabilities(cameraCaps);

    QString customBasemapUrl;
    getParameter(parameters, "custom_basemap_url", customBasemapUrl);
    const bool usingMapBox = customBasemapUrl.isEmpty();

    setTileSize(QSize(256, 256));
    m_noMapTiles = getParameter(parameters, "no_map_tiles");

    QList<QGeoMapType> mapTypes;
    QVector<QString> mapIds;
    if (usingMapBox)
    {
        mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay,
                                GeoTileFetcher::PIX4D_STREETS_SATELLITE,
                                QStringLiteral("Satellite"),
                                false,
                                false,
                                mapTypes.size() + 1,
                                pluginName,
                                cameraCaps);
        mapTypes << QGeoMapType(QGeoMapType::StreetMap,
                                GeoTileFetcher::PIX4D_STREET,
                                QStringLiteral("Street"),
                                false,
                                false,
                                mapTypes.size() + 1,
                                pluginName,
                                cameraCaps);

        for (const auto& type : mapTypes)
        {
            mapIds.push_back(type.name());
        }
    }
    setSupportedMapTypes(mapTypes);

    // Set up the tile fetcher
    const int scaleFactor = getParameter(parameters, "highdpi_tiles") ? 2 : 1;
    auto tileFetcher = new GeoTileFetcher(scaleFactor, enableLogging, customBasemapUrl, this);
    tileFetcher->setMapIds(mapIds);

    QString format;
    if (getParameter(parameters, "format", format))
    {
        tileFetcher->setFormat(format);
    }

    QString accessToken;
    if (getParameter(parameters, "access_token", accessToken))
    {
        tileFetcher->setAccessToken(accessToken);
    }
    else
    {
        qCritical() << "Basemap Plugin no access token was set";
    }

    setTileFetcher(tileFetcher);

    if (usingMapBox)
    {
        // Set up the tile cache
        QString cacheDirectory;
        if (usingMapBox && !getParameter(parameters, "cache_directory", cacheDirectory))
        {
            cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String(pluginName);
        }

        auto tileCache = new GeoFileTileCache(mapTypes, scaleFactor, enableLogging, cacheDirectory);
        tileCache->setCostStrategyDisk(QGeoFileTileCache::Unitary);
        tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
        tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
        tileCache->setMaxDiskUsage(6000);
        setTileCache(tileCache);
    }
    else
    {
        // Do not cache any custom user maps due to legality
        setCacheHint(QAbstractGeoTileCache::CacheArea::MemoryCache);
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();

    if (enableLogging)
    {
        qInfo() << "Basemap Plugin initialised";
    }
}

QGeoMap* GeoTiledMappingManagerEngine::createMap()
{
    return m_noMapTiles ? new NoGeoTiledMap(this, 0) : new QGeoTiledMap(this, 0);
}

QSGNode* NoGeoTiledMap::updateSceneGraph(QSGNode*, QQuickWindow* window)
{
    return nullptr;
}
