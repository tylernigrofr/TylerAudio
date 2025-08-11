option(BUILD_EXAMPLE_PLUGIN "Build the ExamplePlugin target" OFF)

if(BUILD_EXAMPLE_PLUGIN)
    add_subdirectory(ExamplePlugin)
endif()

add_subdirectory(TingeTape)
