// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeocodingmanagerenginemapbox.h"
#include "qgeocodereplymapbox.h"
#include "qmapboxcommon.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QGeoCodeReply>
#include <QtLocation/QGeoServiceProvider>

QT_BEGIN_NAMESPACE

namespace {
    static const QString geocodingApiV6Url = QStringLiteral("https://api.mapbox.com/search/geocode/v6/")
    static const QString searchTypeForward = QStringLiteral("forward?");
    static const QString searchTypeReverse = QStringLiteral("reverse?");
    static const QStringList allAddressType = {
            QStringLiteral("secondary_address"), // Currently available in the US only
            QStringLiteral("address"),
            QStringLiteral("block"), // Special feature type reserved for Japanese addresses
            QStringLiteral("street"),
            QStringLiteral("neighborhood"),
            QStringLiteral("locality"),
            QStringLiteral("place"),
            QStringLiteral("district"),
            QStringLiteral("postcode"),
            QStringLiteral("region"),
            QStringLiteral("country"),
        };
}

QGeoCodingManagerEngineMapbox::QGeoCodingManagerEngineMapbox(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("useragent")))
            m_userAgent = parameters.value(QStringLiteral("useragent")).toString().toLatin1();
        else
            m_userAgent = mapboxDefaultUserAgent;

    m_accessToken = parameters.value(QStringLiteral("access_token")).toString();
    m_urlPrefix = geocodingApiV6Url;

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineMapbox::~QGeoCodingManagerEngineMapbox()
{
}

// TODO: Fix!!!! XP
QGeoCodeReply* QGeoCodingManagerEngineMapbox::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    QUrlQuery queryItems;

    // If address text() is not generated: a manual setText() has been made.
    if (!address.isTextGenerated()) {
        queryItems.addQueryItem(QStringLiteral("type"), allAddressType.join(QStringLiteral(",")));
        return doSearch(address.text().simplified(), queryItems, bounds);
    }

    QStringList addressString;
    QStringList typeString;

    if (!address.street().isEmpty()) {
        addressString.append(address.street());
        typeString.append(QStringLiteral("address"));
    }

    if (!address.district().isEmpty()) {
        addressString.append(address.district());
        typeString.append(QStringLiteral("district"));
        typeString.append(QStringLiteral("locality"));
        typeString.append(QStringLiteral("neighborhood"));
    }

    if (!address.city().isEmpty()) {
        addressString.append(address.city());
        typeString.append(QStringLiteral("place"));
    }

    if (!address.postalCode().isEmpty()) {
        addressString.append(address.postalCode());
        typeString.append(QStringLiteral("postcode"));
    }

    if (!address.state().isEmpty()) {
        addressString.append(address.state());
        typeString.append(QStringLiteral("region"));
    }

    if (!address.country().isEmpty()) {
        addressString.append(address.country());
        typeString.append(QStringLiteral("country"));
    }

    queryItems.addQueryItem(QStringLiteral("q"), addressString.join(QLatin1Char(',')));
    queryItems.addQueryItem(QStringLiteral("type"), typeString.join(QLatin1Char(',')));
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(1));

    return search(SearchType::FORWARD, queryItems, bounds);
}

QGeoCodeReply* QGeoCodingManagerEngineMapbox::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset);

    const QByteArray encodedAddress = QUrl::toPercentEncoding(address);

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("q"), QString::fromUtf8(encodedAddress));
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    
    return search(SearchType::FORWARD, queryItems, bounds);
}

// TODO: Fix!!!! XP
QGeoCodeReply* QGeoCodingManagerEngineMapbox::reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds)
{
    const QString coordinateString = QString::number(coordinate.longitude()) + QLatin1Char(',') + QString::number(coordinate.latitude());

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("q"), coordinateString);
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(1));

    return search(SearchType::REVERSE, queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::search(const SearchType searchType, QUrlQuery& urlQuery, const QGeoShape& bounds)
{
    const QString searchType = searchType == SearchType::FORWARD ? searchTypeForward : searchTypeReverse;
    QUrl requestUrl(m_urlPrefix + searchType);
    
    queryItems.addQueryItem(QStringLiteral("access_token"), m_accessToken);
    const QString languageCode = QLocale::languageToCode(geoCodingManager->locale().language());
    queryItems.addQueryItem(QStringLiteral("language"), languageCode);

    // TODO: need to check it...
    QGeoRectangle boundingBox = bounds.boundingGeoRectangle();
    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bbox"),
                QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().latitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.topLeft().latitude()));
    }

    requestUrl.setQuery(queryItems);

    QNetworkRequest networkRequest;
    networkRequest.setRawHeader("User-Agent", m_userAgent);
    networkRequest.setUrl(requestUrl);

    return new QGeoCodeReplyMapbox(m_networkManager->get(networkRequest), this);
}

QT_END_NAMESPACE
