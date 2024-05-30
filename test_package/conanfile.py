from conans import ConanFile, CMake
import os

class QtBasemapPluginTestConan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'

    default_options = {
        'Qt6:qtdeclarative': True,
        'Qt6:qtpositioning': True,
        "Qt6:qtlocation": True
    }

    def build(self):
        cmake = CMake(self, parallel=True)
        cmake.configure(build_dir='./')
        cmake.build()
        # Note: Not running from the install target to avoid packaging qt properly

    def requirements(self):
        self.requires('Qt6/[6.7.0-0]@pix4d/stable')

    def test(self):
        self.run(os.path.join('bin', 'example'), run_environment=True)
