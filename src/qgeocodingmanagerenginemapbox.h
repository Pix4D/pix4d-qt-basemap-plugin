// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOCODINGMANAGERENGINEMAPBOX_H
#define QGEOCODINGMANAGERENGINEMAPBOX_H

#include <QtLocation/QGeoCodingManagerEngine>

QT_BEGIN_NAMESPACE

class QGeoCodeReply;
class QGeoServiceProvider;
class QNetworkAccessManager;
class QUrlQuery;

class QGeoCodingManagerEngineMapbox : public QGeoCodingManagerEngine
{
    Q_OBJECT
    
public:
    enum class SearchType
    {
        FORWARD,
        REVERSE,
    };

public:
    QGeoCodingManagerEngineMapbox(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                               QString *errorString);
    ~QGeoCodingManagerEngineMapbox();

    QGeoCodeReply* geocode(const QGeoAddress &address, const QGeoShape &bounds = QGeoShape()) override;
    QGeoCodeReply* geocode(const QString &address, int limit = -1, int offset = 0, const QGeoShape &bounds = QGeoShape()) override;
    QGeoCodeReply* reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds = QGeoShape()) override;

private:
    QGeoCodeReply* search(const SearchType searchType, QUrlQuery& urlQuery, const QGeoShape& bounds);

    QNetworkAccessManager* m_networkManager;
    QByteArray m_userAgent;
    QString m_accessToken;
    QString m_urlPrefix;
};

QT_END_NAMESPACE

#endif // QGEOCODINGMANAGERENGINEMAPBOX_H
