#pragma once

#include <QVector>
#include <QtLocation/private/qgeotilefetcher_p.h>

class QGeoTiledMappingManagerEngine;
class QNetworkAccessManager;

class GeoTileFetcher : public QGeoTileFetcher
{
    Q_OBJECT

public:
    GeoTileFetcher(int scaleFactor, bool enableLogging, const QString& customBasemapUrl, QGeoTiledMappingManagerEngine* parent);

    void setMapIds(const QVector<QString>& mapIds);
    void setFormat(const QString& format);
    void setAccessToken(const QString& accessToken);

private:
    QGeoTiledMapReply* getTileImage(const QGeoTileSpec& spec);

private:
    QNetworkAccessManager* m_networkManager{nullptr};
    QByteArray m_userAgent;
    QString m_format;
    QString m_replyFormat;
    QString m_accessToken;
    QVector<QString> m_mapIds;
    int m_scaleFactor{0};
    bool m_enableLogging{false};
    QString m_customBasemapUrl;
};