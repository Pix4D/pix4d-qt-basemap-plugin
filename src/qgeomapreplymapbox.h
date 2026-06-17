// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAPREPLYMAPBOX_H
#define QGEOMAPREPLYMAPBOX_H

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtLocation/private/qgeotiledmapreply_p.h>
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

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    void startRequest();
    void connectReply(QNetworkReply *reply);
    bool isRetriableError(int httpStatus) const;

    QPointer<QNetworkAccessManager> m_networkManager;
    QNetworkRequest m_request;
    QString m_format;
    bool m_enableLogging{false};
    int m_retriesLeft{0};
};

QT_END_NAMESPACE

#endif // QGEOMAPREPLYMAPBOX_H
