// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOTILEFETCHERMAPBOX_H
#define QGEOTILEFETCHERMAPBOX_H

#include <qlist.h>
#include <QtLocation/private/qgeotilefetcher_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngine;
class QNetworkAccessManager;

class QGeoTileFetcherMapbox : public QGeoTileFetcher
{
    Q_OBJECT

public:
    // The list of map style names:
    // The two values below are part of url allowing to fetch corresponsing mapbox tiles
    static constexpr const char* PIX4D_STREET = "ck8zz9gpq0vty1ip30bji3b5a";
    static constexpr const char* PIX4D_STREETS_SATELLITE = "ck8zzfxb30vwp1jo04yktjtbg";
    // name for custom basemap tiles requests
    static constexpr const char* PIX4D_CUSTOM = "custom";

public:
    QGeoTileFetcherMapbox(int scaleFactor, bool enableLogging, const QString& customBasemapUrl, QGeoTiledMappingManagerEngine *parent);

    void setUserAgent(const QByteArray &userAgent);
    void setMapIds(const QList<QString> &mapIds);
    void setFormat(const QString &format);
    void setAccessToken(const QString &accessToken);

private:
    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) override;

    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_format;
    QString m_replyFormat;
    QString m_accessToken;
    QList<QString> m_mapIds;
    int m_scaleFactor;
    bool m_enableLogging{false};
    QString m_customBasemapUrl;
};

QT_END_NAMESPACE

#endif // QGEOTILEFETCHERMAPBOX_H
