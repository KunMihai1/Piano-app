/*
  ==============================================================================

    SelectableLabel.h
    Created: 19 Jul 2025 2:08:23am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * @class SelectableLabel
 * @brief A JUCE Label that supports click callbacks.
 * 
 * This class extends juce::Label to provide a simple `onClick` callback 
 * when the label is clicked. It can be used in UI components where 
 * labels need to act as interactive elements.
 */
class SelectableLabel : public juce::Label
{
public:
    /**
     * @brief Callback invoked when the label is clicked.
     * 
     * Set this function to handle mouse clicks on the label.
     */
    std::function<void()> onClick;

    /**
     * @brief Handles mouse down events.
     * @param event The mouse event that triggered this callback.
     * 
     * If the `onClick` callback is set, it is called first, then the base 
     * juce::Label mouseDown is invoked.
     */
    void mouseDown(const juce::MouseEvent& event) override;


private:
    int row=0;
    juce::TableListBox* table=nullptr;
};
