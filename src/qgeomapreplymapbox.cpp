// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeomapreplymapbox.h"

#include <QtLocation/private/qgeotilespec_p.h>

QGeoMapReplyMapbox::QGeoMapReplyMapbox(QNetworkReply *reply, const QGeoTileSpec &spec, const QString &format, bool enableLogging, QObject *parent)
:   QGeoTiledMapReply(spec, parent), m_format (format), m_enableLogging(enableLogging)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        if (m_enableLogging)
        {
            qWarning() << "QGeoMapReplyMapbox: " << errorString();
        }
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &QGeoMapReplyMapbox::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoMapReplyMapbox::networkReplyError);
    connect(this, &QGeoTiledMapReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QGeoMapReplyMapbox::~QGeoMapReplyMapbox()
{
}

void QGeoMapReplyMapbox::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        if (m_enableLogging)
        {
            qWarning() << "QGeoMapReplyMapbox: " << reply->errorString();
        }
        return;
    }        

    setMapImageData(reply->readAll());
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
    }
    else
    {
        if (m_enableLogging)
        {
            qWarning() << "QGeoMapReplyMapbox: " << reply->errorString();
        }
        setError(QGeoTiledMapReply::CommunicationError, reply->errorString());
    }
}
