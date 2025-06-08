/*
  ==============================================================================

    displayGUI.cpp
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#include "displayGUI.h"

Display::Display()
{
    tabComp = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(tabComp.get());
    //tabComp->addTab("Styles", juce::Colours::lightgrey);
}

Display::~Display()
{
}

void Display::paint(juce::Graphics& g)
{
}

void Display::resized()
{
    tabComp->setBounds(0,0,100,50);
}

StyleViewComponent::StyleViewComponent(const juce::String& styleName)
{
    label.setText(styleName, juce::dontSendNotification);

    addAndMakeVisible(label);

    for (int i = 0; i < 8; i++)
    {
        auto* trackButton = new juce::TextButton("Track " + juce::String(i + 1));
        addAndMakeVisible(trackButtons.add(trackButton));
    }
}

void StyleViewComponent::resized()
{
    label.setBounds(10, 10, getWidth() - 30, 30);

    auto area = getLocalBounds().reduced(10).withTop(50);
    for (auto* button : trackButtons)
    {
        button->setBounds(area.removeFromTop(30).reduced(0, 5));
    }
}
