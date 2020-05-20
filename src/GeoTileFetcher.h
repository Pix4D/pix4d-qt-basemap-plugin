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

public:
    // The list of map style ids
    static constexpr const char* PIX4D_STREET = "ck8zz9gpq0vty1ip30bji3b5a";
    static constexpr const char* PIX4D_STREETS_SATELLITE = "ck8zzfxb30vwp1jo04yktjtbg";

private:
    QNetworkAccessManager* m_networkManager{nullptr};
    QByteArray m_userAgent;
    QString m_replyFormat;
    QString m_accessToken;
    QVector<QString> m_mapIds;
    int m_scaleFactor{0};
    bool m_enableLogging{false};
    QString m_customBasemapUrl;
};
