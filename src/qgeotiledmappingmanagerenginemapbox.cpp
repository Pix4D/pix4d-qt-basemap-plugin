// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeotiledmappingmanagerenginemapbox.h"
#include "qgeotilefetchermapbox.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>
#include "qgeofiletilecachemapbox.h"

typedef QGeoTiledMap Map;

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

QT_BEGIN_NAMESPACE

QGeoTiledMappingManagerEngineMapbox::QGeoTiledMappingManagerEngineMapbox(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
:   QGeoTiledMappingManagerEngine()
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

    setTileSize(QSize(512, 512));
    m_noMapTiles = getParameter(parameters, "no_map_tiles");

    QList<QGeoMapType> mapTypes;
    mapTypes << QGeoMapType(QGeoMapType::NoMap,
                            QGeoTileFetcherMapbox::PIX4D_NONE,
                            QStringLiteral("None"),
                            false,
                            false,
                            mapTypes.size(),
                            pluginName,
                            cameraCaps);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay,
                            QGeoTileFetcherMapbox::PIX4D_SATELLITE,
                            QStringLiteral("Satellite"),
                            false,
                            false,
                            mapTypes.size(),
                            pluginName,
                            cameraCaps);
    mapTypes << QGeoMapType(QGeoMapType::StreetMap,
                            QGeoTileFetcherMapbox::PIX4D_STREETS,
                            QStringLiteral("Streets"),
                            false,
                            false,
                            mapTypes.size(),
                            pluginName,
                            cameraCaps);

    QString customBasemapUrl;
    getParameter(parameters, "custom_basemap_url", customBasemapUrl);
    if (!customBasemapUrl.isEmpty())
    {
        mapTypes << QGeoMapType(QGeoMapType::CustomMap,
                                QGeoTileFetcherMapbox::PIX4D_CUSTOM,
                                QStringLiteral("Custom"),
                                false,
                                false,
                                mapTypes.size(),
                                pluginName,
                                cameraCaps);
    }

    if (enableLogging)
    {
        qInfo() << "Plugin supports " << mapTypes.size() <<" map types:";
        for (const auto& t : std::as_const(mapTypes))
        {
            qInfo() << t.description();
        }
    }

    setSupportedMapTypes(mapTypes);

    const int scaleFactor = getParameter(parameters, "highdpi_tiles") ? 2 : 1;
    QGeoTileFetcherMapbox* tileFetcher = new QGeoTileFetcherMapbox(scaleFactor, enableLogging, customBasemapUrl, this);
    tileFetcher->setMaximumZoomLevel(maximumZoomLevel);

    QVector<QString> mapIds;
    for (const auto& type : std::as_const(mapTypes))
    {
        mapIds.push_back(type.name());
    }
    tileFetcher->setMapIds(mapIds);


    QString useragent;
    if (getParameter(parameters, "useragent", useragent))
    {
        tileFetcher->setUserAgent(useragent.toLatin1());
    }

    QString format;
    if (getParameter(parameters, "format", format))
    {
        tileFetcher->setFormat(format);
    }
    
    tileFetcher->setAdditionalParameters(parameters);
    setTileFetcher(tileFetcher);

    // Set up the tile cache
    QString cacheDirectory;
    if (!getParameter(parameters, "cache_directory", cacheDirectory))
    {
        cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String(pluginName);
    }

    auto tileCache = new QGeoFileTileCacheMapbox(mapTypes, scaleFactor, enableLogging, cacheDirectory);
    tileCache->setMaximumZoomLevel(maximumZoomLevel);
    tileCache->setCostStrategyDisk(QGeoFileTileCache::Unitary);
    tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
    tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
    tileCache->setMaxDiskUsage(6000);
    setTileCache(tileCache);

    if (!customBasemapUrl.isEmpty())
    {
        // If custom basemap URL is not empty, enable both CacheArea::DiskCache and CacheArea::MemoryCache.
        // CacheArea::DiskCache is only for default basemaps that can cache in directory as same as when custom basemap URL is empty.
        // CacheArea::MemoryCache is for custom basemaps that do not cache tile images to files due to legality.
        // In QGeoFileTileCacheMapbox::tileSpecToFilename(),
        // tile images of custom basemaps will be skipped saving in cache directory and will use only memory cache.
        setCacheHint(QAbstractGeoTileCache::CacheArea::AllCaches);
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();

    if (enableLogging)
    {
        qInfo() << "Basemap Plugin initialised";
    }
}

QGeoTiledMappingManagerEngineMapbox::~QGeoTiledMappingManagerEngineMapbox()
{
}

QGeoMap *QGeoTiledMappingManagerEngineMapbox::createMap()
{
    QGeoTiledMap *map = new Map(this, 0);
    map->setPrefetchStyle(m_prefetchStyle);
    return map;
}

QT_END_NAMESPACE
