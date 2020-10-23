#include <QCoreApplication>
#include <QGeoServiceProvider>

#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
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
    return EXIT_SUCCESS;
}
