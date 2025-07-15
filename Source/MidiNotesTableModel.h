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
    MidiNotesTableModel(const juce::MidiMessageSequence& sequence ,int channel);

    int getNumRows() override;

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool) override;

private:
    std::vector<const juce::MidiMessageSequence::MidiEventHolder*> noteOnEvents;
    int channel;
};