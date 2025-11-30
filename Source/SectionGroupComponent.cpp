/*
  ==============================================================================

    ButtonGroupComponent.cpp
    Created: 30 Nov 2025 10:08:03pm
    Author:  Kisuke

  ==============================================================================
*/

#include "SectionGroupComponent.h"

SectionGroupComponent::SectionGroupComponent(const juce::String& title, const std::vector<juce::String>& buttonNames) : titleLabel{ title,title }
{
    addAndMakeVisible(titleLabel);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    for (auto& name : buttonNames)
    {
        auto btn = std::make_unique<juce::TextButton>(name);
        btn->setButtonText(name);

        addAndMakeVisible(*btn);
        buttons.push_back(std::move(btn));
    }
}

void SectionGroupComponent::resized()
{

    int labelHeight = (int)titleLabel.getFont().getHeight() + 2;
    auto area = getLocalBounds();
    titleLabel.setBounds(area.removeFromTop(labelHeight));

    area.removeFromTop(5);

    int count = (int)buttons.size();
    if (count == 0)
        return;
    
    auto row = area;
    int width = row.getWidth() / count;

    for (int i = 0; i < count; i++)
    {
        buttons[i]->setBounds(row.removeFromLeft(width));
        buttons[i]->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }
}

std::vector<std::unique_ptr<juce::TextButton>>& SectionGroupComponent::getButtons()
{
    return buttons;
}
