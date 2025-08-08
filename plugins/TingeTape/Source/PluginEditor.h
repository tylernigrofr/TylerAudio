#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TingeTapeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TingeTapeAudioProcessorEditor(TingeTapeAudioProcessor&);
    ~TingeTapeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TingeTapeAudioProcessor& audioProcessor;
    
    juce::Slider gainSlider;
    juce::Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TingeTapeAudioProcessorEditor)
};