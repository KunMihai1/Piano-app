/*
  ==============================================================================

    ButtonGroupComponent.h
    Created: 30 Nov 2025 10:08:03pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomToolTip.h"

class SectionGroupComponent: public juce::Component {
public:
    SectionGroupComponent(const juce::String& title, const std::vector<juce::String>& buttonNames, bool toggleButton=false);

    void resized() override;

    ~SectionGroupComponent();

    void mouseEnter(const juce::MouseEvent& e) override;

    void mouseExit(const juce::MouseEvent& e) override;

    std::vector<std::unique_ptr<juce::TextButton>>& getButtons();

private:
    juce::Label titleLabel;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
    std::unique_ptr<juce::ToggleButton> toggleButton;
    std::unique_ptr<CustomToolTip> toggleToolTip;

};