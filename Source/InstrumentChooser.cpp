/*
  ==============================================================================

    InstrumentChooser.cpp
    Created: 9 Jul 2025 12:46:50am
    Author:  Kisuke

  ==============================================================================
*/

#include "InstrumentChooser.h"

InstrumentChooserComponent::InstrumentChooserComponent(juce::StringArray& arr): allInstruments{arr}
{
    searchBox.setTextToShowWhenEmpty("Click HERE to Search", juce::Colours::darkgrey);
    searchBox.addListener(this);
    addAndMakeVisible(searchBox);

    instrumentListBox.setModel(this);
    addAndMakeVisible(instrumentListBox);

    updateListBasedOnSearch();
}

InstrumentChooserComponent::~InstrumentChooserComponent()
{
    searchBox.removeListener(this);
}

void InstrumentChooserComponent::resized()
{
    auto area = getLocalBounds().reduced(6);
    searchBox.setBounds(area.removeFromTop(18));
    area.removeFromTop(4);
    instrumentListBox.setBounds(area);
}

int InstrumentChooserComponent::getNumRows()
{
    return filteredInstruments.size();
}

void InstrumentChooserComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::coral);
    g.setColour(juce::Colours::peachpuff);
    g.setFont(16.0f);
    g.drawText(filteredInstruments[rowNumber], 4, 0, width, height, juce::Justification::centredLeft);
}

void InstrumentChooserComponent::listBoxItemClicked(int row, const juce::MouseEvent& ev)
{
    juce::String name = filteredInstruments[row];
    for (int i = 0; i < allInstruments.size(); i++)
    {
        if (name == allInstruments[i])
        {
            if (instrumentSelectedFunction)
                instrumentSelectedFunction(i, name);
            break;
        }
    }
}

void InstrumentChooserComponent::textEditorTextChanged(juce::TextEditor& editor)
{
    updateListBasedOnSearch();
}

void InstrumentChooserComponent::updateListBasedOnSearch()
{
    filteredInstruments.clear();
    auto text = searchBox.getText().trim().toLowerCase();
    for (auto& instr : allInstruments)
    {
        if (text.isEmpty() || instr.toLowerCase().contains(text))
            filteredInstruments.add(instr);
    }

    instrumentListBox.updateContent();
    instrumentListBox.repaint();
}
