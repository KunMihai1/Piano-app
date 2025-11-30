/*
  ==============================================================================

    SectionsComponent.cpp
    Created: 30 Nov 2025 10:08:19pm
    Author:  Kisuke

  ==============================================================================
*/

#include "SectionsComponent.h"

StyleSectionComponent::StyleSectionComponent(const std::vector<juce::String>& names, const std::vector<std::vector<juce::String>>& groupNames)
{
    for (int i = 0; i < names.size(); i++)
    {
        auto group = std::make_unique<SectionGroupComponent>(names[i], groupNames[i]);
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

void StyleSectionComponent::assignCallBacks()
{
}
