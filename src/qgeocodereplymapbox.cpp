// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeocodereplymapbox.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtPositioning/QGeoLocation>

namespace {
    const QString parseChildObjectName(const QJsonObject& object, const QString& key)
    {
        const QJsonObject childObject = object.value(key).toObject();
        if (!childObject.isEmpty())
            return childObject.value(QStringLiteral("name")).toString();
        else
            return QString();
    }

    const QGeoLocation parseGeoLocation(const QJsonObject& object)
    {
        QGeoLocation location;

        const QJsonObject properties = object.value(QStringLiteral("properties")).toObject();
        if (properties.isEmpty())
        {
            qWarning() << "GeoCodingService: Error!";
            return QGeoLocation();
        }

        // Set coordinate latitude and longitude
        const QJsonObject coordinates = properties.value(QStringLiteral("coordinates")).toObject();
        if (coordinates.isEmpty())
        {
            qWarning()
                << "GeoCodingService: could not find any coordinates! Can't continue to parse current JSON object!";
            return QGeoLocation();
        }
        const auto latitude = coordinates.value(QStringLiteral("latitude")).toDouble();
        const auto longitude = coordinates.value(QStringLiteral("longitude")).toDouble();
        location.setCoordinate(QGeoCoordinate(latitude, longitude));

        // Set boundingShape
        // Qt6 introduced QGeoShape instaed of QGeoRectangle. See more
        // https://doc.qt.io/qt-6/qgeolocation.html#boundingShape
        const QJsonArray bBox = properties.value(QStringLiteral("bbox")).toArray();
        if (!bBox.isEmpty())
        {
            const double top = bBox.at(3).toDouble();
            const double left = bBox.at(0).toDouble();
            const double bottom = bBox.at(1).toDouble();
            const double right = bBox.at(2).toDouble();
            location.setBoundingShape(QGeoRectangle(QGeoCoordinate(top, left), QGeoCoordinate(bottom, right)));
        }

        // Define address
        const QString featureType = properties.value(QStringLiteral("feature_type")).toString();
        // address < street < postcode < locality < place < district < region < country
        Q_UNUSED(featureType);

        const QJsonObject contexts = properties.value(QStringLiteral("context")).toObject();
        if (contexts.isEmpty())
        {
            qWarning() << "GeoCodingService: could not find contexts! Can't continue to parse current JSON object!";
            return QGeoLocation();
        }
        const int levelOfContexts = contexts.count();
        Q_UNUSED(levelOfContexts);

        QGeoAddress geoAddress;

        const QJsonObject country = contexts.value(QStringLiteral("country")).toObject();
        geoAddress.setCountry(country.value(QStringLiteral("name")).toString());
        // QGeoAddress::countryCode() must be three charactors according to ISO 3166-1 alpha-3
        // See more about ISO 3166-1 alpha-3 country codes
        // https://en.wikipedia.org/wiki/ISO_3166-1_alpha-3#Officially_assigned_code_elements
        geoAddress.setCountryCode(country.value(QStringLiteral("country_code_alpha_3")).toString());

        geoAddress.setState(parseChildObjectName(contexts, QStringLiteral("region")));
        geoAddress.setCity(parseChildObjectName(contexts, QStringLiteral("place")));
        geoAddress.setDistrict(parseChildObjectName(contexts, QStringLiteral("district")));
        geoAddress.setStreet(parseChildObjectName(contexts, QStringLiteral("street")));
        geoAddress.setPostalCode(parseChildObjectName(contexts, QStringLiteral("postcode")));

        const QJsonObject address = contexts.value(QStringLiteral("address")).toObject();
        if (!address.isEmpty())
        {
            // Check if street and address street_name are same
            if (geoAddress.street() != address.value(QStringLiteral("street_name")).toString())
            {
                qWarning() << "GeoCodingService: street name is invalid! Cannot continue to parse current JSON object!";
                return QGeoLocation();
            }
            geoAddress.setStreetNumber(address.value(QStringLiteral("address_number")).toString());
        }
        location.setAddress(geoAddress);

        // Define extended attributes
        QVariantMap extendedAttributes;

        // Set name of location as an extended attribute
        const QString name = properties.value(QStringLiteral("name")).toString();
        extendedAttributes.insert(QStringLiteral("name"), name);

        // Set full address string as an extended attribute
        const QString fullAddress = properties.value(QStringLiteral("full_address")).toString();
        extendedAttributes.insert(QStringLiteral("full_address"), fullAddress);

        // Set place formatted string as an extended attribute
        const QString placeFormatted = properties.value(QStringLiteral("place_formatted")).toString();
        extendedAttributes.insert(QStringLiteral("place_formatted"), placeFormatted);

        // Set extended attributes
        location.setExtendedAttributes(extendedAttributes);

        return location;
    }
}
QGeoCodeReplyMapbox::QGeoCodeReplyMapbox(QNetworkReply *reply, QObject *parent)
:   QGeoCodeReply(parent)
, m_networkReply(reply)
{
    if (!m_networkReply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        qWarning() << "QGeoCodeReplyMapbox: " << errorString();
        return;
    }
    connect(m_networkReply, &QNetworkReply::finished,
            this, &QGeoCodeReplyMapbox::networkReplyFinished);
    connect(m_networkReply, &QNetworkReply::errorOccurred,
            this, &QGeoCodeReplyMapbox::networkReplyError);
    connect(this, &QGeoCodeReply::aborted, m_networkReply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, m_networkReply, &QObject::deleteLater);
}

QGeoCodeReplyMapbox::~QGeoCodeReplyMapbox()
{
}

QNetworkReply* QGeoCodeReplyMapbox::networkReply()
{
    return m_networkReply;
}

void QGeoCodeReplyMapbox::networkReplyFinished()
{
    qDebug() << "[][][] QGeoCodeReplyMapbox: starts networkReplyFinished()";
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "QGeoCodeReplyMapbox: " << reply->errorString();
        return;
    }

    QList<QGeoLocation> locations;
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject())
    {
        setError(ParseError, QStringLiteral("Response parse error"));
        return;
    }

    const QJsonArray features = document.object().value(QStringLiteral("features")).toArray();
    for (const QJsonValue& feature : features)
    {
        QJsonDocument doc(feature.toObject());
        locations.append(parseGeoLocation(feature.toObject()));
    }

    setLocations(locations);
    setFinished(true);
}

void QGeoCodeReplyMapbox::networkReplyError(QNetworkReply::NetworkError error)
{
    qDebug() << "QGeoCodeReplyMapbox got error";
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (error == QNetworkReply::OperationCanceledError)
        setFinished(true);
    else
        setError(QGeoCodeReply::CommunicationError, reply->errorString());
}
