/*
  ==============================================================================

    StyleSection.h
    Created: 28 Nov 2025 12:40:51am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

struct StyleSection {
    juce::String id;
    juce::String name;
    double startTimeSeconds;
    double endTimeSeconds;
    int startBar = -1;
    int endBar = -1;
};
