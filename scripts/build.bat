@echo off
REM ReversiAI_Platform Build Script

echo ========================================
echo   ReversiAI_Platform Build Script
echo ========================================

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring project...
cmake .. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release

REM Build
echo Building project...
cmake --build . --config Release

REM Check if build succeeded
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   Build Successful!
    echo ========================================
    echo Executable: build\Release\ReversiAI_Platform.exe
    echo.
    echo Run the application with:
    echo   .\build\Release\ReversiAI_Platform.exe
) else (
    echo.
    echo ========================================
    echo   Build Failed!
    echo ========================================
)

cd ..
pause
