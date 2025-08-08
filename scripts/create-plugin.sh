#!/bin/bash
# Script to create a new plugin from the ExamplePlugin template

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <PluginName>"
    echo "Example: $0 MyAwesomeEQ"
    exit 1
fi

PLUGIN_NAME="$1"
PLUGIN_DIR="plugins/$PLUGIN_NAME"

# Validate plugin name
if [[ ! "$PLUGIN_NAME" =~ ^[A-Za-z][A-Za-z0-9]*$ ]]; then
    echo "Error: Plugin name must start with a letter and contain only letters and numbers"
    exit 1
fi

if [ -d "$PLUGIN_DIR" ]; then
    echo "Error: Plugin directory $PLUGIN_DIR already exists"
    exit 1
fi

echo "Creating new plugin: $PLUGIN_NAME"

# Copy template
cp -r plugins/ExamplePlugin "$PLUGIN_DIR"

# Generate unique 4-character plugin code
PLUGIN_CODE=$(echo "$PLUGIN_NAME" | head -c 4 | tr '[:lower:]' '[:upper:]')
if [ ${#PLUGIN_CODE} -lt 4 ]; then
    # Pad with numbers if name is too short
    PLUGIN_CODE="${PLUGIN_CODE}$(seq -f "%g" 1 $((4 - ${#PLUGIN_CODE})) | tr -d '\n')"
fi

echo "Using plugin code: $PLUGIN_CODE"

# Replace template content in CMakeLists.txt
sed -i.bak "s/ExamplePlugin/$PLUGIN_NAME/g" "$PLUGIN_DIR/CMakeLists.txt"
sed -i.bak "s/Expl/$PLUGIN_CODE/g" "$PLUGIN_DIR/CMakeLists.txt"
rm "$PLUGIN_DIR/CMakeLists.txt.bak"

# Replace template content in source files
find "$PLUGIN_DIR/Source" -name "*.cpp" -o -name "*.h" | while read -r file; do
    sed -i.bak "s/ExamplePlugin/$PLUGIN_NAME/g" "$file"
    rm "$file.bak"
done

# Update plugins/CMakeLists.txt to include new plugin
if ! grep -q "add_subdirectory($PLUGIN_NAME)" plugins/CMakeLists.txt; then
    echo "add_subdirectory($PLUGIN_NAME)" >> plugins/CMakeLists.txt
    echo "Added $PLUGIN_NAME to plugins/CMakeLists.txt"
fi

echo "✅ Plugin $PLUGIN_NAME created successfully!"
echo ""
echo "Next steps:"
echo "1. Edit $PLUGIN_DIR/Source/PluginProcessor.h to customize your plugin"
echo "2. Edit $PLUGIN_DIR/Source/PluginProcessor.cpp to implement your DSP"
echo "3. Edit $PLUGIN_DIR/Source/PluginEditor.cpp to customize the GUI"
echo "4. Run 'cmake --build build' to build your new plugin"
echo ""
echo "Plugin code: $PLUGIN_CODE (make sure this is unique!)"
echo "Plugin directory: $PLUGIN_DIR"