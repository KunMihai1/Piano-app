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
    std::function<void()> updateToFile;

    TableContainer(const juce::MidiMessageSequence& seq, const juce::String& displayName, int channel, std::unordered_map<int,MidiChangeInfo>& map)
    {
        juce::String displayN = "Notes - " + displayName;
        model = std::make_unique<MidiNotesTableModel>(seq, channel,map);
        table = std::make_unique<juce::TableListBox>(displayN, nullptr);

        model->onUpdate = [this](int rowNumber)
        {
            table->repaintRow(rowNumber);
            if (updateToFile)
                updateToFile();
        };

        table->setModel(model.get());

        table->setMultipleSelectionEnabled(false);

        table->getHeader().addColumn("Note", 1, 100);
        table->getHeader().addColumn("Time", 2, 120);
        table->getHeader().addColumn("Velocity", 3, 100);
        table->getHeader().addColumn("Channel", 4, 80);

        addAndMakeVisible(table.get());
        
        changeMultipleButton = std::make_unique<juce::TextButton>("Apply");
        changeMultipleButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);

        changeMultipleButton->onClick = [this]()
        {
            int selectedID = actionCB->getSelectedId();
            if (selectedID == 1)
            {

            }
            else if (selectedID == 2)
            {

            }
            else if (selectedID == 3)
            {

            }
            else if (selectedID == 4)
            {

            }

        };
        addAndMakeVisible(changeMultipleButton.get());

        actionCB = std::make_unique<juce::ComboBox>();

        actionCB->addItem("Everything to default",1);
        actionCB->addItem("Notes to default", 2);
        actionCB->addItem("Timestamps to default", 3);
        actionCB->addItem("Velocities to default", 4);

        actionCB->setSelectedId(1);

        addAndMakeVisible(actionCB.get());


        disclaimerLabel = std::make_unique<juce::Label>();
        disclaimerLabel->setText("Note: This view shows the data used in the app only.\nYour original files remain unchanged.", juce::dontSendNotification);
        disclaimerLabel->setJustificationType(juce::Justification::centred);
        disclaimerLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        disclaimerLabel->setFont(juce::Font(12.0f));
        addAndMakeVisible(disclaimerLabel.get());
        table->setBounds(getLocalBounds());
        table->updateContent();
    }

    void resized() override;

    ~TableContainer() override
    {
        table->setModel(nullptr);
    }

private:
    /*for future reference, making this whole thing in a separate thread could benefit if mid playing(if it's needed, but prob not since the playing takes place on another thread,
     with the timer so probably it's no issue)
     */
    std::unique_ptr<juce::TableListBox> table;
    std::unique_ptr<MidiNotesTableModel> model;
    std::unique_ptr<juce::Label> disclaimerLabel;
    std::unique_ptr<juce::TextButton> changeMultipleButton = nullptr;
    std::unique_ptr<juce::ComboBox> actionCB = nullptr;

};