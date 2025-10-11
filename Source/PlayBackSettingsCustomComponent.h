/*
  ==============================================================================

    PlayBackSettingsCustomComponent.h
    Created: 10 Oct 2025 10:16:25pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct PlayBackSettings
{
    int startNote=-1;
    int endNote=-1;
    juce::String VID, PID;
};

class PlayBackSettingsComponent: public juce::Component, private juce::ComboBox::Listener {
public:
    PlayBackSettingsComponent(int lowestNote, int highestNote, const juce::String& VID, const juce::String& PID);

    ~PlayBackSettingsComponent();

    void resized() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:

    int lowestNote, highestNote;
    PlayBackSettings settings;
    juce::String VID, PID;
    juce::ComboBox startNoteBox;
    juce::ComboBox endNoteBox;
    juce::Label startNoteLabel;
    juce::Label endNoteLabel;
};
