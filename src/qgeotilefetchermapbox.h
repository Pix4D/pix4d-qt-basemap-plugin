// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOTILEFETCHERMAPBOX_H
#define QGEOTILEFETCHERMAPBOX_H

#include <qlist.h>
#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QHash>
#include <QPointer>
#include <QUrlQuery>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngine;
class QNetworkAccessManager;
class QNetworkReply;

class QGeoTileFetcherMapbox : public QGeoTileFetcher
{
    Q_OBJECT

public:
    // The list of map style names:
    static constexpr const char* PIX4D_STREETS = "streets";
    static constexpr const char* PIX4D_SATELLITE = "satellite";
    static constexpr const char* PIX4D_CUSTOM = "custom";
    static constexpr const char* PIX4D_NONE = "none";

public:
    QGeoTileFetcherMapbox(int scaleFactor, bool enableLogging, const QString& customBasemapUrl, QGeoTiledMappingManagerEngine *parent);

    void setUserAgent(const QByteArray &userAgent);
    void setMapIds(const QList<QString> &mapIds);
    void setFormat(const QString &format);
    void setAdditionalParameters(const QVariantMap& parameters);

    // Lets the download of a cancelled tile finish into the tile cache. Returns false if the caller must abort it.
    bool adoptCancelledDownload(QNetworkReply *reply, const QGeoTileSpec &spec);

private:
    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) override;
    void cancelledDownloadFinished(QNetworkReply *reply, const QGeoTileSpec &spec);

    QGeoTiledMappingManagerEngine *m_engine;
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_format;
    QString m_replyFormat;
    QList<QString> m_mapIds;
    int m_scaleFactor;
    bool m_enableLogging{false};
    QString m_customBasemapUrl;
    QUrlQuery m_query;
    QHash<QGeoTileSpec, QPointer<QNetworkReply>> m_cancelledDownloads;
};

QT_END_NAMESPACE

#endif // QGEOTILEFETCHERMAPBOX_H
