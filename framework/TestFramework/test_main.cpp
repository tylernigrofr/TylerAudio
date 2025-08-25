#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <JuceHeader.h>

// Initialize JUCE for testing
int main(int argc, char* argv[])
{
    juce::initialiseJuce_GUI();
    
    int result = Catch::Session().run(argc, argv);
    
    juce::shutdownJuce_GUI();
    return result;
}