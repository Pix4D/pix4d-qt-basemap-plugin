from conans import CMake, ConanFile, tools
from conans.tools import os_info
import os
import configparser
from conans.model.version import Version
from conans.errors import ConanInvalidConfiguration
from os import getenv

class QtBasemapPluginConan(ConanFile):
    name = 'QtBasemapPlugin'
    version = '1.0.1-4'
    license = 'LGPL3'
    url = 'http://code.qt.io/cgit/qt/qtlocation.git/tree/src/plugins/geoservices/mapbox?h=5.10'
    description = 'Qt GeoServices plugin for basemaps including MapBox'
    options = {'shared': [True, False], 'lockfile': "ANY"}
    default_options = {'shared': True, 'lockfile': None}
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
    exports_sources = ['CMakeLists.txt', 'src/*']
    short_paths = True  # to circumvent Windows path length limit

    def configure(self):
        del self.settings.compiler.libcxx

    def imports(self):
        if os_info.is_windows:
            destination_folders = "conanLibs", "bin"
            for lib_dir in destination_folders:
                self.copy("*.dll", dst=lib_dir, src="lib")
                self.copy("*.dll", dst=lib_dir, src="bin")

        if os_info.is_linux:
            self.copy("*.so*", dst="conanLibs", src="lib")

        if os_info.is_macos:
            lib_dir = "Frameworks"
            self.copy("*.dylib*", dst=lib_dir, src="lib")
            self.copy("*.framework/*", dst=lib_dir, src="lib")

    def requirements(self):
        if self.options.lockfile:
            self.load_requirements_from_file(str(self.options.lockfile))
        else:
            self.requires('Qt5/[5.12.7]@pix4d/stable')
 
    def load_requirements_from_file(self, path):
        config = configparser.ConfigParser()
        config.read(path)
        for _, ref in config.defaults().items():
            self.requires(ref)

    def build(self):
        cmake = CMake(self, generator='Ninja')
        cmake_args = {
            'VERSION_STRING': self.version,
            'BUILD_SHARED_LIBS': self.options.shared,
            'CMAKE_BUILD_TYPE': self.settings.build_type,
            'CMAKE_INSTALL_PREFIX': self.package_folder,
        }

        if(tools.cross_building(self.settings) and
           str(self.settings.os).lower() == 'android'):
            # this is a workaround for the lack of proper Android support from Conan
            # see https://github.com/conan-io/conan/issues/2856#issuecomment-421036768
            cmake.definitions["CONAN_LIBCXX"] = ""

        cmake.configure(source_dir='.', defs=cmake_args)
        cmake.build(target='install')

    def package_id(self):
        # Make all options and dependencies (direct and transitive) contribute
        # to the package id
        self.info.requires.full_package_mode()
 
        # lockfile is just a way to select the version of the
        # dependencies and it should not contribute to the package_id
        del self.info.options.lockfile

    def package_info(self):
        self.cpp_info.libs = ["qtgeoservices_basemap_pix4d"]
        self.cpp_info.includedirs = ['include']
        self.cpp_info.libdirs += ['plugins/geoservices/']
        self.cpp_info.builddirs += ['share/QtMapboxPlugin/cmake']
