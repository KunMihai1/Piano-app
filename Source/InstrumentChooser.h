/*
  ==============================================================================

    InstrumentChooser.h
    Created: 9 Jul 2025 12:46:50am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * @class InstrumentChooserComponent
 * @brief Provides a searchable list component for selecting instruments.
 *
 * This component displays a text editor for searching and a list box for
 * selecting instruments from a provided list. When an instrument is selected,
 * a callback function is triggered with the selected instrument index and name.
 */
class InstrumentChooserComponent: public juce::Component, 
                                 private juce::TextEditor::Listener, 
                                 private juce::ListBoxModel
{
public:
    /**
     * @brief Callback function triggered when an instrument is selected.
     *
     * @param instrumentIndex Index of the selected instrument in the original list
     * @param name Name of the selected instrument
     */
    std::function<void(int instrumentIndex, const juce::String& name)> instrumentSelectedFunction;

    /**
     * @brief Constructor
     * @param listToShow Reference to a StringArray containing all instrument names
     */
    InstrumentChooserComponent(juce::StringArray& listToShow);

    /** Destructor */
    ~InstrumentChooserComponent() override;

    /** @brief Handles resizing of child components */
    void resized() override;

    /** @brief Returns the number of rows in the list box */
    int getNumRows() override;

    /**
     * @brief Paints a single row in the list box
     * @param rowNumber Row index
     * @param g Graphics object
     * @param width Width of the row
     * @param height Height of the row
     * @param rowIsSelected True if the row is selected
     */
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    /**
     * @brief Called when a list box item is clicked
     * @param row Row index clicked
     * @param ev Mouse event details
     */
    void listBoxItemClicked(int row, const juce::MouseEvent& ev) override;

private:
    juce::StringArray& allInstruments;     ///< Reference to the full instrument list
    juce::StringArray filteredInstruments; ///< Filtered instruments based on search

    juce::TextEditor searchBox;            ///< Search box for filtering instruments
    juce::ListBox instrumentListBox;       ///< List box displaying instruments

    /** @brief Called when text in the search box changes */
    void textEditorTextChanged(juce::TextEditor& editor) override;

    /** @brief Updates the filtered list of instruments based on the search text */
    void updateListBasedOnSearch();
};
