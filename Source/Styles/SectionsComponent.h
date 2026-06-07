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

    juce::OwnedArray<SectionGroupComponent>& getSectionGroups();

    /** When true, button clicks only fire their callback (route to the engine) and do NOT toggle their
        own highlight — the highlight is set externally via setHighlightedButton from the engine's
        actual active section. Used in arranger mode so the lit button is always 100% accurate. */
    void setEngineDrivenHighlight(bool shouldBeEngineDriven) { engineDrivenHighlight = shouldBeEngineDriven; }

    /** Highlight exactly the button whose text matches `buttonName` (case-insensitive), clearing any
        other highlight in this component. Empty string clears all. Does not fire callbacks. */
    void setHighlightedButton(const juce::String& buttonName);

private:
    juce::OwnedArray<SectionGroupComponent> sectionGroups;
    const std::unordered_map<juce::String, std::function<void()>>& callbacks;
    juce::TextButton* lastClickedButton=nullptr;
    std::unordered_map<juce::String, bool>* lastActivationMap=nullptr;
    bool engineDrivenHighlight=false;

    void assignCallBacks();
};