/*
  ==============================================================================

    InstrumentChooser.h
    Created: 9 Jul 2025 12:46:50am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class InstrumentChooserComponent: public juce::Component, private juce::TextEditor::Listener, private juce::ListBoxModel
{
public:
    std::function<void(int instrumentIndex, const juce::String& name)> instrumentSelectedFunction;

    InstrumentChooserComponent(juce::StringArray& listToShow);
    ~InstrumentChooserComponent() override;

    void resized() override;

    int getNumRows() override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void listBoxItemClicked(int row, const juce::MouseEvent& ev) override;

private:

    juce::StringArray& allInstruments;
    juce::StringArray filteredInstruments;

    juce::TextEditor searchBox;
    juce::ListBox instrumentListBox;


    void textEditorTextChanged(juce::TextEditor& editor) override;
    void updateListBasedOnSearch();
};