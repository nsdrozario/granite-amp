# Guitar Amp

Real-time guitar amplifier simulation.

![Screenshot of the app](img/screenshot.png)

*WARNING: In this project's current state, there is no guarantee that the audio levels generated from this application are safe, neither for humans nor audio equipment. Use at your own risk.*

## Features

- Distortion/Overdrive
  - [x] Min/max clipping
  - [x] tanh(x) clipping
  - [x] sin(x) clipping 
- Convolution Reverb
  - Note: Impulse responses not included.
- Compressor
  - [x] Peak activation
  - [ ] RMS activation
  - [ ] Adjustable attack
  - [ ] Adjustable release
  - [x] Adjustable ratio
  - [x] Adjustable threshold (in dB)
- Frequency analyzer (need to double click graph to see spectrum) 
- EQ utilities
  - [x] High/low shelf
  - [ ] Peak filter
  - [ ] Notch filter
  - [ ] Standalone low pass
  - [ ] Standalone high pass
 - Delay
 - Flanger
 - Cabinet Simulation


## Dependencies
- [Dear ImGui](https://github.com/ocornut/imgui) (contained within this repository already)
- [Imgui-SFML](https://github.com/eliasdaler/imgui-sfml) (contained within this repository already)
- [imnodes](https://github.com/Nelarius/imnodes) (contained within this repository already)
- [implot](https://github.com/epezent/implot) (contained within this repository already)
- [Imgui-Addons](https://github.com/gallickgunner/ImGui-Addons) (contained within this repository already)
- [dirent](https://github.com/tronkko/dirent) (contained within this repository already)
- [Miniaudio](https://github.com/mackron/miniaudio) (contained within this repository already)
- [FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver) (contained within this repository already)
    - [AudioFFT](https://github.com/HiFi-LoFi/AudioFFT) (contained within this repository already)
- [SFML (2.5)](https://github.com/SFML/SFML)
- [Lua](https://lua.org)
- [sol2](https://github.com/ThePhD/sol2)

SFML 2.5 can be installed using Homebrew on Mac and `pacman` on MSYS2, but depending on your Linux distribution you may need to compile it from source.

## Building

Note: C++17 required

## Linux

```bash
make -f Makefile_LINUX clean
make -f Makefile_LINUX -j
```

## Windows (MSYS2)

```bash
make -f Makefile_WINDOWS clean
make -f Makefile_WINDOWS -j
```

## Mac

```bash
make -f Makefile_MAC clean
make -f Makefile_MAC -j
```


