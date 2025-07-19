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

    model->onRequestSelectRow = [this](int row)
    {
        auto mods = juce::ModifierKeys::getCurrentModifiers();

        if (mods.isShiftDown())
        {
            int anchor = (lastSelectedRow >= 0) ? lastSelectedRow : row;
            int start = std::min(anchor, row);
            int end = std::max(anchor, row);

            // Copy current selection to an array
            juce::Array<int> selectionArray;
            auto selectionSet = table->getSelectedRows();

            for (int i = 0; i < selectionSet.size(); ++i)
                selectionArray.add(selectionSet[i]);

            // Add the new range to the array if not already selected
            for (int i = start; i <= end; ++i)
                if (!selectionArray.contains(i))
                    selectionArray.add(i);

            // Create new SparseSet for selection
            juce::SparseSet<int> newSelection;
            for (auto v : selectionArray)
                newSelection.addRange({ v, v + 1 });  // add single row as range [v, v+1)

            // Set the updated selection
            table->setSelectedRows(newSelection);

            lastSelectedRow = row;
        }
        else if (mods.isCtrlDown())
        {
            juce::Array<int> selectionArray;
            auto selectionSet = table->getSelectedRows();
            
            for (int i = 0; i < selectionSet.size(); i++)
            {
                selectionArray.add(selectionSet[i]);
            }

            bool isSelected;

            if (selectionSet.contains(row))
                isSelected = true;
            else isSelected = false;

            if (isSelected)
            {
                selectionArray.removeFirstMatchingValue(row);
            }
            else {
                selectionArray.add(row);
            }

            juce::SparseSet<int> newSelection;
            for (auto v : selectionArray)
                newSelection.addRange({ v, v + 1 });

            table->setSelectedRows(newSelection);

            lastSelectedRow = row;
        }
        else
        {
            // Normal single selection, deselect others
            table->selectRow(row, false);
            lastSelectedRow = row;
        }
    };

    table->setModel(model.get());

    table->setMultipleSelectionEnabled(true);

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
            changeAllUI();
        }
        else if (selectedID == 2)
        {
            changeNotesUI();
        }
        else if (selectedID == 3)
        {
            changeTimeStampsUI();
        }
        else if (selectedID == 4)
        {
            changeVelocitiesUI();
        }

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
        applyChangeToSequence(key, info);

        if (validForErase(info))
        {
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
}

void TableContainer::onlyNotesSelected(const juce::SparseSet<int>& allSelected)
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        if (allSelected.contains(key))
        {
            info.newNumber = info.oldNumber;
            applyChangeToSequence(key, info);
        }
        else return;


        if (validForErase(info))
        {
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
        applyChangeToSequence(key, info);

        if (validForErase(info))
        {
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
}

void TableContainer::onlyTimeStampsSelected(const juce::SparseSet<int>& allSelected)
{
}

void TableContainer::onlyVelocities()
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        info.newVelocity = info.oldVelocity;
        applyChangeToSequence(key, info);

        if (validForErase(info))
        {
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
}

void TableContainer::onlyVelocitiesSelected(const juce::SparseSet<int>& allSelected)
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        info.newVelocity = info.oldVelocity;

        if (allSelected.contains(key))
        {
            info.newVelocity = info.oldVelocity;
            applyChangeToSequence(key, info);
        }
        else return;

        if (validForErase(info))
        {
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
        info.newNumber = info.oldNumber;
        info.newTimeStamp = info.oldTimeStamp;
        info.newVelocity = info.oldVelocity;

        applyChangeToSequence(key, info);
    }
    changesMap.clear();
}

void TableContainer::allPropertiesSelected(const juce::SparseSet<int>& allSelected)
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;
        
        if (allSelected.contains(key))
        {
            info.newNumber = info.oldNumber;
            info.newTimeStamp = info.oldTimeStamp;
            info.newVelocity = info.oldVelocity;

            applyChangeToSequence(key, info);
        }
        else return;

        if (validForErase(info))
        {
            it = changesMap.erase(it);
        }
        else
            ++it;
    }
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
            info.newNumber,
            (juce::uint8)info.newVelocity
        );
        newMsg.setTimeStamp(info.newTimeStamp);
        e->message = newMsg;
    }
    else if (e->message.isNoteOff())
    {
        juce::MidiMessage newMsg = juce::MidiMessage::noteOff(
            e->message.getChannel(),
            info.newNumber
        );
        newMsg.setTimeStamp(info.newTimeStamp);
        e->message = newMsg;
    }
}

void TableContainer::onConfirmed()
{
    table->setModel(nullptr);
    model->refreshVectorFromSequence(originalSequence);
    table->setModel(model.get());
    table->updateContent();
    table->repaint();
    if (updateToFile)
        updateToFile();
}

void TableContainer::changeAllUI()
{
    if (anySelected()==true)
    {
        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the track properties?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Reset All", 1);
        alert->addButton("Reset Selected", 2);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 0)
                        return;
                    if (result == 1)
                    {
                        allProperties();
                    }
                    else if (result == 2)
                    {
                        allPropertiesSelected(table->getSelectedRows());
                    }
                    onConfirmed();
                    
                }),
            true);
    }
    else
    {

        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with all the track properties?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 1)
                    {
                        allProperties();
                        onConfirmed();
                    }

                }),
            true);
    }
}

void TableContainer::changeNotesUI()
{
    if (anySelected() == true)
    {
        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the notes?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Change selected", 2);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 0)
                        return;
                    if (result == 1)
                    {
                        onlyNotes();
                    }
                    else if (result == 2)
                    {
                        onlyNotesSelected(table->getSelectedRows());
                    }
                    onConfirmed();

                }),
            true);
    }
    else {

        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the notes?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 1)
                    {
                        onlyNotes();
                        onConfirmed();
                    }

                }),
            true);
    }
}

void TableContainer::changeTimeStampsUI()
{
    if (anySelected() == true)
    {
        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the time stamps?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Change selected", 2);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 0)
                        return;
                    if (result == 1)
                    {
                        onlyTimeStamps();
                    }
                    else if (result == 2)
                    {
                        onlyTimeStampsSelected(table->getSelectedRows());
                    }
                    onConfirmed();

                }),
            true);
    }
    else {

        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the time stamps?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 1)
                    {
                        onlyTimeStamps();
                        onConfirmed();
                    }

                }),
            true);
    }
}

void TableContainer::changeVelocitiesUI()
{
    if (anySelected() == true)
    {
        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the velocities?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Change selected", 2);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 0)
                        return;
                    if (result == 1)
                    {
                        onlyVelocities();
                    }
                    else if (result == 2)
                    {
                        onlyVelocitiesSelected(table->getSelectedRows());
                    }
                    onConfirmed();

                }),
            true);
    }
    else {

        auto* alert = new juce::AlertWindow("Confirm Action",
            "What would you like to do with the velocities?",
            juce::AlertWindow::QuestionIcon);

        alert->addButton("Change All", 1);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int result)
                {
                    if (result == 1)
                    {
                        onlyVelocities();
                        onConfirmed();
                    }

                }),
            true);
    }
}

bool TableContainer::anySelected()
{
    if (table->getSelectedRows().size())
        return true;

    return false;
}

void TableContainer::resetPropertyAndApply(std::function<void(MidiChangeInfo&)> resetProperty, const juce::SparseSet<int>* selected)
{
    for (auto it = changesMap.begin(); it != changesMap.end(); )
    {
        auto& key = it->first;
        auto& info = it->second;

        if (selected && !selected->contains(key))
        {
            ++it;
            continue;
        }

        resetProperty(info);
        applyChangeToSequence(key, info);

        if (validForErase(info))
            it = changesMap.erase(it);
        else
            ++it;
    }
}
