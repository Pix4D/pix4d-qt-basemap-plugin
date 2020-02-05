#include "GeoServiceProviderPlugin.h"
#include "GeoTiledMappingManagerEngine.h"

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

QGeoCodingManagerEngine* GeoServiceProviderFactory::createGeocodingManagerEngine(
    const QVariantMap& parameters,
    QGeoServiceProvider::Error* error,
    QString* errorString) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(error)
    Q_UNUSED(errorString)

    return nullptr;
}

QGeoMappingManagerEngine* GeoServiceProviderFactory::createMappingManagerEngine(
    const QVariantMap& parameters,
    QGeoServiceProvider::Error* error,
    QString* errorString) const
{
    return new GeoTiledMappingManagerEngine(parameters, error, errorString);
}

QGeoRoutingManagerEngine* GeoServiceProviderFactory::createRoutingManagerEngine(
    const QVariantMap& parameters,
    QGeoServiceProvider::Error* error,
    QString* errorString) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(error)
    Q_UNUSED(errorString)

    return nullptr;
}

QPlaceManagerEngine* GeoServiceProviderFactory::createPlaceManagerEngine(const QVariantMap& parameters,
                                                                         QGeoServiceProvider::Error* error,
                                                                         QString* errorString) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(error)
    Q_UNUSED(errorString)

    return nullptr;
}
