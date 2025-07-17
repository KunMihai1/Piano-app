/*
  ==============================================================================

    CustomTableContainer.cpp
    Created: 17 Jul 2025 10:33:04pm
    Author:  Kisuke

  ==============================================================================
*/

#include "CustomTableContainer.h"

TableContainer::TableContainer(juce::MidiMessageSequence& seq, const juce::String& displayName, int channel, std::unordered_map<int, MidiChangeInfo>& map): changesMap{map}, originalSequence{seq}
{
    juce::String displayN = "Notes - " + displayName;
    model = std::make_unique<MidiNotesTableModel>(seq, channel, map);
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
            allProperties();
        }
        else if (selectedID == 2)
        {
            onlyNotes();
        }
        else if (selectedID == 3)
        {
            onlyTimeStamps();
        }
        else if (selectedID == 4)
        {
            onlyVelocities();
        }
        table->setModel(nullptr);          
        model->refreshVectorFromSequence(originalSequence);
        table->setModel(model.get());      
        table->updateContent();
        table->repaint();
        if (updateToFile)
            updateToFile();

    };
    addAndMakeVisible(changeMultipleButton.get());

    actionCB = std::make_unique<juce::ComboBox>();

    actionCB->addItem("Everything to default", 1);
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

void TableContainer::resized()
{
    auto area = getLocalBounds();
    auto labelHeight = 40;
    auto buttonsControlBarHeight = 30;
    const int spacing = 10;
    const int verticalPadding = 5;

    table->setBounds(area.removeFromTop(area.getHeight() - (labelHeight + buttonsControlBarHeight)));

    auto disclaimerArea = area.removeFromBottom(labelHeight);
    auto controlBarArea = area.removeFromBottom(buttonsControlBarHeight);

    auto innerBar = controlBarArea.reduced(10, verticalPadding);
    int comboWidth = 200;
    int buttonWidth = 100;

    actionCB->setBounds(innerBar.removeFromLeft(comboWidth));
    innerBar.removeFromLeft(spacing);
    changeMultipleButton->setBounds(innerBar.removeFromLeft(buttonWidth));

    disclaimerLabel->setBounds(disclaimerArea);
}

TableContainer::~TableContainer()
{
    table->setModel(nullptr);
}

void TableContainer::onlyNotes()
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        info.newNumber = info.oldNumber;

        if (validForErase(info))
        {
            applyChangeToSequence(key, info);
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
}

void TableContainer::onlyTimeStamps()
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        info.newTimeStamp = info.oldTimeStamp;

        if (validForErase(info))
        {
            applyChangeToSequence(key,info);
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
}

void TableContainer::onlyVelocities()
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        info.newVelocity = info.oldVelocity;

        if (validForErase(info))
        {
            applyChangeToSequence(key, info);
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
}

void TableContainer::allProperties()
{
    for (auto& pair : changesMap)
    {
        auto& key = pair.first;
        auto& info = pair.second;
        applyChangeToSequence(key, info);
    }
    changesMap.clear();
}

bool TableContainer::validForErase(MidiChangeInfo& info)
{
    if (info.newNumber == info.oldNumber && info.newTimeStamp == info.oldTimeStamp && info.newVelocity == info.oldVelocity)
        return true;
    return false;
}

void TableContainer::applyChangeToSequence(int index, const MidiChangeInfo& info)
{
    auto* e = originalSequence.getEventPointer(index);
    if (e == nullptr)
        return;

    if (e->message.isNoteOn())
    {
        juce::MidiMessage newMsg = juce::MidiMessage::noteOn(
            e->message.getChannel(),
            info.oldNumber,
            (juce::uint8)info.oldVelocity
        );
        newMsg.setTimeStamp(info.oldTimeStamp);
        e->message = newMsg;
    }
    else if (e->message.isNoteOff())
    {
        juce::MidiMessage newMsg = juce::MidiMessage::noteOff(
            e->message.getChannel(),
            info.oldNumber
        );
        newMsg.setTimeStamp(info.oldTimeStamp);
        e->message = newMsg;
    }
}
