#include "PluginProcessor.h"
#include "PluginEditor.h"

ExamplePluginAudioProcessorEditor::ExamplePluginAudioProcessorEditor(ExamplePluginAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, true);
    
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.5);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 90, 20);
    
    addAndMakeVisible(&gainSlider);
    addAndMakeVisible(&gainLabel);

    setSize(400, 300);
}

ExamplePluginAudioProcessorEditor::~ExamplePluginAudioProcessorEditor()
{
}

void ExamplePluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Tyler Audio Example Plugin", getLocalBounds(), juce::Justification::centredTop, 1);
}

void ExamplePluginAudioProcessorEditor::resized()
{
    gainSlider.setBounds(100, 100, getWidth() - 150, 20);
}