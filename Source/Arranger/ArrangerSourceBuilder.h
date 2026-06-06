#pragma once
#include <JuceHeader.h>
#include "ArrangerStyleFile.h"
#include "TrackEntry.h"
#include <vector>

namespace ArrangerSourceBuilder
{
    /** Convert recorded tracks into source tracks for a style: percussion -> channel 10,
        melodic -> 2..9 then 11..13 (ArrangerChannels). Tracks past the melodic budget are
        dropped. `referenceBpm` must be the tempo `TrackEntry::sequence` is scaled to. */
    std::vector<SourceTrackFile> fromTrackEntries (const std::vector<TrackEntry>& tracks,
                                                   double referenceBpm);
}
