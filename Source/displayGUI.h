/*
  ==============================================================================

    displayGUI.h
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class StyleViewComponent : public juce::Component
{
public:
    StyleViewComponent(const juce::String& styleName);

    void resized() override;

private:
    juce::Label label;
    juce::OwnedArray<juce::TextButton> trackButtons;
};

class Display: public  juce::Component
{
public:
    Display();
    ~Display() override;

    void paint(juce::Graphics& g) override;

    void resized() override;


private:
    std::unique_ptr<juce::TabbedComponent> tabComp;
};
