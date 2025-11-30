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
    StyleSectionComponent(const std::vector<juce::String>& names, const std::vector<std::vector<juce::String>>& groupNames);

    void resized() override;

private:
    juce::OwnedArray<SectionGroupComponent> sectionGroups;

    void assignCallBacks();
};