#include "ArrangerPatternBuilder.h"
#include "ArrangerTime.h"
#include <algorithm>

namespace ArrangerPatternBuilder
{
    std::vector<TimedBeatEvent> buildBeatEvents (const juce::MidiMessageSequence& seq,
                                                 double referenceBpm, int channel)
    {
        std::vector<TimedBeatEvent> out;
        for (int i = 0; i < seq.getNumEvents(); ++i)
        {
            const auto& msg = seq.getEventPointer (i)->message;
            if (! (msg.isNoteOn() || msg.isNoteOff()))
                continue;

            juce::MidiMessage m = msg;
            m.setChannel (channel);
            out.push_back ({ ArrangerTime::secondsToBeats (msg.getTimeStamp(), referenceBpm), m });
        }
        std::stable_sort (out.begin(), out.end(),
                          [] (const TimedBeatEvent& a, const TimedBeatEvent& b) { return a.beats < b.beats; });
        return out;
    }

    ArrangerStyle buildSingleSectionStyle (const std::vector<TrackEntry>& tracks,
                                           int timeSigNum, int timeSigDenom)
    {
        ArrangerStyle style;
        style.timeSigNum = timeSigNum;
        style.timeSigDenom = timeSigDenom;
        if (! tracks.empty())
            style.originalTempo = tracks.front().originalBPM > 0.0 ? tracks.front().originalBPM : 120.0;

        const double bpb = ArrangerTime::beatsPerBar (timeSigNum, timeSigDenom);

        ArrangerSection section;
        section.type = ArrangerSectionType::Variation;
        section.name = "Variation 1";
        section.id = "var_1";

        int melodicChannel = 2;
        int maxBars = 0;

        for (const auto& te : tracks)
        {
            const double refBpm = te.originalBPM > 0.0 ? te.originalBPM : 120.0;
            const bool isPerc = (te.type == TrackType::Percussion);
            const int channel = isPerc ? 10 : melodicChannel++;

            ArrangerTrack at;
            at.id = te.getUniqueID();
            at.name = te.getDisplayName();
            at.partType = isPerc ? ArrangerPartType::Drum : ArrangerPartType::Acc;
            at.instrument = te.instrumentAssociated;
            at.channel = channel;
            at.volume = te.volumeAssociated;
            at.pattern = buildBeatEvents (te.sequence, refBpm, channel);

            double lastBeat = at.pattern.empty() ? 0.0 : at.pattern.back().beats;
            maxBars = std::max (maxBars, ArrangerTime::barsForBeats (lastBeat, bpb));

            section.tracks.push_back (std::move (at));
        }

        section.lengthBars = std::max (1, maxBars);
        style.sections.push_back (std::move (section));
        return style;
    }
}
