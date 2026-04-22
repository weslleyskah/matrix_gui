cmake_minimum_required(VERSION 3.10)
project(MatrixGui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find Dependencies
find_package(Vulkan REQUIRED)

# 2. Local GLFW Source
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW examples" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW tests" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW docs" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "GLFW install" FORCE)
add_subdirectory(dependencies/GLFW)

# 3. ImGui Source Files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

# 4. Create Executable
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    ${IMGUI_SOURCES}
)

# 5. Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
    ${Vulkan_INCLUDE_DIRS}
)

# 6. Link Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    Vulkan::Vulkan
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi user32 gdi32 shell32)
endif()
