from conans import CMake, ConanFile, tools
import os


class QtBasemapPluginConan(ConanFile):
    name = 'QtBasemapPlugin'
    version = '2.0.0-6'
    license = 'LGPL3'
    url = 'http://code.qt.io/cgit/qt/qtlocation.git/tree/src/plugins/geoservices/mapbox?h=6.6.3'
    description = 'Qt GeoServices plugin for basemaps including MapBox'
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
    exports_sources = ['CMakeLists.txt', 'src/*']

    default_options = {
        'Qt6:qtdeclarative': True,
        'Qt6:qtpositioning': True,
        "Qt6:qtlocation": True,
    }

    def build(self):
        cmake = CMake(self, parallel=True)
        cmake_args = {
            'BUILD_SHARED_LIBS': True
        }

        if(tools.cross_building(self.settings) and
           str(self.settings.os).lower() == 'android'):
            # this is a workaround for the lack of proper Android support from Conan
            # see https://github.com/conan-io/conan/issues/2856#issuecomment-421036768
            cmake.definitions["CONAN_LIBCXX"] = ""

        cmake.configure(source_dir='.', defs=cmake_args)
        cmake.build(target='install')

    def requirements(self):
        self.requires('Qt6/[6.6.3-0]@pix4d/stable')

    def configure(self):
        del self.settings.compiler.libcxx

    def package_id(self):
        # Make all options and dependencies (direct and transitive) contribute
        # to the package id
        self.info.requires.full_package_mode()

    def package(self):
        self.copy("*.h", dst="include/QtBasemapHelpers", src="QtBasemapHelpers/include/QtBasemapHelpers")
        self.copy("*QtBasemapHelpers*.", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["qtgeoservices_basemap_pix4d", "QtBasemapHelpers"]
        self.cpp_info.includedirs = ['include']
        self.cpp_info.libdirs += ['plugins/geoservices/', 'lib']
        self.cpp_info.builddirs += ['share/QtMapboxPlugin/cmake', 'share/QtBasemapHelpers/cmake']
