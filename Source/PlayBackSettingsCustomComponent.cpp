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

    juce::StringArray availableNotes="None";
    for (int i = lowestNote; i <= highestNote; i++)
    {
        availableNotes.add(MapHelper::intToStringNote(i));
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
        startNoteBox.setText(MapHelper::intToStringNote(startNote), juce::dontSendNotification);
    }
    else startNoteBox.setText("None",juce::dontSendNotification);
    if (endNote != -1)
    {
        endNoteBox.setText(MapHelper::intToStringNote(endNote), juce::dontSendNotification);
    }
    else endNoteBox.setText("None", juce::dontSendNotification);
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
        settings.startNote=MapHelper::stringToIntNote(startNoteBox.getText());
    }
    else if (comboBoxThatHasChanged == &endNoteBox)
    {
        settings.endNote =MapHelper::stringToIntNote(endNoteBox.getText());
    }

    if(onChangingSettings)
        onChangingSettings(settings);

    PlaybackSettingsIOHelper::saveToFile(IOHelper::getFile("playbackSettings.json"),this->settings);
}