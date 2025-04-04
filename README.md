# Koala Game Engine

<img src="images/koala_engine_logo.png?raw=true" alt="Kohi Logo" height=100/>

This work represents a novel atempt at creating a 3D game engine developed in C++ (mostly C style C++). The engine does not use external third-party libraries and seeks to have as little overhead as possible, i. e. polymorphism, classes and inheritance, smart pointers etc. 

The engine is named Koala Engine as a tribute to the lovable and resilient nature of koalas. Just like how a koala thrives in diverse environments with adaptability and charm, Koala Engine is designed to be flexible and robust, capable of supporting developers in creating a wide variety of 3D games. The name reflects the engine's focus on being user-friendly yet powerful—much like the calm and dependable koala, the engine is built to be a reliable companion in your game development journey. Plus, who wouldn't love the idea of a koala helping you build your dream game?

## Important links

- Xlib Manual: https://tronche.com/gui/x/xlib
- Vulkan API: https://docs.vulkan.org/spec/latest/chapters/introduction.html
- XCB Documentation: https://xcb.freedesktop.org/
- XCB Tutorials: https://www.x.org/releases/current/doc/libxcb/tutorial/index.html
- ASCII code chart: https://www.ascii-code.com/
- Windows key codes: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

## Roadmap

See [here](TODO.md).

### Prerequisites for Windows

- CMake for Windows: `winget install -e --id Kitware.CMake` 
- Visual Studio Build Tools: `winget install Microsoft.VisualStudio.2022.BuildTools`
- Git for Windows: `winget install git.git` OR https://gitforwindows.org/
- Vulkan SDK: `winget install khronosgroup.vulkansdk` OR download from https://vulkan.lunarg.com/

### Prerequisites for Linux

Install these via package manager:

- `sudo apt install llvm` or `sudo pacman -S llvm`
- `sudo apt install git` or `sudo pacman -S git`
- `sudo apt install cmake`

Required for X11:

- `sudo apt install libx11-dev`
- `sudo apt install libxkbcommon-x11-dev`
- `sudo apt install libx11-xcb-dev`

## Contributions

TODO: I will specify some protocol for contributing with PR for the engine in a later moment, once the engine receives a bit more clout :)
