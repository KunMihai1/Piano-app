#pragma once
#include <JuceHeader.h>
#include <vector>

/** Schema version for arranger styles, bumped when the model changes. */
static constexpr int ARRANGER_SCHEMA_VERSION = 1;

/** Musical role of a track (drives transposition behaviour in later phases). */
enum class ArrangerPartType { Drum, Perc, Bass, Acc };

/** Style-element type (the section buttons). */
enum class ArrangerSectionType { Intro, Variation, Fill, Break, Ending, CountIn };

/** One timestamped MIDI event, positioned in beats within its section's loop. */
struct TimedBeatEvent
{
    double beats = 0.0;            // [0, loopLengthBeats)
    juce::MidiMessage message;
};

/** One accompaniment track within a section. */
struct ArrangerTrack
{
    juce::String id;
    juce::String name;
    ArrangerPartType partType = ArrangerPartType::Acc;
    int instrument = -1;
    int channel = 1;
    double volume = 100.0;

    std::vector<TimedBeatEvent> pattern;   // Phase 1: the section's loop for this track

    // --- reserved for later phases, inert in Phase 1 ---
    juce::String originalChord;            // Phase 4/5: recorded key/chord
};

/** One style element / section (e.g. Variation 1), an independent loop. */
struct ArrangerSection
{
    juce::String id;
    juce::String name;
    ArrangerSectionType type = ArrangerSectionType::Variation;
    int lengthBars = 1;                    // this section's own loop length
    std::vector<ArrangerTrack> tracks;
};

/** A whole arranger style. */
struct ArrangerStyle
{
    int schemaVersion = ARRANGER_SCHEMA_VERSION;
    juce::String id;
    juce::String name;
    double originalTempo = 120.0;          // BPM patterns were authored at
    int timeSigNum = 4;
    int timeSigDenom = 4;
    std::vector<ArrangerSection> sections;
};
