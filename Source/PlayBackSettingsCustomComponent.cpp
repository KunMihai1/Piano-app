/*
  ==============================================================================

    PlayBackSettingsCustomComponent.cpp
    Created: 10 Oct 2025 10:16:25pm
    Author:  Kisuke

  ==============================================================================
*/

#include "PlayBackSettingsCustomComponent.h"
#include "IOHelper.h"

PlayBackSettingsComponent::PlayBackSettingsComponent(int lowestNote, int highestNote, int startNote, int endNote,
    const juce::String& VID, const juce::String& PID)
{
    auto settingsFile = IOHelper::getFile("playbackSettings.json");
    if (!settingsFile.existsAsFile())
        settingsFile.create();
        


    this->lowestNote = lowestNote;
    this->highestNote = highestNote;

    this->settings.startNote = startNote;
    this->settings.endNote = endNote;
    this->settings.VID = VID;
    this->settings.PID = PID;

    juce::StringArray availableNotes;
    for (int i = lowestNote; i <= highestNote; i++)
    {
        availableNotes.add(juce::String(i));
    }
    addAndMakeVisible(startNoteBox);
    addAndMakeVisible(endNoteBox);

    startNoteBox.clear();
    endNoteBox.clear();

    startNoteBox.addItemList(availableNotes, 1);

    addAndMakeVisible(startNoteLabel);
    startNoteLabel.setText("Start note", juce::dontSendNotification);
    endNoteBox.addItemList(availableNotes, 1);

    addAndMakeVisible(endNoteLabel);
    endNoteLabel.setText("End note", juce::dontSendNotification);


    if (startNote != -1)
    {
        //this->selectedStartNote = settings.startNote;
        startNoteBox.setText(juce::String(startNote), juce::dontSendNotification);
    }
    if (endNote != -1)
    {
        //this->selectedEndNote = settings.endNote;
        endNoteBox.setText(juce::String(endNote), juce::dontSendNotification);
    }

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
        settings.startNote=startNoteBox.getText().getIntValue();
    }
    else if (comboBoxThatHasChanged == &endNoteBox)
    {
        settings.endNote = endNoteBox.getText().getIntValue();
    }

    if(onChangingSettings)
        onChangingSettings(settings);

    PlaybackSettingsIOHelper::saveToFile(IOHelper::getFile("playbackSettings.json"),this->settings);
}