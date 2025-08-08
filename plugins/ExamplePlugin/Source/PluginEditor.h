#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ExamplePluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ExamplePluginAudioProcessorEditor(ExamplePluginAudioProcessor&);
    ~ExamplePluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ExamplePluginAudioProcessor& audioProcessor;
    
    juce::Slider gainSlider;
    juce::Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExamplePluginAudioProcessorEditor)
};