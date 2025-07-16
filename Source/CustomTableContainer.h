/*
  ==============================================================================

    CustomTableContainer.h
    Created: 15 Jul 2025 9:27:26pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiNotesTableModel.h"

class TableContainer: public juce::Component
{
public:
    TableContainer(const juce::MidiMessageSequence& seq, const juce::String& displayName, int channel)
    {
        juce::String displayN = "Notes - " + displayName;
        model = std::make_unique<MidiNotesTableModel>(seq, channel);
        table = std::make_unique<juce::TableListBox>(displayN, nullptr);

        model->onUpdate = [this](int rowNumber)
        {
            table->repaintRow(rowNumber);
        };

        table->setModel(model.get());

        table->setMultipleSelectionEnabled(false);

        table->getHeader().addColumn("Note", 1, 100);
        table->getHeader().addColumn("Time", 2, 120);
        table->getHeader().addColumn("Velocity", 3, 100);
        table->getHeader().addColumn("Channel", 4, 80);

        addAndMakeVisible(table.get());
        
        disclaimerLabel = std::make_unique<juce::Label>();
        disclaimerLabel->setText("Note: This view shows the data used in the app only.\nYour original files remain unchanged.", juce::dontSendNotification);
        disclaimerLabel->setJustificationType(juce::Justification::centred);
        disclaimerLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        disclaimerLabel->setFont(juce::Font(12.0f));
        addAndMakeVisible(disclaimerLabel.get());
        table->setBounds(getLocalBounds());
        table->updateContent();
    }

    void resized() override {
        auto area = getLocalBounds();
        auto labelHeight = 40;

        table->setBounds(area.removeFromTop(area.getHeight() - labelHeight));
        disclaimerLabel->setBounds(area);
    }

    ~TableContainer() override
    {
        table->setModel(nullptr);
    }

private:
    std::unique_ptr<juce::TableListBox> table;
    std::unique_ptr<MidiNotesTableModel> model;
    std::unique_ptr<juce::Label> disclaimerLabel;
};