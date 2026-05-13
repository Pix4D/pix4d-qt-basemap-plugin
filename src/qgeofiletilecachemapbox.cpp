// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeotilefetchermapbox.h"
#include "qgeofiletilecachemapbox.h"

#include <QtLocation/private/qgeotilespec_p.h>
#include <QDir>

QT_BEGIN_NAMESPACE

QGeoFileTileCacheMapbox::QGeoFileTileCacheMapbox(const QList<QGeoMapType> &mapTypes,
                                                 int scaleFactor, bool enableLogging, const QString &directory,
                                                 QObject *parent)
    :QGeoFileTileCache(directory, parent), 
    m_enableLogging(enableLogging), 
    m_hasCacheDirectory(!directory.isEmpty()),
    m_mapTypes(mapTypes)
{
    m_scaleFactor = qBound(1, scaleFactor, 2);
    for (qsizetype i = 0; i < mapTypes.size(); i++)
        m_mapNameToId.insert(mapTypes[i].name(), i);
}

QGeoFileTileCacheMapbox::~QGeoFileTileCacheMapbox()
{

}

void QGeoFileTileCacheMapbox::setMaximumZoomLevel(int maxZoom)
{
    m_maximumZoomLevel = maxZoom;
}

QSharedPointer<QGeoTileTexture> QGeoFileTileCacheMapbox::get(const QGeoTileSpec &spec)
{
    // First try the requested tile at its actual zoom level.
    QSharedPointer<QGeoTileTexture> tex = QGeoFileTileCache::get(spec);
    if (tex && !tex->image.isNull())
        return tex;

    // If we are overzooming past the tile server's maximum zoom, substitute
    // the best cached ancestor so QGeoTiledMapScene can stretch it. The
    // returned texture's spec carries the ancestor's lower zoom level;
    // QGeoTiledMapScenePrivate::buildGeometry detects this (texture->spec.zoom()
    // < requested.zoom()) and renders the correct sub-rectangle scaled up.
    //
    // Because QGeoTileRequestManagerPrivate::requestTiles asks the engine for
    // a texture *before* its hard-coded 4-level overzoom lookback runs, a hit
    // here also bypasses that loop entirely - and removes the high-zoom spec
    // from the request set so no 404-bound HTTP request is ever issued.
    //
    // We walk from maximum_zoom_level down toward 0 instead of only probing
    // maximum_zoom_level: for tile servers (e.g. MapTiler satellite) whose
    // *effective* per-area top zoom is lower than the configured maximum, the
    // ancestor at maximum_zoom_level will 404 forever and never enter the
    // cache. In that case we'd rather hand back a more pixelated lower-zoom
    // tile that we *do* have (typically from prefetch or an earlier zoom-out
    // session) than fall through to QGeoTileRequestManager's hard-coded 4-
    // level fallback and return black. When the real ancestor eventually
    // lands in cache, addTile() detects the spec collision via
    // m_textures and m_updatedTextures replaces the QSGTexture on the next
    // frame, so the stretched stand-in only stays around as long as it has to.
    if (m_maximumZoomLevel > 0 && spec.zoom() > m_maximumZoomLevel)
    {
        const int maxDelta = qMin<int>(spec.zoom(), 30); // guard 1 << delta
        for (int delta = spec.zoom() - m_maximumZoomLevel; delta <= maxDelta; ++delta)
        {
            const int ancestorZoom = spec.zoom() - delta;
            const int denom = 1 << delta;
            const QGeoTileSpec ancestor(spec.plugin(),
                                        spec.mapId(),
                                        ancestorZoom,
                                        spec.x() / denom,
                                        spec.y() / denom,
                                        spec.version());
            QSharedPointer<QGeoTileTexture> ancestorTex = QGeoFileTileCache::get(ancestor);
            if (ancestorTex && !ancestorTex->image.isNull())
                return ancestorTex;
        }
    }

    return QSharedPointer<QGeoTileTexture>();
}

QString QGeoFileTileCacheMapbox::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format,
                                                    const QString &directory) const
{
    if (!m_hasCacheDirectory)
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: No cache directory.";
        }
        return QString();
    }
    if (m_mapTypes[spec.mapId()].name() == QGeoTileFetcherMapbox::PIX4D_CUSTOM)
    {
        if (m_enableLogging)
        {
            qInfo() << "GeoFileTileCache: Do not cache any custom user maps due to legality. The read/write warnings can be ignored.";
        }
        // Will get the warnings because it returns empty QString
        // WARNING: qt-msg: QFSFileEngine::open: No file name specified
        // WARNING: qt-msg: QIODevice::write (QFile, ""): device not open
        return QString();
    }

    QString filename = spec.plugin();

    filename += QLatin1String("-");
    filename += m_mapTypes[spec.mapId()].name();
    filename += QLatin1String("-");
    filename += QString::number(spec.zoom());
    filename += QLatin1String("-");
    filename += QString::number(spec.x());
    filename += QLatin1String("-");
    filename += QString::number(spec.y());

    if (spec.version() != -1)
    {
        filename += QLatin1String("-");
        filename += QString::number(spec.version());
    }

    filename += QLatin1String("-@");
    filename += QString::number(m_scaleFactor);
    filename += QLatin1Char('x');
    filename += QLatin1String(".");
    filename += format;

    return QDir(directory).filePath(filename);
}

QGeoTileSpec QGeoFileTileCacheMapbox::filenameToTileSpec(const QString &filename) const
{
    // @See QGeoFileTileCacheMapBox for more information for this function
    // General scheme is: plugin_name - map_type - zoom - x - y - @scale.png
    // For now the cached tiles names look like this: 
    //   basemap_pix4d_100-satellite-11-1091-655-@2x.png or
    //   basemap_pix4d_100-streets-16-35012-20990-@2x.png
    // basemap_pix4d_100 = plugin name
    // Satellite or Streets = map type name
    // 13 = zoom
    // 4401-2685 = x, y

    if (!m_hasCacheDirectory)
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: No cache directory.";
        }
        return QGeoTileSpec();
    }

    const QStringList parts = filename.split('.');
    if (parts.length() != 2) // 2 because the map name has always a dot in it.
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: This file doesn't have a file extension.";
        }
        return QGeoTileSpec();
    }

    // name = basemap_pix4d_100-streets-16-35012-20990-@2x (no extension)
    const QString name = parts.at(0);
    const QStringList fields = name.split('-');
    
    // must be at least 6 different fields
    if (fields.length() < 6)
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: The file name without extension doesn't have 6 different fields.";
        }
        return QGeoTileSpec();
    }

    const int length = fields.length();
    const int scaleIdx = fields.last().indexOf("@");
    if (scaleIdx < 0 || fields.last().size() <= (scaleIdx + 2))
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: Scale is not last data in name.";
        }
        return QGeoTileSpec();
    }

    int scaleFactor = fields.last()[scaleIdx + 1].digitValue();
    if (scaleFactor != m_scaleFactor)
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: The last fields doesn't have scale factor.";
        }
        return QGeoTileSpec();
    }

    QList<int> numbers;
    for (int i = 2; i < length - 1; ++i) // skipping -@_X
    {  
        bool ok = false;
        const int value = fields.at(i).toInt(&ok);
        if (!ok)
        {
            if (m_enableLogging)
            {
                qWarning() << "GeoFileTileCache: The value of zoom/x/y must be integer.";
            }
            return QGeoTileSpec();
        }
        numbers.append(value);
    }

    if (numbers.length() < 3)
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoFileTileCache: The cache file must have zoom/x/y value.";
        }
        return QGeoTileSpec();
    }

    // File name without version, append default
    if (numbers.length() < 4)
    {
        numbers.append(-1);
    }

    return QGeoTileSpec(
        fields.at(0), m_mapNameToId[fields.at(1)], numbers.at(0), numbers.at(1), numbers.at(2), numbers.at(3));
}

QT_END_NAMESPACE
