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

    void applyChangeToSequence(int index, const MidiChangeInfo& info);

    void onConfirmed();

    void changeAllUI();

    void changeNotesUI();

    void changeTimeStampsUI();

    void changeVelocitiesUI();

    bool anySelected();

    void resetPropertyAndApply(std::function<void(MidiChangeInfo&)> resetProperty, const juce::SparseSet<int>* selected = nullptr);

private:
    /*for future reference, making this whole thing in a separate thread could benefit if mid playing(if it's needed, but prob not since the playing takes place on another thread,
     with the timer so probably it's no issue)
     */
    std::unique_ptr<juce::TableListBox> table;
    std::unique_ptr<MidiNotesTableModel> model;
    std::unique_ptr<juce::Label> disclaimerLabel;
    std::unique_ptr<juce::TextButton> changeMultipleButton = nullptr;
    std::unique_ptr<juce::ComboBox> actionCB = nullptr;
    juce::MidiMessageSequence& originalSequence;
    int lastSelectedRow = -1;
    std::unordered_map<int, MidiChangeInfo>& changesMap;
};