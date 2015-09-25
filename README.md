# Webbum
A VP8 (and possibly VP9) WebM video encoder.

It is an interface for ffmpeg.

Some indications to anyone trying to use it at this point:  
* May be buggy  
* Very messy code (set for refactoring)  
* Currently only supports VP8 encoding
* 64kbps Opus audio only
* Incomplete UI, freezes while encoding  
* Some "hidden" features (e.g. specify a width or height of 0 to scale automatically on aspect ratio)

Any feedback is appreciated. Please use the Issues system to leave any constructive comments, questions, feature requests, or bug reports.

# Compilation
On Linux, the following packages are required (tested on Debian Jessie):  
`libavutil-dev`, `libavfilter-dev`, `libavcodec-dev`

On Windows, all dependencies should be provided in the source code. Note that `ffmpeg` and its libraries must be present in the build directory (or the `PATH` variable) in order for the application to run after compiling.

# Dependencies
On Linux, the `ffmpeg` and `qt5` packages are required to run Webbum, and the `fontconfig` package is required for proper subtitle support.

On Windows, the [`Microsoft Visual C++ 2013 Redistributable`](https://www.microsoft.com/en-ca/download/details.aspx?id=40784) package is required to run Webbum. Release builds include `ffmpeg` binaries from Zeranoe.

For custom builds of `ffmpeg`, the following options are recommended (though not tested): `--disable-static`, `--enable-shared`, `--enable-fontconfig`, `--enable-libass`, `--enable-libopus`, `--enable-libvorbis`, `--enable-libvpx`, as well as any other libraries for susceptible input video codecs to process with Webbum.

All other libraries and dependencies should be provided.
