/*
  ==============================================================================

    SectionsComponent.h
    Created: 30 Nov 2025 10:08:19pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionGroupComponent.h"

class StyleSectionComponent: public juce::Component
{
public:
    std::function<void()> undoLastClickedForOtherGroupSections;


    StyleSectionComponent(const std::vector<juce::String>& names, const std::vector<std::vector<juce::String>>& groupNames,
        const std::unordered_map<juce::String, std::function<void()>>& buttonCallbacks);

    void resized() override;

    void applyChangeColour(juce::TextButton& button, bool activated = false);

    juce::TextButton* getLastUsedButton();

    void deactivateLastClicked();

private:
    juce::OwnedArray<SectionGroupComponent> sectionGroups;
    const std::unordered_map<juce::String, std::function<void()>>& callbacks;
    juce::TextButton* lastClickedButton=nullptr;
    std::unordered_map<juce::String, bool>* lastActivationMap=nullptr;
    bool isProgramaticClick = false;

    void assignCallBacks();
};