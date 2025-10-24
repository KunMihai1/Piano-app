/*
  ==============================================================================

    PlayBackSettingsCustomComponent.h
    Created: 10 Oct 2025 10:16:25pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "HelperFunctions.h"

struct PlayBackSettings
{
    int startNote=-1;
    int endNote=-1;
    int leftHandBound = -1;
    int rightHandBound = -1;
    juce::String VID, PID;
};

/**
 * @class PlayBackSettingsComponent
 * @brief Custom JUCE component to configure playback settings for MIDI ranges and hand intervals.
 * 
 * This component allows the user to set:
 * - Start and end notes for playback
 * - Left and right hand boundaries for performance
 * 
 * Changes are immediately persisted to a JSON file and can notify listeners via the onChangingSettings callback.
 */
class PlayBackSettingsComponent: public juce::Component, private juce::ComboBox::Listener {
public:

    /**@brief Callback when settings are changed by the user. */
    std::function<void(PlayBackSettings settings)> onChangingSettings;

    /**
     * @brief Constructor
     * @param lowestNote The lowest MIDI note to display in the combo boxes.
     * @param highestNote The highest MIDI note to display in the combo boxes.
     * @param settingsGiven Reference to the PlayBackSettings structure to read/write current settings.
     */
    PlayBackSettingsComponent(int lowestNote, int highestNote, PlayBackSettings& settings);

    /**
     * @brief Destructor; removes listeners from combo boxes.
     */
    ~PlayBackSettingsComponent();

    /**
     * @brief Resizes and positions all child components.
     */
    void resized() override;

    /**
     * @brief Callback for when any combo box value changes.
     * @param comboBoxThatHasChanged Pointer to the combo box that changed.
     * 
     * Updates the associated PlayBackSettings values, notifies listeners, and saves to JSON.
     */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /**
     * @brief Initializes left- and right-hand interval combo boxes.
     * 
     * Populates the available ranges for each hand and sets initial selections.
     */
    void handIntervalsInit();

    /**
     * @brief Initializes start and stop note combo boxes.
     * 
     * Populates the available notes and sets initial selections.
     */
    void startStopInit();

    /**
     * @brief Updates the component with new playback settings.
     * @param newSettings The new PlayBackSettings structure.
     * 
     * Updates all combo boxes to match the new settings and notifies listeners.
     */
    void setNewSettings(PlayBackSettings& newSettings);

    /**
     * @brief Updates the lowest and highest MIDI notes displayed in the combo boxes.
     * @param lowest The new lowest note.
     * @param highest The new highest note.
     */
    void setLowestHighest(int lowest, int highest);

private:

    int lowestNote, highestNote;
    PlayBackSettings& settings;
    juce::ComboBox startNoteBox;
    juce::ComboBox endNoteBox;
    juce::Label startNoteLabel;
    juce::Label endNoteLabel;

    juce::Label leftHandBoundLabel;
    juce::Label rightHandBoundLabel;

    juce::ComboBox leftHandBoundBox;
    juce::ComboBox rightHandBoundBox;
};
