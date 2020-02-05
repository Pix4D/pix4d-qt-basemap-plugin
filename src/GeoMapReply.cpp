#include "GeoMapReply.h"

#include <QtLocation/private/qgeotilespec_p.h>

GeoMapReply::GeoMapReply(QNetworkReply* reply,
                         const QGeoTileSpec& spec,
                         const QString& format,
                         bool enableLogging,
                         QObject* parent)
      : QGeoTiledMapReply(spec, parent)
      , m_format(format)
      , m_enableLogging(enableLogging)
{
    if (!reply)
    {
        setError(UnknownError, QStringLiteral("Null reply"));
        if (m_enableLogging)
        {
            qWarning() << "GeoMapReply: " << errorString();
        }
    }
    else
    {
        // Cannot use qOverload for Windows: https://bugreports.qt.io/browse/QTBUG-61667
        void (QNetworkReply::*errorSignal)(QNetworkReply::NetworkError) = &QNetworkReply::error;

        connect(reply, &QNetworkReply::finished, this, &GeoMapReply::networkReplyFinished);
        connect(reply, errorSignal, this, &GeoMapReply::networkReplyError);
        connect(this, &QGeoTiledMapReply::aborted, reply, &QNetworkReply::abort);
        connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    }
}

void GeoMapReply::networkReplyFinished()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoMapReply: " << reply->errorString();
        }
    }
    else
    {
        setMapImageData(reply->readAll());
        setMapImageFormat(m_format);
        setFinished(true);
    }
}

void GeoMapReply::networkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    if (error == QNetworkReply::OperationCanceledError)
    {
        setFinished(true);
    }
    else
    {
        if (m_enableLogging)
        {
            qWarning() << "GeoMapReply: " << reply->errorString();
        }
        setError(QGeoTiledMapReply::CommunicationError, reply->errorString());
    }
}
