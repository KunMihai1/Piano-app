/*
  ==============================================================================

    CustomTableContainer.cpp
    Created: 17 Jul 2025 10:33:04pm
    Author:  Kisuke

  ==============================================================================
*/

#include "CustomTableContainer.h"



TableContainer::TableContainer(juce::MidiMessageSequence& seq, const juce::String& displayName, int channel, std::unordered_map<int, MidiChangeInfo>& map, bool shouldDisable): changesMap{map}, originalSequence{seq}
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

    model->refreshData = [this]()
    {
        table->updateContent();
    };

    model->onRequestSelectRow = [this](int row)
    {
        if (row >= 0 && row < model->getNumRows())
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
        }
    };

    model->onMidPlayRepaint = [this](int row)
    {
        if (row >= 0 && row < model->getNumRows())
        {
            table->deselectAllRows();

            table->selectRow(row);

            table->scrollToEnsureRowIsOnscreen(row);

            table->repaint();
        }
        else table->deselectAllRows();
    };

    model->getCurrentBpm = [this]()
    {
        return getCurrentBPMstyle();
    };

    model->validForErase = [this](MidiChangeInfo& info)
    {
        return validForErase(info);
    };

    table->setModel(model.get());

    table->setMultipleSelectionEnabled(true);

    table->getHeader().addColumn("Note", 1, 100);
    table->getHeader().addColumn("Time", 2, 120);
    table->getHeader().addColumn("Velocity", 3, 100);
    table->getHeader().addColumn("Channel", 4, 80);

    addAndMakeVisible(table.get());

    changeToOriginalMultipleButton = std::make_unique<juce::TextButton>("Change");
    changeToOriginalMultipleButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    if (shouldDisable)
        changeToOriginalMultipleButton->setEnabled(false);

    changeToOriginalMultipleButton->onClick = [this]()
    {
        int selectedID = actionToOriginalCB->getSelectedId();

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
    addAndMakeVisible(changeToOriginalMultipleButton.get());

    actionToOriginalCB = std::make_unique<juce::ComboBox>();

    actionToOriginalCB->addItem("Everything to default", 1);
    actionToOriginalCB->addItem("Notes to default", 2);
    actionToOriginalCB->addItem("Timestamps to default", 3);
    actionToOriginalCB->addItem("Velocities to default", 4);

    actionToOriginalCB->setSelectedId(1);

    addAndMakeVisible(actionToOriginalCB.get());

    infoLabelChange = std::make_unique<juce::Label>();
    infoLabelChange->setText("Undo settings", juce::dontSendNotification);
    infoLabelChange->setJustificationType(juce::Justification::centred);

    addAndMakeVisible(infoLabelChange.get());

    modifyMultipleButton = std::make_unique<juce::TextButton>("Apply");
    modifyMultipleButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    
    if (shouldDisable)
        modifyMultipleButton->setEnabled(false);

    modifyMultipleButton->onClick = [this]()
    {
        int selectedID = actionModifyCB->getSelectedId();
        if (selectedID == 1)
        {
            modifyTimeStampsUI();
        }
        else if (selectedID == 2)
        {
            modifyVelocitiesUI();
        }
    };

    addAndMakeVisible(modifyMultipleButton.get());

    actionModifyCB = std::make_unique<juce::ComboBox>();

    actionModifyCB->addItem("Modify time stamps",1);
    actionModifyCB->addItem("Modify velocities", 2);
    
    actionModifyCB->setSelectedId(1);

    addAndMakeVisible(actionModifyCB.get());

    infoLabelModify = std::make_unique<juce::Label>();
    infoLabelModify->setText("Modify settings", juce::dontSendNotification);
    infoLabelModify->setJustificationType(juce::Justification::centred);

    addAndMakeVisible(infoLabelModify.get());

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
    const int infoLabelHeight = 20;

    table->setBounds(area.removeFromTop(area.getHeight() - (labelHeight + buttonsControlBarHeight*2)));

    auto disclaimerArea = area.removeFromBottom(labelHeight);
    auto controlBarChangeArea = area.removeFromBottom(buttonsControlBarHeight);
    auto controlBarModifyArea = area.removeFromBottom(buttonsControlBarHeight);

    auto innerBarChange = controlBarChangeArea.reduced(10, verticalPadding);
    int comboWidth = 150;
    int buttonWidth = 50;

    actionToOriginalCB->setBounds(innerBarChange.removeFromLeft(comboWidth));
    innerBarChange.removeFromLeft(spacing);
    changeToOriginalMultipleButton->setBounds(innerBarChange.removeFromLeft(buttonWidth));

    infoLabelChange->setBounds(innerBarChange.withTrimmedTop((innerBarChange.getHeight() - infoLabelHeight) / 2)
        .withHeight(infoLabelHeight));

    auto innerBarModify = controlBarModifyArea.reduced(10, verticalPadding);

    actionModifyCB->setBounds(innerBarModify.removeFromLeft(comboWidth));
    innerBarModify.removeFromLeft(spacing);
    modifyMultipleButton->setBounds(innerBarModify.removeFromLeft(buttonWidth));

    infoLabelModify->setBounds(innerBarModify.withTrimmedTop((innerBarModify.getHeight() - infoLabelHeight) / 2)
        .withHeight(infoLabelHeight));

    disclaimerLabel->setBounds(disclaimerArea);
}

void TableContainer::updateObjects()
{
    if (changeToOriginalMultipleButton)
    {
        changeToOriginalMultipleButton->setEnabled(!changeToOriginalMultipleButton->isEnabled());
    }

    if (modifyMultipleButton)
    {
        modifyMultipleButton->setEnabled(!modifyMultipleButton->isEnabled());
    }   
}

TableContainer::~TableContainer()
{
    table->setModel(nullptr);

    if (model && removeModelFromListener)
        removeModelFromListener(model.get());

    if (removeContainerFromListeners)
        removeContainerFromListeners();
}

void TableContainer::onlyNotes()
{
    resetPropertyAndApply([](MidiChangeInfo& info)
    {
            info.newNumber = info.oldNumber;
    });
}

void TableContainer::onlyNotesSelected(const juce::SparseSet<int>& allSelected)
{
    resetPropertyAndApply([](MidiChangeInfo& info)
        {
            info.newNumber = info.oldNumber;

        } ,
        &allSelected);
}

void TableContainer::onlyTimeStamps()
{
    resetPropertyAndApply([](MidiChangeInfo& info)
        {
            info.newTimeStamp = info.oldTimeStamp;
        },nullptr,false);
}

void TableContainer::onlyTimeStampsSelected(const juce::SparseSet<int>& allSelected)
{
    resetPropertyAndApply([](MidiChangeInfo& info)
        {
            info.newTimeStamp = info.oldTimeStamp;
        },
        &allSelected,false);
}

void TableContainer::onlyVelocities()
{
    resetPropertyAndApply([](MidiChangeInfo& info)
        {
            info.newVelocity = info.oldVelocity;

        },nullptr,true);
    
}

void TableContainer::onlyVelocitiesSelected(const juce::SparseSet<int>& allSelected)
{
    resetPropertyAndApply([](MidiChangeInfo& info)
        {
            info.newVelocity = info.oldVelocity;

        },
        &allSelected,true);
}

void TableContainer::allProperties()
{
    for (auto& pair : changesMap)
    {
        auto& key = pair.first;
        
        int originalIndex = model->getOriginalIndexFromRow(key);

        auto& info = pair.second;
        info.newNumber = info.oldNumber;
        info.newTimeStamp = info.oldTimeStamp;
        info.newVelocity = info.oldVelocity;

        applyChangeFromSequence(originalIndex, info,false);
    }
    changesMap.clear();
}

void TableContainer::allPropertiesSelected(const juce::SparseSet<int>& allSelected)
{
    resetPropertyAndApply([](MidiChangeInfo& info)
        {
            info.newNumber = info.oldNumber;
            info.newTimeStamp = info.oldTimeStamp;
            info.newVelocity = info.oldVelocity;
        }, 
        &allSelected);
}

bool TableContainer::validForErase(MidiChangeInfo& info)
{
    if (info.newNumber == info.oldNumber && info.newTimeStamp == info.oldTimeStamp && info.newVelocity == info.oldVelocity)
        return true;
    return false;
}

void TableContainer::applyChangeFromSequence(int originalIndex, const MidiChangeInfo& info, bool modifyVelocity, bool shouldApplyChanges)
{
    auto* e = originalSequence.getEventPointer(originalIndex);
    if (e == nullptr)
        return;

    double ratio;
    double fromChange=0;
    if (shouldApplyChanges) //this is not a good condition
    {
        fromChange = info.oldTimeStamp - info.newTimeStamp;
        ratio = info.newBPMchange / getCurrentBPMstyle();
    }
    else ratio = info.oldBPMchange / getCurrentBPMstyle();
    


    if (e->message.isNoteOn())
    {
        double duration=0.0;
        if (!modifyVelocity && e->noteOffObject != nullptr)
        {
            auto* noteoffEvent = e->noteOffObject;
            duration = noteoffEvent->message.getTimeStamp() - e->message.getTimeStamp();
        }

        juce::MidiMessage newMsg = juce::MidiMessage::noteOn(
            e->message.getChannel(),
            info.newNumber,
            (juce::uint8)juce::jlimit(0, 127, info.newVelocity)
        );
        bool shouldClamp = false;

        double timeToApplyOn = 0.0;

        if (shouldApplyChanges)
        {
            timeToApplyOn = info.oldTimeStamp * ratio - fromChange;
        }
        else timeToApplyOn = info.newTimeStamp * ratio;

        /*
        if (timeToApplyOn < 0)
        {
            timeToApplyOn = 0.0;
            shouldClamp = true;
        }
        */

        newMsg.setTimeStamp(timeToApplyOn);

        e->message = newMsg;

        if (!modifyVelocity && e->noteOffObject != nullptr)
        {
            auto* noteOffEvent = e->noteOffObject;
            juce::MidiMessage offMsg = juce::MidiMessage::noteOff(
                noteOffEvent->message.getChannel(),
                info.newNumber
            );
            

            double offTime = info.newTimeStamp + duration;
                
            double timetoApplyOff=0.0;
            if (shouldApplyChanges)
            {
                offTime = info.oldTimeStamp + duration;
                timetoApplyOff = offTime * ratio - fromChange;
            }
            else timetoApplyOff = offTime * ratio; 

            //if (shouldClamp)
            //    timetoApplyOff = duration; 

            offMsg.setTimeStamp(timetoApplyOff);

            noteOffEvent->message = offMsg;
        }
    }
}

void TableContainer::onConfirmed()
{
    int scrollPos = table->getViewport()->getViewPositionY();

    juce::SparseSet<int> selectedRows = table->getSelectedRows();

    table->setModel(nullptr);
    model->refreshVectorFromSequence(originalSequence);
    table->setModel(model.get());
    table->updateContent();

    table->setSelectedRows(selectedRows);

    table->getViewport()->setViewPosition(table->getViewport()->getViewPositionX(), scrollPos);

    table->repaint();

    if (updateToFile)
        updateToFile();
}

void TableContainer::changeAllUI()
{
    showModifyChangeDialog("Confirm Action",
        "What would you like to do with the track properties?",
        "What would you like to do with the track properties?",
        { "Change All", "Change selected", "Cancel" },
        { "Change All", "Cancel" },
        [this](int result)
        {
            if (result == 1)
                allProperties();
            else if (result == 2)
                allPropertiesSelected(table->getSelectedRows());
        });
}

void TableContainer::changeNotesUI()
{
    showModifyChangeDialog("Confirm Action",
        "What would you like to do with the notes?",
        "What would you like to do with the notes?",
        { "Change All", "Change selected", "Cancel" },
        { "Change All", "Cancel" },
        [this](int result)
        {
            if (result == 1)
                onlyNotes();
            else if (result == 2)
                onlyNotesSelected(table->getSelectedRows());
        });
}

void TableContainer::changeTimeStampsUI()
{
    showModifyChangeDialog("Confirm Action",
        "What would you like to do with the time stamps?",
        "What would you like to do with the time stamps?",
        { "Change All", "Change selected", "Cancel" },
        { "Change All", "Cancel" },
        [this](int result)
        {
            if (result == 1)
                onlyTimeStamps();
            else if (result == 2)
                onlyTimeStampsSelected(table->getSelectedRows());
        });
}

void TableContainer::changeVelocitiesUI()
{
    showModifyChangeDialog("Confirm Action",
        "What would you like to do with the velocities?",
        "What would you like to do with the velocities?",
        { "Change All", "Change selected", "Cancel" },
        { "Change All", "Cancel" },
        [this](int result)
        {
            if (result == 1)
                onlyVelocities();
            else if (result == 2)
                onlyVelocitiesSelected(table->getSelectedRows());
        });
}

bool TableContainer::anySelected()
{
    if (table->getSelectedRows().size())
        return true;

    return false;
}

void TableContainer::modifyTimeStampsUI()
{
    showModifyChangeDialog("Confirm action",
        "What would you like to do with the time stamps?",
        "What would you like to do with the time stamps?",
        { "Modify All", "Modify selected", "Cancel" },
        { "Modify All", "Cancel" },
        [this](int result)
        {
            if (result == 1)
                modifyOnlyTimeStamps();
            else if (result == 2)
                modifyOnlyTimeStampsSelected(table->getSelectedRows());
        },true);
}

void TableContainer::modifyVelocitiesUI()
{
    showModifyChangeDialog("Confirm action",
        "What would you like to do with the velocities?",
        "What would you like to do with the velocities?",
        { "Modify All", "Modify selected", "Cancel" },
        { "Modify All", "Cancel" },
        [this](int result)
        {
            if (result == 1)
            {
                modifyOnlyVelocities();
            }
            else if (result == 2)
            {
                modifyOnlyVelocitiesSelected(table->getSelectedRows());
            }
        },true);
}

void TableContainer::modifyOnlyTimeStamps()
{
    newPropertyAndApply([this](MidiChangeInfo& info)
        {
            double old = info.newTimeStamp;
            if (std::holds_alternative<double>(editorValue))
                info.newTimeStamp = old+std::get<double>(editorValue);
            else if (std::holds_alternative<int>(editorValue))
                info.newTimeStamp = old+ static_cast<double>(std::get<int>(editorValue));
        });
}

void TableContainer::modifyOnlyTimeStampsSelected(const juce::SparseSet<int>& allSelected)
{
    newPropertyAndApply([this](MidiChangeInfo& info)
        {
            double old= info.newTimeStamp;
            if (std::holds_alternative<double>(editorValue))
                info.newTimeStamp = old + std::get<double>(editorValue);
            else if (std::holds_alternative<int>(editorValue))
                info.newTimeStamp = old + static_cast<double>(std::get<int>(editorValue));

        },&allSelected);
}

void TableContainer::modifyOnlyVelocities()
{
    newPropertyAndApply([this](MidiChangeInfo& info)
        {
            if (std::holds_alternative<int>(editorValue))
                info.newVelocity = std::get<int>(editorValue);
            else if (std::holds_alternative<double>(editorValue))
                info.newVelocity = static_cast<int>(std::get<double>(editorValue));

        },nullptr,true);
}

void TableContainer::modifyOnlyVelocitiesSelected(const juce::SparseSet<int>& allSelected)
{
    newPropertyAndApply([this](MidiChangeInfo& info)
        {
            if (std::holds_alternative<int>(editorValue))
                info.newVelocity = std::get<int>(editorValue);
            else if (std::holds_alternative<double>(editorValue))
                info.newVelocity = static_cast<int>(std::get<double>(editorValue));

        },&allSelected,true);
}


void TableContainer::resetPropertyAndApply(const std::function<void(MidiChangeInfo&)>& resetProperty, const juce::SparseSet<int>* selected, bool modifyVelocity)
{
    if (selected!=nullptr)
    {
        for (int rangeIndex = 0; rangeIndex < selected->getNumRanges(); ++rangeIndex)
        {
            auto range = selected->getRange(rangeIndex);
            for (int row = range.getStart(); row < range.getEnd(); ++row)
            {
                int originalIndex = model->getOriginalIndexFromRow(row);
                int noteOnIndex = model->getChangesMapIndexFromRow(row);

                auto it = changesMap.find(noteOnIndex);

                if (it != changesMap.end())
                {
                    auto& info = it->second;
                    resetProperty(info);
                    applyChangeFromSequence(originalIndex, info, modifyVelocity);

                    if (validForErase(info))
                        changesMap.erase(it);
                }
            }
        }
    }
    else
    {
        for (auto it = changesMap.begin(); it != changesMap.end(); )
        {
            auto& key = it->first;
            auto& info = it->second;

            int originalIndex = model->getOriginalIndexFromRow(key);

            resetProperty(info);
            applyChangeFromSequence(originalIndex, info, modifyVelocity);

            if (validForErase(info))
                it = changesMap.erase(it);
            else
                ++it;
        }
    }
}

void TableContainer::newPropertyAndApply(const std::function<void(MidiChangeInfo&)>& newProperty, const juce::SparseSet<int>* selected, bool modifyVelocity)
{
    if (selected != nullptr)
    {
        for (int rangeIndex = 0; rangeIndex < selected->getNumRanges(); ++rangeIndex)
        {
            auto range = selected->getRange(rangeIndex);
            for (int row = range.getStart(); row < range.getEnd(); ++row)
            {
                int originalIndex = model->getOriginalIndexFromRow(row);
                int noteOnIndex = model->getChangesMapIndexFromRow(row);
                newPropertyHelperFunction(newProperty, selected, modifyVelocity,originalIndex, noteOnIndex);
            }
        }
    }
    else
    {
        for (auto& entry : model->getEvents())
        {
            auto& originalIndex = entry.originalIndex;
            auto& noteOnIndex = entry.indexForChangesMap;
            newPropertyHelperFunction(newProperty, selected, modifyVelocity,originalIndex, noteOnIndex);
        }
    }

}

void TableContainer::newPropertyHelperFunction(const std::function<void(MidiChangeInfo&)>& newProperty, const juce::SparseSet<int>* selected, bool modifyVelocity, int originalIndex, int noteOnIndex)
{
    auto* e = originalSequence.getEventPointer(originalIndex);
    if (e == nullptr)
        return;

    auto oldMsg = e->message;

    auto [it, inserted] = changesMap.try_emplace(noteOnIndex);
    MidiChangeInfo& info = it->second;

    if (inserted)
    {
        
        info.oldNumber = oldMsg.getNoteNumber();
        info.oldTimeStamp = oldMsg.getTimeStamp();
        info.oldVelocity = oldMsg.getVelocity();
        info.oldBPMchange = getCurrentBPMstyle();

        info.newBPMchange = getCurrentBPMstyle();
        info.newNumber = oldMsg.getNoteNumber();
        info.newTimeStamp = oldMsg.getTimeStamp();
        info.newVelocity = oldMsg.getVelocity();
    }

    newProperty(info);

    if (modifyVelocity)
        applyChangeFromSequence(originalIndex, info, true,true);
    else
        applyChangeFromSequence(originalIndex, info,false,true);

    info.newBPMchange = getCurrentBPMstyle();

    if (validForErase(info))
        changesMap.erase(noteOnIndex);
}

void TableContainer::showModifyChangeDialog(
    const juce::String& title,
    const juce::String& messageAllSelected,
    const juce::String& messageNoSelection,
    const std::vector<juce::String>& buttonsWithSelection,
    const std::vector<juce::String>& buttonsWithoutSelection,
    std::function<void(int)> onValidResults,
    bool isModify)
{

    if (isModify)
    {
       juce::String title = "Modify", text, label;
       int selectedID = actionModifyCB->getSelectedId();

       if (selectedID == 1)
       {
           text = "Shift the time stamps of your notes";
           title += " time stamps";
           label = "Time stamp";
       }
       else if (selectedID == 2)
       {
           text = "Enter a new velocity for your notes";
           title += " velocities";
           label = "Velocity";
       }
       label += " value:";

       auto* window = new juce::AlertWindow{ title, text, juce::AlertWindow::NoIcon };

       window->addTextEditor("propertyEditor", "", label);

       window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
       window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

       window->enterModalState(true,
           juce::ModalCallbackFunction::create([this, window, selectedID, title,messageAllSelected,messageNoSelection,buttonsWithSelection,buttonsWithoutSelection,onValidResults](int result)
               {
                   std::unique_ptr<juce::AlertWindow> cleanup{ window };
                   if (result != 1)
                       return;

                   juce::String userValue = window->getTextEditor("propertyEditor")->getText().trim();
                   if (selectedID == 1)
                   {
                       if (!Validator::isValidMidiDoubleString(userValue))
                       {
                           juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Modify time stamps", "Invalid input value");
                           return;
                       }

                       double doubleValue = userValue.getDoubleValue();

                       int currentRow = table->getSelectedRows()[0];
                       DBG("current row is: " + juce::String(currentRow));

                       double timeStampPreviousNoteOn=model->getPreviousNoteOnTimeStamp(currentRow);
                       double timeStampCurrentNoteOn = model->getCurrentNoteOnTimeStamp(currentRow);
                       double timeStampNextNoteOn = model->getNextNoteOnTimeStamp(currentRow);

                       DBG("prev: " + juce::String(timeStampPreviousNoteOn) + " current: " + juce::String(timeStampCurrentNoteOn) + " next: " + juce::String(timeStampNextNoteOn));

                       if (timeStampPreviousNoteOn == -1)
                           return;

                       if (!Validator::isValidMidiDoubleValueTimeStamps(doubleValue,timeStampCurrentNoteOn,timeStampPreviousNoteOn,timeStampNextNoteOn))
                       {
                           juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Modify time stamps", "Invalid value");
                           return;
                       }
                       editorValue = doubleValue;
                   }
                   else if (selectedID == 2)
                   {
                       if (!Validator::isValidMidiIntegerString(userValue))
                       {
                           juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Modify velocities", "Invalid input value");
                           return;
                       }

                       int intValue = userValue.getIntValue();

                       if (!Validator::isValidMidiIntValue(intValue))
                       {
                           juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Modify velocities", "Invalid value");
                           return;
                       }

                       editorValue = intValue;
                   }

                   auto* alert = new juce::AlertWindow(title, anySelected() ? messageAllSelected : messageNoSelection, juce::AlertWindow::QuestionIcon);

                   auto& buttons = anySelected() ? buttonsWithSelection : buttonsWithoutSelection;

                   for (int i = 0; i < buttons.size(); i++)
                       alert->addButton(buttons[i], i + 1);

                   alert->enterModalState(true,
                       juce::ModalCallbackFunction::create([this, onValidResults](int result)
                           {
                               if (result == 0) return;
                               onValidResults(result);
                               onConfirmed();
                           }),
                       true);
                   
               }));
       juce::MessageManager::callAsync([window]()
           {
               if (auto* editor = window->getTextEditor("propertyEditor"))
                   editor->grabKeyboardFocus();
           });
    }
    else
    {
        auto* alert = new juce::AlertWindow(title, anySelected() ? messageAllSelected : messageNoSelection, juce::AlertWindow::QuestionIcon);

        auto& buttons = anySelected() ? buttonsWithSelection : buttonsWithoutSelection;

        for (int i = 0; i < buttons.size(); i++)
            alert->addButton(buttons[i], i + 1);

        alert->enterModalState(true,
            juce::ModalCallbackFunction::create([this, onValidResults](int result)
                {
                    if (result == 0) return;
                    onValidResults(result);
                    onConfirmed();
                }),
            true);
    }
}

void TableContainer::addModelAsListenerToTrackPlayer(MultipleTrackPlayer* player)
{
    if (player)
        player->addSubjectTrackPlayerListener(model.get());
}