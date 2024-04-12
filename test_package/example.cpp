#include <QCoreApplication>
#include <QGeoServiceProvider>

#include <iostream>

#include <QtBasemapHelpers/QQuickGeoMapGestureArea.h>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins/");
    
    if (!QGeoServiceProvider::availableServiceProviders().contains(QStringLiteral("basemap_pix4d"), Qt::CaseInsensitive))
    {
        std::cout << "ERROR locating plugin!" << std::endl;
        return EXIT_FAILURE;
    }

    QGeoServiceProvider plugin("basemap_pix4d");
    if (plugin.error())
    {
        std::cout << "ERROR loading plugin!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Plugin available" << std::endl;

    std::unique_ptr<QQuickGeoMapGestureArea> ptr(new QQuickGeoMapGestureArea(nullptr));
    QObject::connect(ptr.get(), &QQuickGeoMapGestureArea::panActiveChanged, []() { 
        std::cout << "Connetion is working";
    });


    return EXIT_SUCCESS;
}
