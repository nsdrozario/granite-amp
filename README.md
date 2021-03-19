# Guitar Amp

[![Build Status](https://travis-ci.com/nsdrozario/guitar-amp.svg?branch=main)](https://travis-ci.com/nsdrozario/guitar-amp)

Real-time guitar amplifier simulation.

*WARNING: In this project's current state, there is no guarantee that the audio levels generated from this application are safe, neither for humans nor audio equipment. Use at your own risk.*

## Features

- [x] Distortion/Overdrive
  - [x] Min/max clipping
  - [x] Tanh clipping
- [x] Convolution Reverb

## Dependencies

- [Dear ImGui](https://github.com/ocornut/imgui) (contained within this repository already)
- [Imgui-SFML](https://github.com/eliasdaler/imgui-sfml) (contained within this repository already)
- [imnodes](https://github.com/Nelarius/imnodes) (contained within this repository already)
- [implot](https://github.com/epezent/implot) (contained within this repository already)
- [Miniaudio](https://github.com/mackron/miniaudio) (contained within this repository already)
- [FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver) (contained within this repository already)
- [SFML (2.5)](https://github.com/SFML/SFML)

## Building

## Linux

```bash
make -f Makefile_LINUX -j
```

## Windows (MSYS2)

```bash
make -f Makefile_WINDOWS -j
```


