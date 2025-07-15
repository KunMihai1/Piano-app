/*
  ==============================================================================

    MidiNotesTableModel.cpp
    Created: 15 Jul 2025 8:46:13pm
    Author:  Kisuke

  ==============================================================================
*/

#include "MidiNotesTableModel.h"

MidiNotesTableModel::MidiNotesTableModel(const juce::MidiMessageSequence& seq, int ch): channel{ch}
{
    for (int i = 0; i < seq.getNumEvents(); ++i)
    {
        const auto* event = seq.getEventPointer(i);
        if (event != nullptr && event->message.isNoteOn())
            noteOnEvents.push_back(event);
    }
}

int MidiNotesTableModel::getNumRows()
{
    return static_cast<int>(noteOnEvents.size());
}

void MidiNotesTableModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else g.fillAll(juce::Colours::lightgrey);
}

void MidiNotesTableModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool isSelected)
{
    if (rowNumber<0  || rowNumber>=noteOnEvents.size()) return;
    auto* event = noteOnEvents[rowNumber];

    if (event==nullptr) 
        return;

    const auto& msg = event->message;
    juce::String text;

    if (msg.isNoteOn())
    {
        switch (columnId)
        {
        case 1: text = juce::MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 3); break;
        case 2: text = juce::String(msg.getTimeStamp(), 6); break;
        case 3: text = juce::String(msg.getVelocity()); break;
        case 4: text = juce::String(channel); break;
        default: break;
        }
    }

    g.setColour(juce::Colours::black);
    g.setFont(14.0f);
    g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft);
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(0, 0, width, height);
}