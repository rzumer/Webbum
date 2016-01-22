# Webbum
A VP8 and VP9 WebM video encoder.

It is an interface for ffmpeg.

Known restrictions:  
* 64kbps audio only (Opus/Vorbis codec restricted by the WebM format)  

Any feedback is appreciated. Please use the Issues system to leave any constructive comments, questions, feature requests, or bug reports.

# Compilation
On Linux, the following packages are required (tested on Debian Jessie):  
`libavutil-dev`, `libavfilter-dev`, `libavcodec-dev`

On Windows, all dependencies should be provided in the source code. Note that `ffmpeg` and its libraries must be present in the build directory (or the `PATH` variable) in order for the application to run after compiling.

# Dependencies
On Linux, the `ffmpeg` and `qt5` packages are required to run Webbum, and the `fontconfig` package is required for proper subtitle support.

On Windows, the [`Microsoft Visual C++ 2013 Redistributable`](https://www.microsoft.com/en-ca/download/details.aspx?id=40784) package is required to run Webbum. Release builds include the installers and `ffmpeg` binaries from Zeranoe.

For custom builds of `ffmpeg`, the following options are recommended (though not tested): `--disable-static`, `--enable-shared`, `--enable-fontconfig`, `--enable-libass`, `--enable-libopus`, `--enable-libvorbis`, `--enable-libvpx`, as well as any other libraries for susceptible input video codecs to process with Webbum.

All other libraries and dependencies should be provided.
