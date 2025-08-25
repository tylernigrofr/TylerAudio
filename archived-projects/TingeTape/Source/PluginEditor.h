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

    // Controls
    juce::Slider wowSlider;
    juce::Slider dirtSlider;
    juce::Slider toneSlider;
    juce::Slider lowCutFreqSlider;
    juce::Slider lowCutQSlider;
    juce::Slider highCutFreqSlider;
    juce::Slider highCutQSlider;
    juce::ToggleButton bypassButton { "Bypass" };

    // Labels
    juce::Label wowLabel { {}, "Wow" };
    juce::Label dirtLabel { {}, "Dirt" };
    juce::Label toneLabel { {}, "Tone" };
    juce::Label lowCutFreqLabel { {}, "Low Cut" };
    juce::Label lowCutQLabel { {}, "Low Cut Q" };
    juce::Label highCutFreqLabel { {}, "High Cut" };
    juce::Label highCutQLabel { {}, "High Cut Q" };

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> wowAttachment;
    std::unique_ptr<SliderAttachment> dirtAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> lowCutFreqAttachment;
    std::unique_ptr<SliderAttachment> lowCutQAttachment;
    std::unique_ptr<SliderAttachment> highCutFreqAttachment;
    std::unique_ptr<SliderAttachment> highCutQAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TingeTapeAudioProcessorEditor)
};