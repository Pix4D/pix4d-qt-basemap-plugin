from conans import CMake, ConanFile, tools
import os


class QtBasemapPluginConan(ConanFile):
    name = 'QtBasemapPlugin'
    version = '1.0.1-4'
    license = 'LGPL3'
    url = 'http://code.qt.io/cgit/qt/qtlocation.git/tree/src/plugins/geoservices/mapbox?h=5.10'
    description = 'Qt GeoServices plugin for basemaps including MapBox'
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
    exports_sources = ['CMakeLists.txt', 'src/*']

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

    def build_requirements(self):
        self.build_requires('Qt5/[5.12.7-2]@pix4d/stable')

    def configure(self):
        del self.settings.compiler.libcxx

    def package_id(self):
        # Make all options and dependencies (direct and transitive) contribute
        # to the package id
        self.info.requires.full_package_mode()

    def package_info(self):
        self.cpp_info.libs = ["qtgeoservices_basemap_pix4d"]
        self.cpp_info.includedirs = ['include']
        self.cpp_info.libdirs += ['plugins/geoservices/']
        self.cpp_info.builddirs += ['share/QtMapboxPlugin/cmake']
