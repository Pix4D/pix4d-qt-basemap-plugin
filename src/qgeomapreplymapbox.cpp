// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeomapreplymapbox.h"
#include "qgeotilefetchermapbox.h"

#include <QtLocation/private/qgeotilespec_p.h>
#include <QRandomGenerator>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace
{
    constexpr int RETRY_BASE_DELAY_MS = 500;
    constexpr int RETRY_AFTER_CONNECTION_DROP_MS = 1500;
}

QGeoMapReplyMapbox::QGeoMapReplyMapbox(QNetworkAccessManager *networkManager,
                                       const QNetworkRequest &request,
                                       const QGeoTileSpec &spec,
                                       const QString &format,
                                       bool enableLogging,
                                       QGeoTileFetcherMapbox *fetcher,
                                       QNetworkReply *runningDownload,
                                       int maxRetries)
:   QGeoTiledMapReply(spec, fetcher),
    m_fetcher(fetcher),
    m_networkManager(networkManager),
    m_request(request),
    m_format(format),
    m_enableLogging(enableLogging),
    m_retriesLeft(maxRetries)
{
    connect(this, &QGeoTiledMapReply::aborted, this, &QGeoMapReplyMapbox::handOverDownload);
    if (runningDownload)
    {
        connectReply(runningDownload);
        return;
    }
    if (!m_networkManager)
    {
        setError(UnknownError, QStringLiteral("Null network manager"));
        return;
    }
    startRequest();
}

QGeoMapReplyMapbox::~QGeoMapReplyMapbox()
{
    if (m_networkReply)
        m_networkReply->deleteLater();
}

void QGeoMapReplyMapbox::handOverDownload()
{
    QNetworkReply *reply = std::exchange(m_networkReply, nullptr);
    if (!reply)
        return;

    disconnect(reply, nullptr, this, nullptr);
    // Aborting resets the HTTP/2 stream, and servers answer too many resets with GOAWAY PROTOCOL_ERROR.
    if (!m_fetcher->adoptCancelledDownload(reply, tileSpec()))
    {
        reply->abort();
        reply->deleteLater();
    }
}

void QGeoMapReplyMapbox::startRequest()
{
    // A retry can still be scheduled when the tile is cancelled.
    if (isFinished())
        return;
    if (!m_networkManager)
    {
        setError(UnknownError, QStringLiteral("Network manager unavailable"));
        setFinished(true);
        return;
    }

    QNetworkReply *reply = m_networkManager->get(m_request);
    if (!reply)
    {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connectReply(reply);
}

void QGeoMapReplyMapbox::connectReply(QNetworkReply *reply)
{
    m_networkReply = reply;
    connect(reply, &QNetworkReply::finished,
            this, &QGeoMapReplyMapbox::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoMapReplyMapbox::networkReplyError);
}

bool QGeoMapReplyMapbox::isRetriableError(int httpStatus) const
{
    return httpStatus == 0          // network-level failure (timeout, DNS, etc.)
        || httpStatus == 429        // Too Many Requests
        || httpStatus == 500
        || httpStatus == 502
        || httpStatus == 503
        || httpStatus == 504;
}

int QGeoMapReplyMapbox::retryDelayMs(QNetworkReply::NetworkError error)
{
    int delayMs = RETRY_BASE_DELAY_MS << std::min(m_retryCount++, 4);
    // After a dropped connection, such as a GOAWAY, give the server time before reconnecting.
    if (error == QNetworkReply::ProtocolFailure || error == QNetworkReply::RemoteHostClosedError)
        delayMs = std::max(delayMs, RETRY_AFTER_CONNECTION_DROP_MS);
    // Jitter keeps the tiles that failed together from retrying together.
    return delayMs + QRandomGenerator::global()->bounded(delayMs / 2 + 1);
}

void QGeoMapReplyMapbox::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    setMapImageData(reply->readAll());
    setMapImageFormat(m_format);
    setFinished(true);
}

// A transfer timeout aborts with OperationCanceledError (TimeoutError since Qt 6.11), which is retried like other
// network failures.
void QGeoMapReplyMapbox::networkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString errorMsg = QStringLiteral("HTTP %1: %2").arg(httpStatus).arg(reply->errorString());

    if (m_retriesLeft > 0 && isRetriableError(httpStatus))
    {
        if (!m_networkManager)
        {
            setError(QGeoTiledMapReply::CommunicationError, errorMsg);
            setFinished(true);
            return;
        }

        --m_retriesLeft;
        const int delayMs = retryDelayMs(error);
        if (m_enableLogging)
        {
            qWarning() << "QGeoMapReplyMapbox:" << errorMsg
                        << "- retrying in" << delayMs << "ms (" << m_retriesLeft << "left)";
        }
        QTimer::singleShot(delayMs, this, &QGeoMapReplyMapbox::startRequest);
        return;
    }

    if (m_enableLogging)
    {
        qWarning() << "QGeoMapReplyMapbox:" << errorMsg;
    }
    setError(QGeoTiledMapReply::CommunicationError, errorMsg);
}
