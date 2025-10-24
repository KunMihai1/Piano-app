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
 * @brief A component that allows users to search and select instruments from a list.
 *
 * @details
 * Provides a text editor for searching and a list box for displaying filtered instruments.
 * Selecting an instrument triggers a callback with the selected instrument index and name.
 */
class InstrumentChooserComponent: public juce::Component, private juce::TextEditor::Listener, private juce::ListBoxModel
{
public:
    /** 
     * @brief Callback function called when an instrument is selected.
     * @param instrumentIndex Index of the selected instrument in the original list
     * @param name Name of the selected instrument
     */
    std::function<void(int instrumentIndex, const juce::String& name)> instrumentSelectedFunction;


    /**
     * @brief Constructor
     * @param listToShow Reference to a juce::StringArray containing all instrument names
     */
    InstrumentChooserComponent(juce::StringArray& listToShow);

    /** @brief Destructor */
    ~InstrumentChooserComponent() override;


    /** @brief Resizes and lays out the search box and instrument list box */
    void resized() override;


    /** @brief Returns the number of rows in the list box */
    int getNumRows() override;

    /** 
     * @brief Paints a single row in the list box
     * @param rowNumber Index of the row
     * @param g Graphics context
     * @param width Row width
     * @param height Row height
     * @param rowIsSelected Whether the row is currently selected
     */
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    /** 
     * @brief Called when a row in the list box is clicked
     * @param row Index of the clicked row
     * @param ev Mouse event information
     */
    void listBoxItemClicked(int row, const juce::MouseEvent& ev) override;

private:

    juce::StringArray& allInstruments;
    juce::StringArray filteredInstruments;

    juce::TextEditor searchBox;
    juce::ListBox instrumentListBox;


    void textEditorTextChanged(juce::TextEditor& editor) override;
    void updateListBasedOnSearch();
};
