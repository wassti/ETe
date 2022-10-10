# ETe

[![build](../../workflows/build/badge.svg)](../../actions?query=workflow%3Abuild) * <a href="https://discord.com/invite/hsDQVby"><img src="https://img.shields.io/discord/253600486219972608?color=7289da&logo=discord&logoColor=white" alt="Discord server" /></a>

This is a modern Wolfenstein: Enemy Territory engine aimed to be fast, secure and compatible with many existing ET mods and etmain.
It is based on the [Quake3e](https://github.com/ec-/Quake3e) engine which itself is based on the last non-SDL source dump of [ioquake3](https://github.com/ioquake/ioq3) with latest upstream fixes applied. Several common fixes from upstream [ET:Legacy](https://github.com/etlegacy/etlegacy) also are applied as needed for security.

Go to [Releases](../../releases) section to download latest binaries for your platform or follow [Build Instructions](#build-instructions).

**Notes**: Skip the **ete-etmain-mod-replacement-allplatform.zip** unless you know what you are doing. You will want **ete-docs.zip** and **ete-platform-name.zip** when downloading prebuilt packages. 64-bit (x86_64) platforms are unstable currently with bugs that do not exist under 32-bit platforms.

*This repository does not contain any game content, so in order to play you must copy the resulting binaries into your existing Wolfenstein: Enemy Territory installation*

**Key features**:

* optimized OpenGL renderer
* ~~optimized Vulkan renderer~~ **NOT YET**
* raw mouse input support, enabled automatically instead of DirectInput(**\in_mouse 1**) if available
* unlagged mouse events processing, can be reverted by setting **\in_lagged 1**
* **\in_minimize** - hotkey for minimize/restore main window (win32-only, direct replacement for Q3Minimizer)
* **\video-pipe** - to use external ffmpeg binary as an encoder for better quality and smaller output files
* improved server-side DoS protection, much reduced memory usage
* raised filesystem limits (up to 20,000 maps can be handled in a single directory)
* reworked Zone memory allocator, no more out-of-memory errors
* non-intrusive support for SDL2 backend (video,audio,input), selectable at compile time for *nix via CMake
* tons of bug fixes and other improvements
* basic Discord Rich Presence support (win32-only currently, linux support to follow when 64-bit support is ready)

## Vulkan renderer

***NOT YET AVAILABLE***

## OpenGL renderer

Based on classic OpenGL renderers from [idq3](https://github.com/id-Software/Quake-III-Arena)/[sdet](https://github.com/id-Software/Enemy-Territory)/[ioquake3](https://github.com/ioquake/ioq3)/[cnq3](https://bitbucket.org/CPMADevs/cnq3)/[openarena](https://github.com/OpenArena/engine), features:

* OpenGL 1.1 compatible, uses features from newer versions whenever available
* high-quality per-pixel dynamic lighting, can be triggered by **\r_dlightMode** cvar
* ~~merged lightmaps (atlases)~~
* static world surfaces cached in VBO (**\r_vbo 1**)
* offscreen rendering, enabled with **\r_fbo 1**, all following requires it enabled:
* `screenMap` texture rendering - to create realistic environment reflections
* multisample anti-aliasing (**\r_ext_multisample**)
* supersample anti-aliasing (**\r_ext_supersample**)
* per-window gamma-correction which is important for screen-capture tools like OBS
* you can minimize game window any time during **\video**|**\video-pipe** recording
* high dynamic range render targets (**\r_hdr 1**) to avoid color banding
* bloom post-processing effect
* arbitrary resolution rendering
* greyscale mode

Performance is usually greater or equal to other opengl1 renderers

## OpenGL2 renderer

Original ioquake3 renderer, not available and unlikely to be ported

## Missing ioquake3 features

* OpenAL(-soft) support
* VoIP support (probably never going to add since Discord etc)
* Extra sound codecs like ogg and opus

## [Build Instructions](BUILD.md)

## Contacts

Discord channel: https://discordapp.com/invite/hsDQVby

## Links

* https://github.com/ec-/Quake3e
* https://github.com/etlegacy/etlegacy
* https://bitbucket.org/CPMADevs/cnq3
* https://github.com/ioquake/ioq3
* https://github.com/kennyalive/Quake-III-Arena-Kenny-Edition
* https://github.com/OpenArena/engine
