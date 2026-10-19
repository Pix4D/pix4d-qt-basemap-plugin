// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAPREPLYMAPBOX_H
#define QGEOMAPREPLYMAPBOX_H

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoTileFetcherMapbox;

class QGeoMapReplyMapbox : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    /// Uses \a runningDownload, a download of the same tile that is still running, instead of sending a new GET.
    explicit QGeoMapReplyMapbox(QNetworkAccessManager *networkManager,
                                const QNetworkRequest &request,
                                const QGeoTileSpec &spec,
                                const QString &format,
                                bool enableLogging,
                                QGeoTileFetcherMapbox *fetcher,
                                QNetworkReply *runningDownload = nullptr,
                                int maxRetries = 2,
                                QObject *parent = nullptr);
    ~QGeoMapReplyMapbox();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    void startRequest();
    void connectReply(QNetworkReply *reply);
    void handOverDownload();
    bool isRetriableError(int httpStatus) const;

    QPointer<QGeoTileFetcherMapbox> m_fetcher;
    QPointer<QNetworkReply> m_networkReply;
    QPointer<QNetworkAccessManager> m_networkManager;
    QNetworkRequest m_request;
    QString m_format;
    bool m_enableLogging{false};
    int m_retriesLeft{0};
};

QT_END_NAMESPACE

#endif // QGEOMAPREPLYMAPBOX_H
