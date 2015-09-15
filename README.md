# Webbum
A VP8 (and possibly VP9) WebM video encoder.

It is an interface for ffmpeg.

Some indications to anyone trying to use it at this point:  
* May be buggy or have lower quality output than expected  
* Very messy code (set for refactoring)  
* Missing some features (subtitle support)
* Currently only supports VP8 encoding
* 64kbps Opus audio only
* Incomplete UI, freezes while encoding  
* Some "hidden" features (e.g. specify a width or height of 0 to scale automatically on aspect ratio)

Any feedback is appreciated. Please use the Issues system to leave any constructive comments, questions, feature requests, or bug reports.

# Compilation
On Linux, the following packages are required (tested on Debian Jessie):  
ffmpeg, libavutil-dev, libavfilter-dev, libavcodec-dev

On Windows, all dependencies should be provided in the source code.

# Releases
Windows release builds include Microsoft Visual C++ Redistributable packages, required to run Webbum.

All other libraries and dependencies should be provided.
