# Webbum
An advanced VP8, VP9 and AV1 WebM video encoding interface for ffmpeg.

![](https://rzumer.tebako.net/img/webbum1.png?raw=true)
![](https://rzumer.tebako.net/img/webbum2.png?raw=true)

# Features
- Dynamic form completion for quick and responsive adjustments
- High quality default encoding settings
- Support for selecting among multiple video, audio and subtitle streams
- Support for embedding common text and image subtitle formats
- All VP8, VP9 and AV1 features and rate control modes
- Vorbis and Opus audio with VP8 and VP9, Opus audio with AV1
- Target file size and bit rate inputs
- Support for trimming by chapter markers
- Advanced custom parameter input
- Translation support

# Compilation
On Linux, the following packages are required (tested on Debian Jessie):  
`libavutil-dev`, `libavfilter-dev`, `libavcodec-dev`

On Windows, all dependencies should be provided in the source code. 
Note that `ffmpeg` and its libraries must be present in the build directory 
(or the `PATH` environment variable) in order for the application to run after compiling.

A deployment script for Qt components is also provided. 
It will compile translations and copy dependencies to a specified build directory on Windows for MSVC and MinGW compilers.
`wget` and `unzip` (e.g. GnuWin32 ports) are required to deploy ffmpeg libraries automatically.
Edit the variables in `release.bat` based on your Qt installation, then run it *after* compiling in release mode.

# Dependencies
On Linux, the `ffmpeg` and `qt5` packages are required to run Webbum, and the `fontconfig` package is required for proper subtitle support.

On Windows, the [`Microsoft Visual C++ 2013 Redistributable`](https://www.microsoft.com/en-ca/download/details.aspx?id=40784) package is required to run Webbum. Release builds include the appropriate `ffmpeg` binaries from Zeranoe.

For custom builds of `ffmpeg`, the following options are recommended: `--disable-static`, `--enable-shared`, `--enable-fontconfig`, `--enable-libass`, `--enable-libopus`, `--enable-libvorbis`, `--enable-libvpx`, `--enable-libx265`, `--enable-libaom`. These options should cover most typical input content.

All other libraries and dependencies should be provided.
