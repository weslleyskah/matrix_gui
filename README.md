# Matrix GUI

A C++ graphical application to perform matrix and vector operations.

| | |
| --- | --- |
| ![Screen](img/matrix_gui.png) | ![Screen](img/matrix_gui2.png) |

### Download

[MatrixGui.exe](https://github.com/weslleyskah/matrix_gui/releases)

### Run

> Run `scripts/build.bat` to generate the `build`

> Run `matrix_gui/build/Debug/MatrixGui.exe`

> Open the `matrix_gui/build/MatrixGui.slnx`, set `MakeGui.sln` as startup project, and run the code on Visual Studio

### Dependencies

- CMAKE
- [Vulkan](https://vulkan.lunarg.com/sdk/home#windows)
- [ImGui](https://github.com/ocornut/imgui)
- GLFW
- [Eigen](https://libeigen.gitlab.io/)

### Structure

| Folder | Description |
| :--- | :--- |
| `src/` | application |
| `dependencies/` | dependencies |
| `scripts/` | build |
