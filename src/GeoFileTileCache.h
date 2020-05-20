#pragma once

#include <QtLocation/private/qgeofiletilecache_p.h>
#include <QMap>

class GeoFileTileCache : public QGeoFileTileCache
{
    Q_OBJECT

public:
    GeoFileTileCache(const QList<QGeoMapType>& mapTypes, int scaleFactor, bool enableLogging, const QString& directory = QString(), QObject* parent = nullptr);

protected:
    QString tileSpecToFilename(const QGeoTileSpec& spec, const QString& format, const QString& directory) const override;
    QGeoTileSpec filenameToTileSpec(const QString& filename) const override;

    bool m_enableLogging{false};
    bool m_hasCacheDirectory{false};
    QList<QGeoMapType> m_mapTypes;
    QMap<QString, int> m_mapNameToId;
    int m_scaleFactor{0};
};
