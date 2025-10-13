/*
  ==============================================================================

    PlayBackSettingsCustomComponent.h
    Created: 10 Oct 2025 10:16:25pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "HelperFunctions.h"

struct PlayBackSettings
{
    int startNote=-1;
    int endNote=-1;
    int leftHandBound = -1;
    int rightHandBound = -1;
    juce::String VID, PID;
};

class PlayBackSettingsComponent: public juce::Component, private juce::ComboBox::Listener {
public:

    std::function<void(PlayBackSettings settings)> onChangingSettings;

    PlayBackSettingsComponent(int lowestNote, int highestNote, const PlayBackSettings& settings);
    
    ~PlayBackSettingsComponent();

    void resized() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void handIntervalsInit();

    void startStopInit();

private:

    int lowestNote, highestNote;
    PlayBackSettings settings;
    juce::ComboBox startNoteBox;
    juce::ComboBox endNoteBox;
    juce::Label startNoteLabel;
    juce::Label endNoteLabel;

    juce::Label leftHandBoundLabel;
    juce::Label rightHandBoundLabel;

    juce::ComboBox leftHandBoundBox;
    juce::ComboBox rightHandBoundBox;
};
