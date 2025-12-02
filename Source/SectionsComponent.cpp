/*
  ==============================================================================

    SectionsComponent.cpp
    Created: 30 Nov 2025 10:08:19pm
    Author:  Kisuke

  ==============================================================================
*/

#include "SectionsComponent.h"

StyleSectionComponent::StyleSectionComponent(const std::vector<juce::String>& names, const std::vector<std::vector<juce::String>>& groupNames,
    const std::unordered_map<juce::String, std::function<void()>>& buttonCallbacks): callbacks{buttonCallbacks}
{
    for (int i = 0; i < names.size(); i++)
    {
        bool toggleNeeded = false;
        if (names[i].equalsIgnoreCase("fills"))
            toggleNeeded = true;
        auto group = std::make_unique<SectionGroupComponent>(names[i], groupNames[i],toggleNeeded);
        addAndMakeVisible(group.get());
        sectionGroups.add(std::move(group));
    }

    assignCallBacks();
}

void StyleSectionComponent::resized()
{
    auto area = getLocalBounds();
    int groupWidth = area.getWidth() / (sectionGroups.size() > 0 ? sectionGroups.size() : 1);

    for (auto* group : sectionGroups)
    {
        group -> setBounds(area.removeFromLeft(groupWidth).reduced(5));
    }
}

void StyleSectionComponent::applyChangeColour(juce::TextButton& button, bool activated)
{
    if (activated)
    {
        button.removeColour(juce::TextButton::buttonColourId);
    }
    else
    {
        button.setColour(juce::TextButton::buttonColourId, juce::Colours::green.withAlpha(0.8f));
    }

    button.repaint();
}

void StyleSectionComponent::assignCallBacks()
{
    for (auto* group : sectionGroups)
    {
        auto& buttons = group->getButtons();
        for (auto& button : buttons)
        {
            juce::String buttonText = button->getButtonText();
            auto it = callbacks.find(buttonText);

            if (it != callbacks.end())
            {
                auto callbackCopy = it->second;
                auto btnPtr = button.get();
                auto& activationMap = group->getActivationMap();
                button->onClick = [callbackCopy, buttonText, this, btnPtr, &activationMap]()
                {
                    callbackCopy();

                    bool activatedState = false;
                    auto itAct = activationMap.find(btnPtr->getButtonText());
                    if (itAct != activationMap.end())
                        activatedState = itAct->second;
                    applyChangeColour(*btnPtr, activatedState);
                    activationMap[buttonText] = !activatedState;
                };
            }
        }
    }
}
