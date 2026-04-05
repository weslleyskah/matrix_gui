# Matrix GUI

A simple C++ application to perform matrix operations built with the Walnut framework.

![Screenshot](img/matrix_gui.png)

### Download

[matrix_gui.exe](https://github.com/weslleyskah/matrix_gui/releases)

### Build

> Run `scripts/Setup.bat` to generate the `.sln` file.

>Open the `.sln` file and run the code on Visual Studio.

### Dependencies

- Visual Studio
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
- [Walnut](https://github.com/StudioCherno/Walnut)
- [ImGui](https://github.com/ocornut/imgui) 
- [Eigen](https://libeigen.gitlab.io/)

### Structure

| | |
| :--- | :--- |
| `WalnutApp/src/WalnutApp.cpp` | Application |
| `Walnut/` | Framework |
| `vendor/` | Dependencies |
| `scripts/Setup.bat` | Script |
| `premake5.lua` | Build |