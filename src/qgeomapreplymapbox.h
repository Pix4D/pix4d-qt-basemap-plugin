// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAPREPLYMAPBOX_H
#define QGEOMAPREPLYMAPBOX_H

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoMapReplyMapbox : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    explicit QGeoMapReplyMapbox(QNetworkAccessManager *networkManager,
                                const QNetworkRequest &request,
                                const QGeoTileSpec &spec,
                                const QString &format,
                                bool enableLogging,
                                int maxRetries = 2,
                                QObject *parent = nullptr);
    ~QGeoMapReplyMapbox();

    // Tells the reply that the URL it's about to hit is actually the maxZoom
    // ancestor of its tileSpec(). On a successful response we emit
    // ancestorReady() so the fetcher can cache the bytes under the ancestor's
    // spec, then complete ourselves with an empty payload under the original
    // spec - which still triggers QGeoTileRequestManager::tileFetched() (so
    // m_requested clears) while QGeoFileTileCacheMapbox::isTileBogus() skips
    // the disk write for the empty payload.
    void setAncestorRemap(const QGeoTileSpec &ancestorSpec);

Q_SIGNALS:
    void ancestorReady(const QGeoTileSpec &ancestor, const QByteArray &bytes, const QString &format);

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    void startRequest();
    void connectReply(QNetworkReply *reply);
    bool isRetriableError(int httpStatus) const;

    QNetworkAccessManager *m_networkManager;
    QNetworkRequest m_request;
    QString m_format;
    bool m_enableLogging{false};
    int m_retriesLeft{0};
    QGeoTileSpec m_ancestorSpec;
    bool m_ancestorRemap{false};
};

QT_END_NAMESPACE

#endif // QGEOMAPREPLYMAPBOX_H
