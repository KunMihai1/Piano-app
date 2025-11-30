/*
  ==============================================================================

    ButtonGroupComponent.h
    Created: 30 Nov 2025 10:08:03pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SectionGroupComponent: public juce::Component {
public:
    SectionGroupComponent(const juce::String& title, const std::vector<juce::String>& buttonNames);

    void resized() override;

    std::vector<std::unique_ptr<juce::TextButton>>& getButtons();

private:
    juce::Label titleLabel;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;

};