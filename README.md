# Guitar Amp

Real-time guitar amplifier simulation.

*WARNING: In this project's current state, there is no guarantee that the audio levels generated from this application are safe, neither for humans nor audio equipment. Use at your own risk.*

## Dependencies

- [Dear ImGui](https://github.com/ocornut/imgui) (contained within this repository already)
- [Imgui-SFML](https://github.com/eliasdaler/imgui-sfml) (contained within this repository already)
- [imnodes](https://github.com/Nelarius/imnodes) (contained within this repository already)
- [Miniaudio](https://github.com/mackron/miniaudio) (contained within this repository already)
- [SFML](https://github.com/SFML/SFML)
- [KFR](https://github.com/kfrlib/kfr)

## Building

### Linux

KFR should compile from source smoothly on Linux, so after making sure SFML and KFR are installed, you should be able to simply run
```bash
make clean && make
```
and it should successfully generate the `amp` binary. If linker errors concerning KFR occur, try following the instructions in the Windows section of this README.

### Windows (MinGW/MSYS2)

#### Setting up KFR for this project

As of the time this was written, KFR only supports compilation using Clang.
You can follow the same instructions for "Linux, macOS, other" for building KFR for MinGW/MSYS2 at https://github.com/kfrlib/kfr.
Personally, I had build errors about `ld` being unable to find `-lc`, but adding
```
link_directories(/path/to/libc)
```
to the top of `CMakeLists.txt` before invoking CMake solved it. For MinGW/MSYS2, libc may be located in /usr/lib.

You may need to invoke Ninja without any targets at first to build the C API at first, but after that, you should be able to
run `ninja install` successfully afterwards. Now, this project can be built with the command `make clean && make` while the working directory is the root directory of this repository.
