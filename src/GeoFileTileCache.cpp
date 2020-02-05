#include "GeoFileTileCache.h"

#include <QDir>
#include <QtLocation/private/qgeotilespec_p.h>

GeoFileTileCache::GeoFileTileCache(const QList<QGeoMapType>& mapTypes,
    int scaleFactor,
    const QString& directory,
    QObject* parent)
    : QGeoFileTileCache(directory, parent)
    , m_hasCacheDirectory(!directory.isEmpty())
    , m_mapTypes(mapTypes)
{
    m_scaleFactor = qBound(1, scaleFactor, 2);
    for (int i = 0; i < mapTypes.size(); i++)
    {
        m_mapNameToId.insert(mapTypes[i].name(), i + 1);
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
    filename += m_mapTypes[spec.mapId() - 1].name();
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
    //   basemap_pix4d_100-mapbox.streets-satellite-13-4401-2685-@2x.png or 
    //   basemap_pix4d_100-mapbox.satellite-16-35208-21496-@1x.png
    // basemap_pix4d_100 = plugin name
    // mapbox.streets-satellite / mapbox.satellite = map type
    // 13 = zoom
    // 4401-2685 = x, y

    if (!m_hasCacheDirectory)
    {
        return QGeoTileSpec();
    }

    const QStringList parts = filename.split('.');
    if (parts.length() != 3) // 3 because the map name has always a dot in it.
    {
        return QGeoTileSpec();
    }

    // name = mapbox_pix4d_100-mapbox.streets-satellite-13-4401-2685-@2x (no extension)
    const QString name = parts.at(0) + "." + parts.at(1);
    const QStringList fields = name.split('-');

    // must be at least 6 different fields
    if (fields.length() < 6)
    {
        return QGeoTileSpec();
    }

    const int length = fields.length();
    const int scaleIdx = fields.last().indexOf("@");
    if (scaleIdx < 0 || fields.last().size() <= (scaleIdx + 2))
    {
        return QGeoTileSpec();
    }

    int scaleFactor = fields.last()[scaleIdx + 1].digitValue();
    if (scaleFactor != m_scaleFactor)
    {
        return QGeoTileSpec();
    }

    QList<int> numbers;
    int startIndex = 2;

    // try to combine the fields from [1] and [2]. In case the map type 
    // contains a '-' in it (e.g. mapbox.streets-satellite vs mapbox.satellite)
    QString pluginName = fields.at(1);
    if (m_mapNameToId.find(pluginName + "-" + fields.at(2)) != m_mapNameToId.end())
    {
        pluginName += "-" + fields.at(2);
        startIndex = 3;
    }

    for (int i = startIndex; i < length - 1; ++i) // skipping -@_X
    {  
        bool ok = false;
        const int value = fields.at(i).toInt(&ok);
        if (!ok)
        {
            return QGeoTileSpec();
        }
        numbers.append(value);
    }

    if (numbers.length() < 3)
    {
        return QGeoTileSpec();
    }

    // File name without version, append default
    if (numbers.length() < 4)
    {
        numbers.append(-1);
    }

    return QGeoTileSpec(
        fields.at(0), m_mapNameToId[pluginName], numbers.at(0), numbers.at(1), numbers.at(2), numbers.at(3));
}