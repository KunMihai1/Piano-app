/*
  ==============================================================================

    MidiNotesTableModel.h
    Created: 15 Jul 2025 8:46:13pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "TrackEntry.h"
#include "SelectableLabel.h"
#include "TrackPlayerListener.h"


/**
 * @brief Table model for displaying and editing MIDI note-on events.
 * 
 * This class extracts note-on events from a MidiMessageSequence and provides
 * functionality to display them in a JUCE TableListBox or similar component.
 * Supports editing note number, timestamp, velocity, and tracking changes.
 */
class MidiNotesTableModel: public juce::TableListBoxModel, public TrackPlayerListener
{
public:
    struct EventWithIndex {
        const juce::MidiMessageSequence::MidiEventHolder* event;
        int originalIndex;
    };

    /** @brief Callback triggered when a cell is updated (row index provided). */
    std::function<void(int rowNumber)> onUpdate;

    /** @brief Callback to request selection of a row. */
    std::function<void(int row)> onRequestSelectRow;

    /** 
   * @brief Optional callback to refresh the table UI after sorting or other changes. 
   * Can be set externally to trigger a repaint or update in the TableListBox.
   */
    std::function<void()> refreshData;

    /** @brief Callback to repaint a row during MIDI playback. */
    std::function<void(int row)> onMidPlayRepaint;

    /**
     * @brief Constructs the table model from a MIDI sequence.
     * @param seq Reference to the MidiMessageSequence containing events.
     * @param ch MIDI channel associated with the sequence.
     * @param map Reference to a map to track changes for each original note.
     */
    MidiNotesTableModel(const juce::MidiMessageSequence& sequence ,int channel, std::unordered_map<int, MidiChangeInfo>& map);

    /** @brief Returns the number of note-on events (rows). */
    int getNumRows() override;

    /**
     * @brief Sorts the events by a column (note, timestamp, velocity) and order.
     * @param newSortColumnId Column to sort (1=note, 2=timestamp, 3=velocity)
     * @param isForwards True for ascending, false for descending
     */
    void sortOrderChanged(int newSortColumnId, bool isForwards);


    /** @brief Paints the background of a row. */
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    /** @brief Paints the contents of a cell in the table. */
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool) override;


    /**
     * @brief Provides a custom JUCE Component (label) for editing a cell.
     * @param rowNumber Row index
     * @param columnId Column index
     * @param isRowSelected Whether the row is selected
     * @param existingComponentToUpdate Component to reuse (if any)
     * @return Component* Pointer to component to display/edit
     */
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;


    /**
     * @brief Converts a note name (e.g., "C#4") to a MIDI note number.
     * @param noteName Name of the note
     * @param octaveNoteMiddleC Optional middle C octave (default 4)
     * @return int MIDI note number (0-127), or -1 if invalid
     */
    int getMidiNoteNumberFromName(const juce::String& name, int octaveMiddleC = 4);


    /** @brief Finds the first numeric or '-' character in a note name string. */
    int findFirstDigitName(const juce::String& name);


    /** @brief Records changes to a note when it didn’t exist in the map. */
    void handleMapNonExistingCase(juce::MidiMessage& oldMessage, juce::MidiMessage& newMessage, MidiChangeInfo& str);


    /** @brief Updates an existing entry in the changes map. */
    void handleMapExistingCase(juce::MidiMessage& newMessage, MidiChangeInfo& str);


    /** @brief Checks if two MIDI messages are identical. */
    bool areMidiMessagesEqual(const juce::MidiMessage& a, const juce::MidiMessage& b);


    /** @brief Rebuilds the note-on event vector from a MidiMessageSequence. */
    void refreshVectorFromSequence(const juce::MidiMessageSequence& seq);


    /** @brief Gets the table row corresponding to an original sequence index. */
    int getRowFromOriginalIndex(int originalIndex);

    /** @brief Gets the original sequence index from a table row. */
    int getOriginalIndexFromRow(int row);

    /** @brief Returns the timestamp of the first note-on event. */
    double getFirstNoteOnTimeStamps();

    /** @brief Returns the timestamp of the previous note-on event before the given index. */
    double getPreviousNoteOnTimeStamp(int currentIndex);

    /** @brief Returns the timestamp of the current note-on event at the given index. */
    double getCurrentNoteOnTimeStamp(int currentIndex);

    std::vector<EventWithIndex> getEvents();


    /** @brief Returns the row index corresponding to a given time. */
    int getRowFromTime(double currentTime);

    /** @brief Updates the currently highlighted row based on playback time. */
    void updateCurrentRowBasedOnTime(double currentTime) override;

private:
    int highlightedRow = -1;
    std::unordered_map<int, int> originalIndexToRowMap;
    std::vector<EventWithIndex> noteOnEvents;
    int channel;
    std::unordered_map<int, MidiChangeInfo>* changesMap=nullptr;
};
