# Matrix GUI

A simple C++ application for performing 3x3 matrix operations built with the Walnut framework.

![Screenshot](img/matrix_gui.png)

>Run `scripts/Setup.bat` to generate the `.sln` file.

### Dependencies

- Visual Studio
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
- [Walnut](https://github.com/StudioCherno/Walnut)
- [Dear ImGui](https://github.com/ocornut/imgui) 

### Structure

| | |
| :--- | :--- |
| `WalnutApp/src/WalnutApp.cpp` | Application and UI |
| `Walnut/` | Framework |
| `vendor/` | Dependencies (Imgui, GLFW, GLM) |
| `scripts/Setup.bat` | Script to generate the solution file |
| `premake5.lua` | Build |