#pragma once
#include <JuceHeader.h>
#include "ArrangerModel.h"
#include "TrackEntry.h"
#include <vector>

/** Builds Phase-1 arranger content (beat-based patterns, single-section style) from TrackEntry data. */
namespace ArrangerPatternBuilder
{
    /** Convert a seconds-timestamped sequence to note-on/off beat events on `channel`. */
    std::vector<TimedBeatEvent> buildBeatEvents (const juce::MidiMessageSequence& seq,
                                                 double referenceBpm, int channel);

    /** Wrap the selected tracks as one looped section. Channels: perc->10, melodic->2,3,4,…
        Loop length = longest track rounded up to whole bars.
        `referenceBpm` MUST be the tempo `TrackEntry::sequence` is currently scaled to
        (i.e. the live playback tempo) so beat positions line up with the bar grid. */
    ArrangerStyle buildSingleSectionStyle (const std::vector<TrackEntry>& tracks,
                                           int timeSigNum, int timeSigDenom,
                                           double referenceBpm);
}
