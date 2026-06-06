#include "ArrangerSourceBuilder.h"
#include "ArrangerChannels.h"
#include "ArrangerPatternBuilder.h"

namespace ArrangerSourceBuilder
{
    std::vector<SourceTrackFile> fromTrackEntries (const std::vector<TrackEntry>& tracks,
                                                   double referenceBpm)
    {
        if (referenceBpm <= 0.0) referenceBpm = 120.0;

        std::vector<SourceTrackFile> out;
        int melodicIndex = 0;

        for (const auto& te : tracks)
        {
            const bool isPerc = (te.type == TrackType::Percussion);
            int channel;
            if (isPerc)
            {
                channel = ArrangerChannels::kDrumChannel;
            }
            else
            {
                channel = ArrangerChannels::melodicChannelForIndex (melodicIndex++);
                if (channel < 0)
                    continue; // melodic budget exhausted; drop this track
            }

            SourceTrackFile st;
            st.id        = te.getUniqueID();
            st.name      = te.getDisplayName();
            st.partType  = isPerc ? ArrangerPartType::Drum : ArrangerPartType::Acc;
            st.channel   = channel;
            st.instrument = te.instrumentAssociated;
            st.volume    = te.volumeAssociated;
            st.events    = ArrangerPatternBuilder::buildBeatEvents (te.sequence, referenceBpm, channel);
            out.push_back (std::move (st));
        }

        return out;
    }
}
