/*
  ==============================================================================

    SelectableLabel.h
    Created: 19 Jul 2025 2:08:23am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SelectableLabel : public juce::Label
{
public:
    std::function<void()> onClick;

    void mouseDown(const juce::MouseEvent& event) override;


private:
    int row=0;
    juce::TableListBox* table=nullptr;
};