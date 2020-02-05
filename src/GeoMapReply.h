#pragma once

#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtNetwork/QNetworkReply>

class GeoMapReply : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    GeoMapReply(QNetworkReply* reply,
                const QGeoTileSpec& spec,
                const QString& format,
                bool enableLogging,
                QObject* parent = nullptr);

private slots:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QString m_format;
    bool m_enableLogging{false};
};
