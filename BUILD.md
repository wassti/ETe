## Build Instructions

### windows/msvc

Install Visual Studio Community Edition 2017 or later and compile all projects from solution

`src/win32/msvc2017/wolf.sln`

Default settings are using Visual Studio 2017 with XP compatible toolset.
*Note: Visual Studio 2022 no longer supports the XP toolset.*

Only 32-bit binaries compatible until 64-bit mods become available and a 64-bit etmain is available

Copy resulting exe from `src/win32/msvc2017/output/[Debug|Release]` directory

~~To compile with Vulkan backend - clean solution, right click on `wolf` project, find `Project Dependencies` and select `renderervk` instead of `renderer`~~

---

### ~~windows/mingw~~

**Unavailable - TODO**

...

---

### generic/ubuntu linux/bsd

[CMake](https://cmake.org/) is required to generate make files for this project under *nix based OS'

You may need to run the following commands to install packages (using fresh ubuntu-18.04 installation as example):

```
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt install make gcc cmake gcc-multilib g++-multilib
sudo apt install libsdl2-dev:i386 libcurl4-openssl-dev:i386
```

[Ninja](https://ninja-build.org/) is recommended to be used in conjunction with CMake for faster builds but not required:
```
sudo apt install ninja-build
```

Make a build directory:
```
mkdir build
cd build
```

Configure with CMake (without Ninja):
```
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchains/linux-i686.cmake" -DCMAKE_INSTALL_PREFIX=/path/to/et-installation ..
```

Configure with CMake & Ninja:
```
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchains/linux-i686.cmake" -DCMAKE_INSTALL_PREFIX=/path/to/et-installation ..
```

You may omit the toolchain section if you do not wish to cross compile for 32-bit binaries. Also omit the i386 and multilib portions of the packages you download from apt.

Build:
```
cmake --build . --config Release --parallel
```

Copy the resulting binaries from created build directory or use command:
```
cmake --install .
```

***Note: Most mods available online only support 32-bit binaries. ETJump and ET:Legacy are the only known 64-bit compatible mods available for public playing.***

---

### Arch Linux

Follow generic linux instructions however, use the following package manager commands instead of `apt`:

```
sudo pacman -S gcc make cmake
sudo pacman -S lib32-alsa-lib lib32-libpulse lib32-jack lib32-sdl2 lib32-curl
```

[Ninja](https://ninja-build.org/) is recommended to be used in conjunction with CMake for faster builds but not required:
```
sudo pacman -S ninja
```

See generic linux section for the remainder of build steps.

***Note: Most mods available online only support 32-bit binaries. ETJump and ET:Legacy are the only known 64-bit compatible mods available for public playing.***

---

### raspbery pi os

**Unavailable - TODO**

...

---

### macOS

* install the official SDL2 framework to /Library/Frameworks
* `brew install ninja` if desired, to use [Ninja](https://ninja-build.org/) in place of `make`
* build should work similarly to generic/ubuntu linux/bsd section
* M1/M2 (**aarch64**) compilation not tested at all due to lack of device or Github Action support
* Need testers and someone to help finish supporting the ET mac filesystem differences with regards to packaging and handling of mod binaries
* Probably will skip PPC support but 32/64/aarch64 separate and universal would be ideal.

---

Several CMake options are available for linux/macOS builds:

`BUILD_CLIENT=ON` - build unified client/server executable, enabled by default

`BUILD_DEDSERVER=ON` - build dedicated server executable, enabled by default

`USE_SDL2=ON` - use SDL2 backend for video, audio, input subsystems, enabled by default, enforced for macos

`DYNAMIC_RENDERER=OFF` - do not link single renderer into client binary, compile all enabled renderers as dynamic libraries and allow to switch them on the fly via \cl_renderer cvar, disabled by default

`ENABLE_SPLINES=ON` - support spline camera system, enabled by default

`USE_SYSTEM_JPEG=OFF` - use current system JPEG(-turbo) library, disabled by default

Example:

`cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchains/linux-i686.cmake" -DCMAKE_INSTALL_PREFIX=/path/to/et-installation -DBUILD_DEDSERVER=OFF ..` - Which means don't build the dedicated binary