/*
  ==============================================================================

    PlayBackSettingsCustomComponent.cpp
    Created: 10 Oct 2025 10:16:25pm
    Author:  Kisuke

  ==============================================================================
*/

#include "PlayBackSettingsCustomComponent.h"
#include "IOHelper.h"

PlayBackSettingsComponent::PlayBackSettingsComponent(int lowestNote, int highestNote, const juce::String& VID, const juce::String& PID)
{
    auto settingsFile = IOHelper::getFile("playbackSettings.json");
    if (!settingsFile.existsAsFile())
        settingsFile.create();
        
    settings = PlaybackSettingsIOHelper::loadFromFile(IOHelper::getFile("playbackSettings.json"),VID,PID);

    this->lowestNote = lowestNote;
    this->highestNote = highestNote;
    juce::StringArray availableNotes;
    for (int i = lowestNote; i <= highestNote; i++)
    {
        availableNotes.add(juce::String(i));
    }

    addAndMakeVisible(startNoteBox);
    startNoteBox.addItemList(availableNotes, 1);

    addAndMakeVisible(startNoteLabel);
    startNoteLabel.setText("Start note", juce::dontSendNotification);

    addAndMakeVisible(endNoteBox);
    endNoteBox.addItemList(availableNotes, 1);

    addAndMakeVisible(endNoteLabel);
    endNoteLabel.setText("End note", juce::dontSendNotification);

    if (settings.startNote != -1)
    {
        this->selectedStartNote = settings.startNote;
        startNoteBox.setText(juce::String(settings.startNote), juce::dontSendNotification);
    }
    if (settings.endNote != -1)
    {
        this->selectedEndNote = settings.endNote;
        endNoteBox.setText(juce::String(settings.endNote), juce::dontSendNotification);
    }
    this->VID = VID;
    this->PID = PID;

    startNoteBox.addListener(this);
    endNoteBox.addListener(this);
}

PlayBackSettingsComponent::~PlayBackSettingsComponent()
{
    startNoteBox.removeListener(this);
    endNoteBox.removeListener(this);
}

void PlayBackSettingsComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    int comboWidth = 100;
    int comboHeight = 20;
    int labelHeight = 10;   
    int spacing = 5;        

    startNoteLabel.setBounds(area.removeFromLeft(comboWidth).withHeight(labelHeight));
    startNoteBox.setBounds(startNoteLabel.getX(), startNoteLabel.getBottom() + spacing, comboWidth, comboHeight);

    area.removeFromLeft(spacing);

    endNoteLabel.setBounds(area.removeFromLeft(comboWidth).withHeight(labelHeight));
    endNoteBox.setBounds(endNoteLabel.getX(), endNoteLabel.getBottom() + spacing, comboWidth, comboHeight);

}

void PlayBackSettingsComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{   
    if (comboBoxThatHasChanged == &startNoteBox)
    {
        selectedStartNote = startNoteBox.getText().getIntValue();
    }
    else if (comboBoxThatHasChanged == &endNoteBox)
    {
        selectedEndNote = endNoteBox.getText().getIntValue();
    }

    PlayBackSettings settings;
    settings.startNote = selectedStartNote;
    settings.endNote = selectedEndNote;
    settings.VID = VID;
    settings.PID = PID;

    PlaybackSettingsIOHelper::saveToFile(IOHelper::getFile("playbackSettings.json"),settings);
}