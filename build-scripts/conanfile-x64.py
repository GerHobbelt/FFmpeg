from conan import ConanFile
from conan.tools.files import copy
import os

# NOTE: when updating, place libraries in alphabetical order to reduce diffs


class conanRecipe(ConanFile):
    name = "FFmpeg"
    # version
    settings = "os", "build_type", "arch"

    def configure(self):
        self.options["zimg"].shared = True

    def build_requirements(self):
        if self.settings.os == "Windows" and self.settings.arch == "x86_64":
            self.tool_requires("nasm/2.16.01")

    def requirements(self):
        self.requires("zimg/3.0.5")

    def generate(self):
        for dep in self.dependencies.values():
            if dep.package_folder:
                print(f"copying {dep}: {dep.package_folder} -> {self.build_folder}")
                if self.settings.os == "Windows":
                    # Copy all the libraries to lib3rdparty
                    # Cannot only grab specific types, because for some reason
                    # tensorflow-gpu has c++ headers with no extension
                    copy(self, "*", src=dep.package_folder, dst=os.path.join("lib3rdparty", str(dep.ref).split('/')[0]))
                    # Copy DLLs and Crashpad executables to bin folder
                    copy(self, "*.xml", src=os.path.join(dep.package_folder, "bin"), dst="bin")
                    copy(self, "*.dll", src=os.path.join(dep.package_folder, "bin"), dst="bin")
                    # Copy DLLs and other things from older pre-builts that use binr/bind
                    copy(self, "*", dst="bin", src=os.path.join(dep.package_folder, "binr"))

                if self.settings.os == "Macos":
                    copy(self, "*", src=os.path.join(dep.package_folder, "include"), dst="include")
                    copy(self, "*", src=os.path.join(dep.package_folder, "lib"), dst="lib")
                if self.settings.os == "Linux":
                    copy(self, "*", src=os.path.join(dep.package_folder, "include"), dst="include")
                    copy(self, "*", src=os.path.join(dep.package_folder, "lib"), dst="lib")
