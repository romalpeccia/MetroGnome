/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "Utilities.h"

//==============================================================================
/**
*/
class MetroGnomeAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    MetroGnomeAudioProcessorEditor(MetroGnomeAudioProcessor&);
    ~MetroGnomeAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void paintMetronomeMode(juce::Graphics&);
    void paintPolyRhythmMetronomeMode(juce::Graphics&);
    void drawPolyRhythmCircle(juce::Graphics& g, int radius, int width, int height, int X, int Y, int rhythmValue, float radiusSkew, juce::Colour color1, juce::Colour color, int index);
    void paintPolyRhythmMachineMode(juce::Graphics& g);
    void changeMenuButtonColors(juce::TextButton *buttonOn);

    juce::Rectangle<int> getVisualArea();

    void resized() override;
    void timerCallback() override {
        repaint();
    };

    void toggleAudioProcessorChildrenStates();
    void togglePlayState();
    void togglePlayStateOff();
    void togglePlayStateOn();
    void MetroGnomeAudioProcessorEditor::storePolyRhythmMachineParams();

private:
    MetroGnomeAudioProcessor& audioProcessor;

    //persistent comps
    juce::Image logo;
    juce::TextButton playButton{ "Play" };
    juce::TextButton metronomeButton{ "Default" };
    juce::TextButton polyRhythmButton{ "PolyRhythm" };
    juce::TextButton polyMeterButton{ "PolyMeter" };
    juce::TextButton polyRhythmMachineButton{ "PolyRhythm Machine" };
    juce::TextButton placeholderButton{ "Placeholder" };
    juce::TextButton loadPresetButton{ "load preset" };
    juce::TextButton savePresetButton{ "save preset" };

    void loadPreset();
    void savePreset();
    std::unique_ptr<juce::FileChooser> fileChooser;

    //Sliders
    //the attachment attaches an APVTS param to a slider
    RotarySliderWithLabels    bpmSlider, subdivisionSlider, numeratorSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment bpmAttachment, subdivisionAttachment, numeratorAttachment;

    //polyrhythm metronome buttons
    juce::ToggleButton Rhythm1Buttons[MAX_LENGTH];
    juce::ToggleButton Rhythm2Buttons[MAX_LENGTH];


    //polyrhythm machine buttons
    juce::TextButton polyRhythmMachineButtons[MAX_MIDI_CHANNELS][MAX_LENGTH];
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> polyRhythmMachineButtonAttachments[MAX_MIDI_CHANNELS][MAX_LENGTH];
    juce::Slider polyRhythmMachineSubdivisionSliders[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> polyRhythmMachineSubdivisionSliderAttachments[MAX_MIDI_CHANNELS];
    juce::Slider polyRhythmMachineMidiSliders[MAX_MIDI_CHANNELS];
    juce::TextEditor polyRhythmMachineMidiTextEditors[MAX_MIDI_CHANNELS];
    juce::ToggleButton polyRhythmMachineMuteButtons[MAX_MIDI_CHANNELS];
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> polyRhythmMachineMuteButtonAttachments[MAX_MIDI_CHANNELS];

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> polyRhythmMachineMidiSliderAttachments[MAX_MIDI_CHANNELS];


    std::vector<juce::Component*> getVisibleComps();
    std::vector<juce::Component*> getHiddenComps();







    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetroGnomeAudioProcessorEditor)
};


