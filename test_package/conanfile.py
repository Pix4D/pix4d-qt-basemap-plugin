from conans import ConanFile, CMake
import os

class QtBasemapPluginTestConan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'

    default_options = {
        'Qt6:qtdeclarative': True,
        'Qt6:qtpositioning': True,
        "Qt6:qtlocation": True,
        'Qt6:qtwebengine': True,
        'Qt6:qtwebchannel': True,
        'Qt6:qtwebsockets': True,
        'Qt6:with_quick': True
    }

    def build(self):
        cmake = CMake(self, parallel=True)
        cmake.configure(build_dir='./')
        cmake.build()
        # Note: Not running from the install target to avoid packaging qt properly

    def requirements(self):
        self.requires('Qt6/[>=6.6.0-3]@pix4d/qt6')
        self.requires("xorg/11.7.7-0@pix4d/qt6")
        self.requires("xkbcommon/1.6.0-0@pix4d/qt6")

    def test(self):
        self.run(os.path.join('bin', 'example'))
