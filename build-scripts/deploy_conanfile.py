from conan import ConanFile
from conan.tools.files import copy, collect_libs

class conanRecipe(ConanFile):
    name = "topaz-ffmpeg"
    settings = "os", "build_type", "arch"

    def configure(self):
        self.options["zimg"].shared = True
        if self.settings.os == "Macos" or self.settings.os == "Linux":
            self.options["libvpx"].shared = True

    def requirements(self):
        self.requires("videoai/1.9.21-oiio3b2")
        self.requires("zimg/3.0.5")
        if self.settings.os == "Macos" or self.settings.os == "Linux":
            self.requires("libvpx/1.11.0") #libvpx is static on Windows
            self.requires("aom/3.5.0")
            
    def package_id(self):
        self.info.requires["videoai"].minor_mode()

    def package(self):
        copy(
            self,
            "*",
            src=self.source_folder,
            dst=self.package_folder,
            keep_path=True,
        )

    def package_info(self):
        # Define individual components for each FFmpeg library
        
        # avutil - core utility library (base for others)
        self.cpp_info.components["avutil"].libs = ["avutil"]
        
        # avcodec - codec library
        self.cpp_info.components["avcodec"].libs = ["avcodec"]
        self.cpp_info.components["avcodec"].requires = ["avutil"]
        
        # avformat - format library
        self.cpp_info.components["avformat"].libs = ["avformat"]
        self.cpp_info.components["avformat"].requires = ["avutil", "avcodec"]
        
        # avfilter - filter library
        self.cpp_info.components["avfilter"].libs = ["avfilter"]
        self.cpp_info.components["avfilter"].requires = ["avutil"]
        
        # avdevice - device library
        self.cpp_info.components["avdevice"].libs = ["avdevice"]
        self.cpp_info.components["avdevice"].requires = ["avutil", "avformat"]
        
        # swscale - scaling library
        self.cpp_info.components["swscale"].libs = ["swscale"]
        self.cpp_info.components["swscale"].requires = ["avutil"]
        
        # swresample - resampling library
        self.cpp_info.components["swresample"].libs = ["swresample"]
        self.cpp_info.components["swresample"].requires = ["avutil"]
        
        # postproc - post-processing library
        self.cpp_info.components["postproc"].libs = ["postproc"]
        self.cpp_info.components["postproc"].requires = ["avutil"]

    def layout(self):
        self.folders.source = self.conf.get("user.profile_name")
