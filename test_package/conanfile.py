from conans import ConanFile, CMake
import os

class QtBasemapPluginTestConan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'

    def build(self):
        cmake = CMake(self, parallel=True)
        cmake.configure(build_dir='./')
        cmake.build()
        # Note: Not running from the install target to avoid packaging qt properly

    def requirements(self):
        self.requires('Qt5/[5.12.7-2]@pix4d/stable')

    def test(self):
        self.run(os.path.join('bin', 'example'))
