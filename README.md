# ETe

This is a modern Wolfenstein: Enemy Territory engine aimed to be fast, secure and compatible with many existing ET mods and etmain.
It is based on the Quake3e engine which itself is based on the last non-SDL source dump of ioquake3 with latest upstream fixes applied. Several common fixes from upstream [ET:Legacy](https://github.com/etlegacy/etlegacy) also are applied as needed for security.

**Key features**:

* optimized OpenGL renderer with per-pixel dynamic lights, bloom, antialiasing, greyscale options, etc.
* ~~optimized Vulkan renderer~~ **NOT YET**
* improved server-side DoS protection, much reduced memory usage
* raised filesystem limits (up to 20,000 maps can be handled in a single directory)
* reworked Zone memory allocator, no more out-of-memory errors
* ~~non-intrusive support for SDL2 backend (video,audio,input), selectable at compile time~~ **NOT YET**
* tons of bugfixes and other improvements

*This repository do not contains any game content so in order to play you must copy resulting binaries into your existing Wolfenstein: Enemy Territory installation*

**Missing ioquake3 features**:
* OpenAL support
* VoIP support (probably never going to add since Discord etc)
* Extra sound codecs
* renderer2

## Build Instructions

### windows/msvc 

Install Visual Studio Community Edition 2017 or later and compile all projects from solution

`src/wolf.sln`

Default settings are using Visual Studio 2017 with XP compatible toolset

Only 32-bit binaries compatible until 64-bit mods become available and a 64-bit etmain is available

Copy resulting exe from `src/[Debug|Release]` directory and `src/ded/[Debug|Release]` directory

~~To compile with Vulkan backend - clean solution, right click on `wolf` project, find `Project Dependencies` and select `renderervk` instead of `renderer`~~

### ~~windows/mingw~~

WIP

~~All build dependencies (libraries, headers) are bundled-in~~

~~Build with either `make ARCH=x86` or `make ARCH=x86_64` commands depending from your target system, then copy resulting binaries from created `build` directory or use command: ~~

~~`make install DESTDIR=<path_to_game_files>`~~

### linux/bsd

WIP

Only 32-bit binaries compatible until 64-bit mods become available and a 64-bit etmain is available

Use `scons`

~~You may need to run following commands to install packages (using fresh ubuntu-18.04 installation as example):~~

* ~~sudo apt install make gcc libcurl4-openssl-dev mesa-common-dev~~
* ~~sudo apt install libxxf86dga-dev libxrandr-dev libxxf86vm-dev libasound-dev~~
* ~~sudo apt install libsdl2-dev~~

~~Build with: `make`~~

~~Copy resulting binaries from created `build` directory or use command:~~

~~`make install DESTDIR=<path_to_game_files>`~~

~~Several make options available for linux/mingw builds:~~

~~`BUILD_CLIENT=1` - build unified client/server executable, enabled by default~~
~~`BUILD_SERVER=1` - build dedicated server executable, enabled by default~~

~~`USE_SDL=0`- use SDL2 backend for video, audio, input subsystems, disabled by default~~
~~`USE_VULKAN=0` - link client with vulkan renderer instead of OpenGL, disabled by default~~

~~`USE_RENDERER_DLOPEN=0` - do not link single renderer into client binary, compile all renderers as dynamic libraries and allow to switch them on the fly via `\cl_renderer` cvar, disabled by default - *not recommented due to not stable renderer API*~~

~~Example:~~

~~`make BUILD_SERVER=0 USE_VULKAN=1` - which means do not build dedicated binary, build client with static vulkan renderer~~

### macOS

**UNSUPPORTED** Patches and pull requests welcome

## Links

* https://github.com/ec-/Quake3e
* https://github.com/etlegacy/etlegacy
* https://bitbucket.org/CPMADevs/cnq3
* https://github.com/ioquake/ioq3
* https://github.com/kennyalive/Quake-III-Arena-Kenny-Edition
* https://github.com/OpenArena/engine
