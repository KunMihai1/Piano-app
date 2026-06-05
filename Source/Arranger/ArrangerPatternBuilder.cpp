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
                                           int timeSigNum, int timeSigDenom,
                                           double referenceBpm)
    {
        if (referenceBpm <= 0.0)
            referenceBpm = 120.0;

        ArrangerStyle style;
        style.timeSigNum = timeSigNum;
        style.timeSigDenom = timeSigDenom;
        style.originalTempo = referenceBpm;

        const double bpb = ArrangerTime::beatsPerBar (timeSigNum, timeSigDenom);

        ArrangerSection section;
        section.type = ArrangerSectionType::Variation;
        section.name = "Variation 1";
        section.id = "var_1";

        int melodicChannel = 2;
        int maxBars = 0;

        for (const auto& te : tracks)
        {
            const bool isPerc = (te.type == TrackType::Percussion);
            const int channel = isPerc ? 10 : melodicChannel++;

            ArrangerTrack at;
            at.id = te.getUniqueID();
            at.name = te.getDisplayName();
            at.partType = isPerc ? ArrangerPartType::Drum : ArrangerPartType::Acc;
            at.instrument = te.instrumentAssociated;
            at.channel = channel;
            at.volume = te.volumeAssociated;
            // te.sequence is already scaled to referenceBpm's timebase, so convert with referenceBpm.
            at.pattern = buildBeatEvents (te.sequence, referenceBpm, channel);

            double lastBeat = at.pattern.empty() ? 0.0 : at.pattern.back().beats;
            maxBars = std::max (maxBars, ArrangerTime::barsForBeats (lastBeat, bpb));

            section.tracks.push_back (std::move (at));
        }

        section.lengthBars = std::max (1, maxBars);
        style.sections.push_back (std::move (section));
        return style;
    }

    ArrangerStyle buildDemoMultiSectionStyle (const std::vector<TrackEntry>& tracks,
                                              int timeSigNum, int timeSigDenom,
                                              double referenceBpm)
    {
        ArrangerStyle base = buildSingleSectionStyle (tracks, timeSigNum, timeSigDenom, referenceBpm);

        ArrangerStyle style;
        style.timeSigNum    = timeSigNum;
        style.timeSigDenom  = timeSigDenom;
        style.originalTempo = base.originalTempo;

        if (base.sections.empty())
            return style;

        const double bpb     = ArrangerTime::beatsPerBar (timeSigNum, timeSigDenom);
        ArrangerSection var  = base.sections.front();     // the full loop, correct channels/length
        const int loopBars   = juce::jmax (1, var.lengthBars);

        var.id = "var_1"; var.name = "Variation 1";
        var.type = ArrangerSectionType::Variation;
        var.afterComplete = ArrangerAfterComplete::Loop;

        // Slice a range of whole bars out of a cloned section and rebase to beat 0. Keeps the
        // throwaway demo Intro/Fill/Ending short so the transport (intro-once, fill-once,
        // ending-once-then-stop) is observable instead of replaying the whole-song Variation
        // loop. Lengths are a compromise until Phase 3 supplies real per-section phrases.
        auto sliceBars = [bpb] (ArrangerSection sec, double startBeats, int numBars) -> ArrangerSection
        {
            const int    bars     = juce::jmax (1, numBars);
            const double endBeats = startBeats + (double) bars * bpb;
            for (auto& tr : sec.tracks)
            {
                std::vector<TimedBeatEvent> sliced;
                for (const auto& ev : tr.pattern)
                    if (ev.beats >= startBeats - 1e-9 && ev.beats < endBeats - 1e-9)
                        sliced.push_back ({ ev.beats - startBeats, ev.message });
                tr.pattern = std::move (sliced);
            }
            sec.lengthBars = bars;
            return sec;
        };

        const int    endingBars   = juce::jmin (4, loopBars);                  // a short wind-down
        const double lastBarStart = (double) (loopBars - 1)          * bpb;
        const double endingStart  = (double) (loopBars - endingBars) * bpb;

        ArrangerSection intro = sliceBars (var, 0.0, 1);                 // first bar -> lead-in
        intro.id = "intro_1"; intro.name = "Intro 1";
        intro.type = ArrangerSectionType::Intro;
        intro.afterComplete = ArrangerAfterComplete::FallThrough;

        ArrangerSection fill = sliceBars (var, lastBarStart, 1);         // last bar -> fill accent
        fill.id = "fill_1"; fill.name = "Fill 1";
        fill.type = ArrangerSectionType::Fill;
        fill.afterComplete = ArrangerAfterComplete::FallThrough;

        ArrangerSection ending = sliceBars (var, endingStart, endingBars); // last few bars -> wind-down, then stop
        ending.id = "ending_1"; ending.name = "Ending 1";
        ending.type = ArrangerSectionType::Ending;
        ending.afterComplete = ArrangerAfterComplete::Stop;

        style.sections.push_back (std::move (intro));
        style.sections.push_back (std::move (var));
        style.sections.push_back (std::move (fill));
        style.sections.push_back (std::move (ending));
        return style;
    }
}
