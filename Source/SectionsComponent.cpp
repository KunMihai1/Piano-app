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

juce::TextButton* StyleSectionComponent::getLastUsedButton()
{
    return lastClickedButton;
}

void StyleSectionComponent::deactivateLastClicked()
{
    if (lastClickedButton && lastActivationMap)
    {
        auto btnText = lastClickedButton->getButtonText();

        bool oldState = false;
        auto it = lastActivationMap->find(btnText);
        if (it != lastActivationMap->end())
            oldState = it->second;

        applyChangeColour(*lastClickedButton, oldState);
        (*lastActivationMap)[btnText] = !oldState;
        lastClickedButton = nullptr;
        lastActivationMap = nullptr;
    }
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
                    if (undoLastClickedForOtherGroupSections)
                        undoLastClickedForOtherGroupSections();
                    if (lastClickedButton && lastActivationMap && lastClickedButton!=btnPtr)
                    {
                        bool activatedStateLast = false;
                        auto itLast = lastActivationMap->find(lastClickedButton->getButtonText());
                        if (itLast != lastActivationMap->end())
                        {
                            activatedStateLast = itLast->second;
                            applyChangeColour(*lastClickedButton, activatedStateLast);
                            (*lastActivationMap)[lastClickedButton->getButtonText()] = !activatedStateLast;
                        }
                    }
                    

                    bool activatedState = false;
                    auto itAct = activationMap.find(btnPtr->getButtonText());
                    if (itAct != activationMap.end())
                        activatedState = itAct->second;

                    bool newState = !activatedState;
                    
                    applyChangeColour(*btnPtr, activatedState);

                    activationMap[buttonText] = newState;
                    
                    if (newState)
                    {
                        lastClickedButton = btnPtr;
                        lastActivationMap = &activationMap;
                    }
                    else {
                        lastClickedButton = nullptr;
                        lastActivationMap = nullptr;
                    }

                    callbackCopy();
                };
            }
        }
    }
}
