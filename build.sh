#!/bin/bash

echo "Building Tyler Audio Plugins..."

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "CMake configuration failed"
    exit 1
fi

cmake --build . --config Release --parallel 4
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

echo "Build completed successfully!"
echo "Plugin artifacts are in: build/plugins/ExamplePlugin/ExamplePlugin_artefacts/Release/"