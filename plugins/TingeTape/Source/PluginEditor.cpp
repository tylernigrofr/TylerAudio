#include "PluginProcessor.h"
#include "PluginEditor.h"

TingeTapeAudioProcessorEditor::TingeTapeAudioProcessorEditor(TingeTapeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    auto& params = audioProcessor.getParameters();

    auto setupSlider = [] (juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    };

    setupSlider(wowSlider);
    setupSlider(dirtSlider);
    setupSlider(toneSlider);
    setupSlider(lowCutFreqSlider);
    setupSlider(lowCutQSlider);
    setupSlider(highCutFreqSlider);
    setupSlider(highCutQSlider);

    // Attachments
    wowAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kWow, wowSlider);
    dirtAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kDirt, dirtSlider);
    toneAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kTone, toneSlider);
    lowCutFreqAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kLowCutFreq, lowCutFreqSlider);
    lowCutQAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kLowCutRes, lowCutQSlider);
    highCutFreqAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kHighCutFreq, highCutFreqSlider);
    highCutQAttachment = std::make_unique<SliderAttachment>(params, TylerAudio::ParameterIDs::kHighCutRes, highCutQSlider);
    bypassAttachment = std::make_unique<ButtonAttachment>(params, TylerAudio::ParameterIDs::kBypass, bypassButton);

    // Labels
    auto setupLabel = [] (juce::Label& l, juce::Component& target) {
        l.setJustificationType(juce::Justification::centredLeft);
        l.attachToComponent(&target, true);
    };
    setupLabel(wowLabel, wowSlider);
    setupLabel(dirtLabel, dirtSlider);
    setupLabel(toneLabel, toneSlider);
    setupLabel(lowCutFreqLabel, lowCutFreqSlider);
    setupLabel(lowCutQLabel, lowCutQSlider);
    setupLabel(highCutFreqLabel, highCutFreqSlider);
    setupLabel(highCutQLabel, highCutQSlider);

    // Add components
    addAndMakeVisible(wowSlider);
    addAndMakeVisible(dirtSlider);
    addAndMakeVisible(toneSlider);
    addAndMakeVisible(lowCutFreqSlider);
    addAndMakeVisible(lowCutQSlider);
    addAndMakeVisible(highCutFreqSlider);
    addAndMakeVisible(highCutQSlider);
    addAndMakeVisible(bypassButton);

    addAndMakeVisible(wowLabel);
    addAndMakeVisible(dirtLabel);
    addAndMakeVisible(toneLabel);
    addAndMakeVisible(lowCutFreqLabel);
    addAndMakeVisible(lowCutQLabel);
    addAndMakeVisible(highCutFreqLabel);
    addAndMakeVisible(highCutQLabel);

    setSize(520, 260);
}

TingeTapeAudioProcessorEditor::~TingeTapeAudioProcessorEditor()
{
}

void TingeTapeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.setColour(juce::Colours::orange);
    g.drawFittedText("TingeTape", { 10, 6, getWidth() - 20, 20 }, juce::Justification::centred, 1);
}

void TingeTapeAudioProcessorEditor::resized()
{
    // Simple two-column layout
    const int labelWidth = 90;
    const int rowHeight = 28;
    const int x0 = labelWidth + 16;
    const int w = getWidth() - x0 - 12;
    int y = 34;

    wowSlider.setBounds(x0, y, w, 20);              y += rowHeight;
    dirtSlider.setBounds(x0, y, w, 20);             y += rowHeight;
    toneSlider.setBounds(x0, y, w, 20);             y += rowHeight;
    lowCutFreqSlider.setBounds(x0, y, w, 20);       y += rowHeight;
    lowCutQSlider.setBounds(x0, y, w, 20);          y += rowHeight;
    highCutFreqSlider.setBounds(x0, y, w, 20);      y += rowHeight;
    highCutQSlider.setBounds(x0, y, w, 20);         y += rowHeight;
    bypassButton.setBounds(x0, y + 4, 100, 20);
}