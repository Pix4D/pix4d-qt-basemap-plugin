// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeomapreplymapbox.h"

#include <QtLocation/private/qgeotilespec_p.h>
#include <QTimer>

QGeoMapReplyMapbox::QGeoMapReplyMapbox(QNetworkAccessManager *networkManager,
                                       const QNetworkRequest &request,
                                       const QGeoTileSpec &spec,
                                       const QString &format,
                                       bool enableLogging,
                                       int maxRetries,
                                       QObject *parent)
:   QGeoTiledMapReply(spec, parent),
    m_networkManager(networkManager),
    m_request(request),
    m_format(format),
    m_enableLogging(enableLogging),
    m_retriesLeft(maxRetries)
{
    if (!m_networkManager)
    {
        setError(UnknownError, QStringLiteral("Null network manager"));
        return;
    }
    startRequest();
}

QGeoMapReplyMapbox::~QGeoMapReplyMapbox()
{
}

void QGeoMapReplyMapbox::setAncestorRemap(const QGeoTileSpec &ancestorSpec)
{
    m_ancestorSpec = ancestorSpec;
    m_ancestorRemap = true;
}

void QGeoMapReplyMapbox::startRequest()
{
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
    connect(reply, &QNetworkReply::finished,
            this, &QGeoMapReplyMapbox::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoMapReplyMapbox::networkReplyError);
    connect(this, &QGeoTiledMapReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
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

void QGeoMapReplyMapbox::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    const QByteArray bytes = reply->readAll();

    if (m_ancestorRemap)
    {
        // The URL we fetched is actually the maxZoom ancestor of our
        // (high-zoom) tileSpec(). Fan out the result in two stages:
        //   1. ancestorReady -> QGeoTileFetcherMapbox emits tileFinished for
        //      the ancestor spec, which makes
        //      QGeoTiledMappingManagerEngine::engineTileFinished() cache the
        //      real bytes under the ancestor's filename. From there
        //      QGeoFileTileCacheMapbox::get() will substitute it (stretched)
        //      for any high-zoom request whose ancestor is this one.
        //   2. setFinished(true) below emits the reply's own finished signal
        //      with an *empty* payload under the original (high-zoom) spec.
        //      QGeoFileTileCacheMapbox::isTileBogus() short-circuits the
        //      disk write, but the empty emission still flows through
        //      engineTileFinished -> QGeoTileRequestManager::tileFetched(),
        //      clearing m_requested so the next render cycle re-asks the
        //      cache and the substitution returns the freshly-cached
        //      ancestor.
        emit ancestorReady(m_ancestorSpec, bytes, m_format);
        setMapImageData(QByteArray());
    }
    else
    {
        setMapImageData(bytes);
    }
    setMapImageFormat(m_format);
    setFinished(true);
}

void QGeoMapReplyMapbox::networkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (error == QNetworkReply::OperationCanceledError)
    {
        setFinished(true);
        return;
    }

    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString errorMsg = QStringLiteral("HTTP %1: %2").arg(httpStatus).arg(reply->errorString());

    if (m_retriesLeft > 0 && isRetriableError(httpStatus))
    {
        --m_retriesLeft;
        if (m_enableLogging)
        {
            qWarning() << "QGeoMapReplyMapbox:" << errorMsg
                        << "- retrying (" << m_retriesLeft << "left)";
        }
        QTimer::singleShot(500, this, &QGeoMapReplyMapbox::startRequest);
        return;
    }

    if (m_enableLogging)
    {
        qWarning() << "QGeoMapReplyMapbox:" << errorMsg;
    }
    setError(QGeoTiledMapReply::CommunicationError, errorMsg);
}
