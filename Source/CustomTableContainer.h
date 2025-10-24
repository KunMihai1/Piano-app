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


/**
 * @brief A container component that hosts and manages a table UI for editing MIDI
 *        events such as note numbers, timestamps, and velocities.
 *
 * Handles:
 *  - Applying changes to an underlying MIDI sequence
 *  - Showing/limiting which MIDI properties are visible/editable
 *  - Confirming change dialogs + undo/erase decisions
 *  - Tracking selected table rows
 */
class TableContainer: public juce::Component
{
public:

    /** Callback triggered when changes should be written back to file */
    std::function<void()> updateToFile;

     /** Callback to detach model as listener when the table is removed */
    std::function<void(TrackPlayerListener* listener)> removeModelFromListener;

     /**
     * @brief Creates the table container using a MIDI sequence and a change-map.
     *
     * @param seq Reference to original MIDI sequence being edited.
     * @param displayName A label name for this table (e.g., instrument/track name).
     * @param channel MIDI channel this table belongs to.
     * @param map Map tracking modified MIDI event data by index.
     */
    TableContainer(juce::MidiMessageSequence& seq, const juce::String& displayName, int channel, std::unordered_map<int, MidiChangeInfo>& map);

    /** @brief Resizes internal components when UI layout changes. */
    void resized() override;

    /** @brief Cleans up listeners and UI components. */
    ~TableContainer() override;

    /** 
     * @brief Resets only the note numbers to their original values for all rows, leaving timestamps and velocities unchanged.
     */
    void onlyNotes();

    /**
     * @brief Resets only the note numbers to their original values for selected rows, leaving other properties unchanged.
     * @param allSelected Set of selected row indices to reset.
     */
    void onlyNotesSelected(const juce::SparseSet<int>& allSelected);

    /** 
     * @brief Resets only the timestamps to their original values for all rows, leaving note numbers and velocities unchanged.
     */
    void onlyTimeStamps();

    /**
     * @brief Resets only the timestamps to their original values for selected rows, leaving other properties unchanged.
     * @param allSelected Set of selected row indices to reset.
     */
    void onlyTimeStampsSelected(const juce::SparseSet<int>& allSelected);


    /** 
     * @brief Resets only the velocities to their original values for all rows, leaving note numbers and timestamps unchanged.
     */
    void onlyVelocities();

    /**
     * @brief Resets only the velocities to their original values for selected rows, leaving other properties unchanged.
     * @param allSelected Set of selected row indices to reset.
     */
    void onlyVelocitiesSelected(const juce::SparseSet<int>& allSelected);

    /**
     * @brief Resets all properties (note number, timestamp, velocity) of all MIDI events
     *        in the table to their original values, applies them to the sequence,
     *        and clears the changes map.
     */
    void allProperties();


    /**
     * @brief Resets all properties (note number, timestamp, velocity) for selected rows only,
     *        leaving other rows unchanged.
     *
     * @param allSelected Set of row indices to reset.
     */
    void allPropertiesSelected(const juce::SparseSet<int>& allSelected);

    /**
     * @brief Checks whether a MIDI change can be considered "unchanged" and therefore
     *        safe to erase from the changes map.
     *
     * @param info The MIDI change information to check.
     * @return true if all properties (note number, timestamp, velocity) match their original values.
     */
    bool validForErase(MidiChangeInfo& info);


    /**
     * @brief Applies the given MIDI change to the sequence at the specified index.
     *
     * @param index Index of the MIDI event in the sequence.
     * @param info Change data containing new note number, timestamp, and velocity.
     * @param modifyVelocity If true, velocity is applied; otherwise, only note number and timestamp may change.
     *
     * @details
     * If the event is a Note On, also updates the corresponding Note Off event to maintain duration
     * unless modifyVelocity is true.
     */
    void applyChangeToSequence(int index, const MidiChangeInfo& info, bool modifyVelocity=false);

    /**
     * @brief Commits all pending changes to the MIDI sequence, refreshes the table model,
     *        preserves selection and scroll position, and triggers the updateToFile callback.
     */
    void onConfirmed();


    /**
     * @brief Shows a confirmation dialog and then applies all properties reset
     *        to either all rows or selected rows, depending on user choice.
     */
    void changeAllUI();

    /**
     * @brief Shows a confirmation dialog and then resets note numbers either for all rows
     *        or only for selected rows, depending on user choice.
     */
    void changeNotesUI();


    /**
     * @brief Shows a confirmation dialog and then resets timestamps either for all rows
     *        or only for selected rows, depending on user choice.
     */
    void changeTimeStampsUI();

    /**
     * @brief Shows a confirmation dialog and then resets velocities either for all rows
     *        or only for selected rows, depending on user choice.
     */
    void changeVelocitiesUI();

    /**
     * @brief Returns true if any table row is currently selected.
     * @return true if at least one row is selected, false otherwise.
     */
    bool anySelected();

    /**
     * @brief Opens a dialog to modify timestamps for all rows or selected rows,
     *        then applies the modification based on the value entered by the user.
     */
    void modifyTimeStampsUI();

    /**
     * @brief Opens a dialog to modify velocities for all rows or selected rows,
     *        then applies the modification based on the value entered by the user.
     */
    void modifyVelocitiesUI();

    /**
     * @brief Modifies timestamps of all rows by adding the current editor value.
     *        The editor value can be an int or double and is applied as an offset.
     */
    void modifyOnlyTimeStamps();


    /**
     * @brief Modifies timestamps of selected rows by adding the current editor value.
     *
     * @param allSelected Set of selected row indices to modify.
     */
    void modifyOnlyTimeStampsSelected(const juce::SparseSet<int>& allSelected);

    /**
     * @brief Modifies velocities of all rows by setting them to the current editor value.
     *        The editor value can be an int or double.
     */
    void modifyOnlyVelocities();

    /**
     * @brief Modifies velocities of selected rows by setting them to the current editor value.
     *
     * @param allSelected Set of selected row indices to modify.
     */
    void modifyOnlyVelocitiesSelected(const juce::SparseSet<int>& allSelected);

    /**
     * @brief Resets one or more properties of the MIDI events using the provided lambda,
     *        then applies the changes to the sequence.
     *
     * @param resetProperty Lambda that modifies the MidiChangeInfo to reset a property.
     * @param selected Optional set of row indices to apply the reset (nullptr = all rows).
     * @param modifyVelocity Whether velocity should also be applied/updated.
     */
    void resetPropertyAndApply(const std::function<void(MidiChangeInfo&)>& resetProperty, const juce::SparseSet<int>* selected = nullptr, bool modifyVelocity=false);


    /**
     * @brief Applies a new property value to one or more rows using the provided lambda.
     *
     * @param newProperty Lambda that modifies the MidiChangeInfo to set a new value.
     * @param selected Optional set of row indices to apply the change (nullptr = all rows).
     * @param modifyVelocity Whether velocity should also be applied/updated.
     */
    void newPropertyAndApply(const std::function<void(MidiChangeInfo&)>& newProperty, const juce::SparseSet<int>* selected = nullptr, bool modifyVelocity=false);

    /**
     * @brief Helper function used internally by newPropertyAndApply to apply
     *        a change to a single row in the sequence and the changes map.
     *
     * @param newProperty Lambda that modifies the MidiChangeInfo for this row.
     * @param selected Optional set of selected rows (may be nullptr for all rows).
     * @param modifyVelocity Whether to apply velocity updates.
     * @param originalIndex Index of the MIDI event in the original sequence.
     */
    void newPropertyHelperFunction(const std::function<void(MidiChangeInfo&)>& newProperty, const juce::SparseSet<int>* selected, bool modifyVelocity, int originalIndex);


    /**
     * @brief Shows a dialog to confirm changes or modifications for selected or all rows.
     *        For "modify" operations, prompts the user to enter a new value for timestamps or velocities.
     *
     * @param title The dialog window title.
     * @param messageAllSelected Message shown when there are selected rows.
     * @param messageNoSelection Message shown when no rows are selected.
     * @param buttonsWithSelection Buttons shown when rows are selected.
     * @param buttonsWithoutSelection Buttons shown when no rows are selected.
     * @param onValidResults Callback called with the index of the chosen button.
     * @param isModify True if this is a "modify" operation that requires user input, false otherwise.
     *
     * @details
     * For modify operations, validates user input before applying it.
     * For non-modify operations, simply shows a confirmation dialog.
     */
    void showModifyChangeDialog(const juce::String& title, const juce::String& messageAllSelected, const juce::String& messageNoSelection,
        const std::vector<juce::String>& buttonsWithSelection, const std::vector<juce::String>& buttonsWithoutSelection,
        std::function<void(int)> onValidResults, bool isModify=false);


    /**
     * @brief Registers the table model as a listener to a subject (e.g., TrackPlayerListener).
     *
     * @param subject Pointer to the Subject to attach; if nullptr, does nothing.
     */
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
