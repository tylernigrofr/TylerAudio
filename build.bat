@echo off
echo Building Tyler Audio Plugins...

if not exist build mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed
    exit /b 1
)

cmake --build . --config Release --parallel 4
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    exit /b 1
)

echo Build completed successfully!
echo Plugin artifacts are in: build/plugins/ExamplePlugin/ExamplePlugin_artefacts/Release/
pause