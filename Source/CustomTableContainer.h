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
#include "ValidatorUI.h"
#include <variant>
#include "SubjectInterface.h"

class TableContainer: public juce::Component
{
public:
    std::function<void()> updateToFile;
    std::function<void(TrackPlayerListener* listener)> removeModelFromListener;

    TableContainer(juce::MidiMessageSequence& seq, const juce::String& displayName, int channel, std::unordered_map<int, MidiChangeInfo>& map);

    void resized() override;

    ~TableContainer() override;

    void onlyNotes();

    void onlyNotesSelected(const juce::SparseSet<int>& allSelected);

    void onlyTimeStamps();

    void onlyTimeStampsSelected(const juce::SparseSet<int>& allSelected);
    
    void onlyVelocities();

    void onlyVelocitiesSelected(const juce::SparseSet<int>& allSelected);

    void allProperties();

    void allPropertiesSelected(const juce::SparseSet<int>& allSelected);

    bool validForErase(MidiChangeInfo& info);

    void applyChangeToSequence(int index, const MidiChangeInfo& info, bool modifyVelocity=false);

    void onConfirmed();

    void changeAllUI();

    void changeNotesUI();

    void changeTimeStampsUI();

    void changeVelocitiesUI();

    bool anySelected();

    void modifyTimeStampsUI();

    void modifyVelocitiesUI();

    void modifyOnlyTimeStamps();

    void modifyOnlyTimeStampsSelected(const juce::SparseSet<int>& allSelected);

    void modifyOnlyVelocities();

    void modifyOnlyVelocitiesSelected(const juce::SparseSet<int>& allSelected);

    void resetPropertyAndApply(const std::function<void(MidiChangeInfo&)>& resetProperty, const juce::SparseSet<int>* selected = nullptr, bool modifyVelocity=false);

    void newPropertyAndApply(const std::function<void(MidiChangeInfo&)>& newProperty, const juce::SparseSet<int>* selected = nullptr, bool modifyVelocity=false);

    void newPropertyHelperFunction(const std::function<void(MidiChangeInfo&)>& newProperty, const juce::SparseSet<int>* selected, bool modifyVelocity, int originalIndex);

    void showModifyChangeDialog(const juce::String& title, const juce::String& messageAllSelected, const juce::String& messageNoSelection,
        const std::vector<juce::String>& buttonsWithSelection, const std::vector<juce::String>& buttonsWithoutSelection,
        std::function<void(int)> onValidResults, bool isModify=false);

    void addModelAsListener(Subject<TrackPlayerListener>* subject=nullptr);

private:
    /*for future reference, making this whole thing in a separate thread could benefit if mid playing(if it's needed, but prob not since the playing takes place on another thread,
     with the timer so probably it's no issue)
     */
    std::unique_ptr<juce::TableListBox> table;
    std::unique_ptr<MidiNotesTableModel> model;
    std::unique_ptr<juce::Label> disclaimerLabel;
    std::unique_ptr<juce::TextButton> changeToOriginalMultipleButton = nullptr;
    std::unique_ptr<juce::ComboBox> actionToOriginalCB = nullptr;
    std::unique_ptr<juce::Label> infoLabelChange = nullptr;

    std::unique_ptr<juce::TextButton> modifyMultipleButton = nullptr;
    std::unique_ptr<juce::ComboBox> actionModifyCB = nullptr;
    std::unique_ptr<juce::Label> infoLabelModify = nullptr;

    juce::MidiMessageSequence& originalSequence;
    int lastSelectedRow = -1;
    std::unordered_map<int, MidiChangeInfo>& changesMap;
    std::variant<int, double> editorValue=0;
};