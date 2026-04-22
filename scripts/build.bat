@echo off
setlocal
cd /d "%~dp0.."
set BUILD_DIR=build

if "%~1"=="clean" (
    echo [MatrixGui] Cleaning old build files...
    if exist %BUILD_DIR% rd /s /q %BUILD_DIR%
)

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

cd %BUILD_DIR%
echo [MatrixGui] Configuring project...
cmake ..
if %ERRORLEVEL% NEQ 0 (
    echo [MatrixGui] CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

echo [MatrixGui] Building project (Debug)...
cmake --build . --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo [MatrixGui] Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [MatrixGui] Build successful! 
echo Executable is at: %BUILD_DIR%\Debug\MatrixGui.exe
echo.
pause
