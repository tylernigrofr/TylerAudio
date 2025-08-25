#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_session.hpp>
#include <JuceHeader.h>

// Initialize JUCE for benchmarks
int main(int argc, char* argv[])
{
    juce::initialiseJuce_GUI();
    
    int result = Catch::Session().run(argc, argv);
    
    juce::shutdownJuce_GUI();
    return result;
}