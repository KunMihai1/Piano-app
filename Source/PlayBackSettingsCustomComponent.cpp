/*
  ==============================================================================

    PlayBackSettingsCustomComponent.cpp
    Created: 10 Oct 2025 10:16:25pm
    Author:  Kisuke

  ==============================================================================
*/

#include "PlayBackSettingsCustomComponent.h"
#include "IOHelper.h"

PlayBackSettingsComponent::PlayBackSettingsComponent(int lowestNote, int highestNote, PlayBackSettings& settingsGiven): settings{settingsGiven}
{
    auto settingsFile = IOHelper::getFile("playbackSettings.json");
    if (!settingsFile.existsAsFile())
        settingsFile.create();
        


    this->lowestNote = lowestNote;
    this->highestNote = highestNote;

    startStopInit();
    handIntervalsInit();


}

PlayBackSettingsComponent::~PlayBackSettingsComponent()
{
    startNoteBox.removeListener(this);
    endNoteBox.removeListener(this);
}

void PlayBackSettingsComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    int comboWidth = 85;
    int comboHeight = 20;
    int labelHeight = 10;   
    int spacing = 5;        

    startNoteLabel.setBounds(area.removeFromLeft(comboWidth).withHeight(labelHeight));
    startNoteBox.setBounds(startNoteLabel.getX(), startNoteLabel.getBottom() + spacing, comboWidth, comboHeight);

    area.removeFromLeft(spacing);

    endNoteLabel.setBounds(area.removeFromLeft(comboWidth).withHeight(labelHeight));
    endNoteBox.setBounds(endNoteLabel.getX(), endNoteLabel.getBottom() + spacing, comboWidth, comboHeight);

    area.removeFromLeft(spacing);

    leftHandBoundLabel.setBounds(area.removeFromLeft(comboWidth).withHeight(labelHeight));
    leftHandBoundBox.setBounds(leftHandBoundLabel.getX(), leftHandBoundLabel.getBottom() + spacing, comboWidth, comboHeight);

    area.removeFromLeft(spacing);

    rightHandBoundLabel.setBounds(area.removeFromLeft(comboWidth).withHeight(labelHeight));
    rightHandBoundBox.setBounds(rightHandBoundLabel.getX(), rightHandBoundLabel.getBottom() + spacing, comboWidth, comboHeight);

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
    else if (comboBoxThatHasChanged == &leftHandBoundBox)
    {
        juce::String text = leftHandBoundBox.getText();
        settings.leftHandBound = MapHelper::stringToIntNote(text.fromFirstOccurrenceOf("-",false,false));
    }
    else if (comboBoxThatHasChanged == &rightHandBoundBox)
    {
        juce::String text = rightHandBoundBox.getText();
        settings.rightHandBound = MapHelper::stringToIntNote(text.upToFirstOccurrenceOf("-", false, false));
    }

    if(onChangingSettings)
        onChangingSettings(settings); //this is to call the listeners of the display, midiHandler for example. MidiHandler could've had a reference to the structure PlayBackSettings
                                      //that lives in the Display class, but it's more flexible this way, the design of the midiHandler.

    PlaybackSettingsIOHelper::saveToFile(IOHelper::getFile("playbackSettings.json"),this->settings);
}

void PlayBackSettingsComponent::handIntervalsInit()
{
    juce::StringArray availableRangeLeft= "None";
    juce::StringArray availableRangeRight = "None";

    addAndMakeVisible(leftHandBoundBox);
    addAndMakeVisible(rightHandBoundBox);

    leftHandBoundBox.clear();
    rightHandBoundBox.clear();

    addAndMakeVisible(leftHandBoundLabel);

    leftHandBoundLabel.setText("Left hand", juce::dontSendNotification);

    addAndMakeVisible(rightHandBoundLabel);

    rightHandBoundLabel.setText("Right hand", juce::dontSendNotification);

    leftHandBoundBox.addListener(this);
    rightHandBoundBox.addListener(this);

    juce::String lowestNoteString = MapHelper::intToStringNote(lowestNote);
    juce::String highestNoteString = MapHelper::intToStringNote(highestNote);

    for (int i = lowestNote; i <= highestNote; i++)
    {
        availableRangeLeft.add(lowestNoteString + "-" + MapHelper::intToStringNote(i));
        availableRangeRight.add(MapHelper::intToStringNote(i) + "-" + highestNoteString);
    }
    

    leftHandBoundBox.addItemList(availableRangeLeft, 1);
    rightHandBoundBox.addItemList(availableRangeRight, 1);

    if (settings.leftHandBound != -1)
        leftHandBoundBox.setText(lowestNoteString + "-" + MapHelper::intToStringNote(settings.leftHandBound), juce::dontSendNotification);
    else leftHandBoundBox.setText("None", juce::dontSendNotification);

    if (settings.rightHandBound != -1)
        rightHandBoundBox.setText(MapHelper::intToStringNote(settings.rightHandBound) + "-" + highestNoteString, juce::dontSendNotification);
    else rightHandBoundBox.setText("None", juce::dontSendNotification);
}

void PlayBackSettingsComponent::startStopInit()
{
    juce::StringArray availableNotes = "None";
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


    if (settings.startNote != -1)
    {
        startNoteBox.setText(MapHelper::intToStringNote(settings.startNote), juce::dontSendNotification);
    }
    else startNoteBox.setText("None", juce::dontSendNotification);
    if (settings.endNote != -1)
    {
        endNoteBox.setText(MapHelper::intToStringNote(settings.endNote), juce::dontSendNotification);
    }
    else endNoteBox.setText("None", juce::dontSendNotification);
    startNoteBox.addListener(this);
    endNoteBox.addListener(this);
}

void PlayBackSettingsComponent::setNewSettings(PlayBackSettings& newSettings)
{
    this->settings = newSettings;
    if (settings.startNote != -1)
    {
        startNoteBox.setText(MapHelper::intToStringNote(settings.startNote), juce::dontSendNotification);
    }
    else startNoteBox.setText("None", juce::dontSendNotification);
    if (settings.endNote != -1)
    {
        endNoteBox.setText(MapHelper::intToStringNote(settings.endNote), juce::dontSendNotification);
    }
    else endNoteBox.setText("None", juce::dontSendNotification);


    juce::String lowestNoteString = MapHelper::intToStringNote(lowestNote);
    juce::String highestNoteString = MapHelper::intToStringNote(highestNote);

    if (settings.leftHandBound != -1)
        leftHandBoundBox.setText(lowestNoteString + "-" + MapHelper::intToStringNote(settings.leftHandBound), juce::dontSendNotification);
    else leftHandBoundBox.setText("None", juce::dontSendNotification);

    if (settings.rightHandBound != -1)
        rightHandBoundBox.setText(MapHelper::intToStringNote(settings.rightHandBound) + "-" + highestNoteString, juce::dontSendNotification);
    else rightHandBoundBox.setText("None", juce::dontSendNotification);

    if (onChangingSettings)
        onChangingSettings(settings);
}

void PlayBackSettingsComponent::setLowestHighest(int lowest, int highest)
{
    this->lowestNote = lowest;
    this->highestNote = highest;
}