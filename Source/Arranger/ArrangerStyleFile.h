#pragma once
#include <JuceHeader.h>
#include "ArrangerModel.h"   // ArrangerPartType, ArrangerSectionType, ArrangerAfterComplete, TimedBeatEvent
#include <vector>

/** One source track in a saved style: events stored ONCE, beats measured from bar 1. */
struct SourceTrackFile
{
    juce::String id, name;
    ArrangerPartType partType = ArrangerPartType::Acc;
    int    channel    = 2;
    int    instrument = -1;
    double volume     = 100.0;
    std::vector<TimedBeatEvent> events;   // note-on/off only
    // effects slot intentionally omitted in Part A (reserved for Phase 4+)
};

/** One section as a bar-window into the source tracks. */
struct SectionWindow
{
    juce::String id, name;
    ArrangerSectionType   type          = ArrangerSectionType::Variation;
    int                   startBar       = 1;   // 1-based
    int                   lengthBars     = 1;
    ArrangerAfterComplete afterComplete  = ArrangerAfterComplete::Loop;
};

/** A whole self-contained arranger style as stored on disk. */
struct ArrangerStyleFile
{
    int schemaVersion = 3;
    juce::String id, name;
    double originalTempo = 120.0;
    int timeSigNum = 4, timeSigDenom = 4;
    std::vector<SourceTrackFile> sourceTracks;
    std::vector<SectionWindow>   sections;
};
