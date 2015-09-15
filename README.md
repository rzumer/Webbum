# Webbum
A VP8 (and possibly VP9) WebM video encoder.

Some indications to anyone trying to use this at this point:  
* May be buggy or have lower quality output than expected  
* Very messy code (set for refactoring)  
* Missing some features (subtitle support)
* Currently in VP8 mode, so VP9 modes will not function (Constrained Quality + Lossless)  
* Incomplete UI, freezes while encoding  
* Some "hidden" features (e.g. specify a width or height of 0 to scale automatically on aspect ratio)

Any feedback is appreciated. Please use the Issues system to leave any constructive comments, questions, feature requests, or bug reports.

To compile on Linux, the following packages are required (tested on Debian Jessie):  
ffmpeg, libavutil-dev, libavfilter-dev, libavcodec-dev

On Windows, ffmpeg binaries (in ffmpeg/[x86|x64]/bin) must be present in the application's working directory.
