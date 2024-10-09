// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOCODEREPLYMAPBOX_H
#define QGEOCODEREPLYMAPBOX_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>

QT_BEGIN_NAMESPACE

class QGeoCodeReplyMapbox : public QGeoCodeReply
{
    Q_OBJECT

public:
    explicit QGeoCodeReplyMapbox(QNetworkReply *reply, QObject *parent = nullptr);
    ~QGeoCodeReplyMapbox();
    
    QNetworkReply* networkReply();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);
    
private:
    QNetworkReply* m_networkReply{nullptr};
};

QT_END_NAMESPACE

#endif // QGEOCODEREPLYMAPBOX_H
