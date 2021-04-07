#include "GeoFileTileCache.h"

#include <QDir>
#include <QtLocation/private/qgeotilespec_p.h>

GeoFileTileCache::GeoFileTileCache(const QList<QGeoMapType>& mapTypes,
    int scaleFactor,
    bool enableLogging,
    const QString& directory,
    QObject* parent)
    : QGeoFileTileCache(directory, parent)
    , m_enableLogging(enableLogging)
    , m_hasCacheDirectory(!directory.isEmpty())
    , m_mapTypes(mapTypes)
{
    m_scaleFactor = qBound(1, scaleFactor, 2);
    for (int i = 0; i < mapTypes.size(); i++)
    {
        m_mapNameToId.insert(mapTypes[i].name(), i);
    }
}

QString GeoFileTileCache::tileSpecToFilename(const QGeoTileSpec& spec,
    const QString& format,
    const QString& directory) const
{
    if (!m_hasCacheDirectory)
    {
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

QGeoTileSpec GeoFileTileCache::filenameToTileSpec(const QString& filename) const
{
    // @See QGeoFileTileCacheMapBox for more information for this function
    // General scheme is: plugin_name - map_type - zoom - x - y - @scale.png
    // For now the cached tiles names look like this: 
    //   basemap_pix4d_100-ck8zzfxb30vwp1jo04yktjtbg-13-4401-2685-@2x.png or
    //   basemap_pix4d_100-ck8zz9gpq0vty1ip30bji3b5a-16-35208-21496-@1x.png
    // basemap_pix4d_100 = plugin name
    // ck8zzfxb30vwp1jo04yktjtbg = streets-satellite type map
    // ck8zz9gpq0vty1ip30bji3b5a = street type map
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

    // name = mapbox_pix4d_100-ck8zzfxb30vwp1jo04yktjtbg-13-4401-2685-@2x (no extension)
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
