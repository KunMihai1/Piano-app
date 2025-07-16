/*
  ==============================================================================

    MidiNotesTableModel.h
    Created: 15 Jul 2025 8:46:13pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MidiNotesTableModel: public juce::TableListBoxModel
{
public:
    std::function<void(int rowNumber)> onUpdate;

    MidiNotesTableModel(const juce::MidiMessageSequence& sequence ,int channel);

    int getNumRows() override;

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool) override;
    
    //juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

    int getMidiNoteNumberFromName(const juce::String& name, int octaveMiddleC = 4);

    int findFirstDigitName(const juce::String& name);

private:
    std::vector<const juce::MidiMessageSequence::MidiEventHolder*> noteOnEvents;
    int channel;
};