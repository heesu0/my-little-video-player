# my-little-video-player

`my-little-video-player` is a video player implemented as C++14 using FFmpeg and SDL2

## Demo
![demo1](https://user-images.githubusercontent.com/34677157/162398532-4edf5356-e88d-4a67-bea9-38d421a437a7.gif)

## Usage

### Linux
```
$ sudo apt-get install cmake pkg-config libsdl2-dev libavformat-dev libavfilter-dev libavdevice-dev ffmpeg
```

### MacOS
```
$ brew install cmake pkg-config ffmpeg sdl2
```

### Build & Run
```
$ ./init.sh
$ ./build/video_player ./sample_video.mp4
```

### Control
- `Space Key` : Play / Stop
- `Arrow Key` : Forward / Reverse


## TODO
- [x]  logging
- [x]  demuxing
- [x]  decoding
- [x]  video converting
- [x]  video rendering
- [x]  audio sampling
- [x]  audio rendering
- [x]  multi threading
- [x]  video synching
- [x]  audio synching
- [x]  seeking, start, stop
