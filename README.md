# DX9 ImGui Base

Minimalist C++ DLL base using **ImGui v1.92.5 (WIP)** and **MinHook**.

### Setup
* **Menu Toggle:** `INSERT` (Change in `gui.hpp` ~350)
* **Panic/Unload:** `END` (Change in `dllmain.cpp` ~28)
* **Core Logic:** Located in `/src/`

### Why .hpp?
I use headers mainly for ease and readability:
* **No Redundancy:** Stop wasting time copying function signatures between `.cpp` and `.h` files.
* **Instant Logic:** All menu code is right there in the header for quick tweaks without jumping files.
* **Zero Linking Hassle:** Everything is in one place so no weird linker errors during compilation.

### Project Requirements
If you are setting this up manually or don't have the property pages, ensure these are set:

* **Configuration Type:** Dynamic Library (.dll)
* **Windows SDK:** 10.0 (Latest)
* **Platform Toolset:** v143 (Recommended)
* **C++ Language Standard:** ISO C++20 Standard (`/std:c++20`)
* **Character Set:** Use Multi-Byte Character Set
* **Optimization:** Use Link Time Code Generation

### Notable Directory Configs

* **Include Directories:** Must include `$(DXSDK_DIR)Include;`
* **Library Directories:** Must include `$(DXSDK_DIR)Lib\x86;`

### Usage
1. Open in Visual Studio.
2. Build as DLL.
3. Inject into target process.

Example on DX9 game.
![SCEENSHOTS](https://i.ibb.co/MkTffm4M/Black-Ops-MP-z-Sar-LH8eax.png)
