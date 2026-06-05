#include "ArrangerScheduler.h"
#include <algorithm>
#include <cmath>

void ArrangerScheduler::setLoop (std::vector<TimedBeatEvent> events, double loopLengthBeats)
{
    sortedEvents = std::move (events);
    std::stable_sort (sortedEvents.begin(), sortedEvents.end(),
                      [] (const TimedBeatEvent& a, const TimedBeatEvent& b) { return a.beats < b.beats; });
    loopLen = loopLengthBeats;
    activeNotes.clear();
}

void ArrangerScheduler::reset()
{
    activeNotes.clear();
}

void ArrangerScheduler::trackActiveNote (const juce::MidiMessage& m)
{
    const auto key = std::make_pair (m.getChannel(), m.getNoteNumber());
    if (m.isNoteOn() && m.getVelocity() > 0)
        activeNotes.insert (key);
    else if (m.isNoteOff() || (m.isNoteOn() && m.getVelocity() == 0))
        activeNotes.erase (key);
}

std::vector<EmittedEvent> ArrangerScheduler::advance (double fromBeats, double toBeats)
{
    std::vector<EmittedEvent> result;
    if (loopLen <= 0.0 || toBeats <= fromBeats)
        return result;

    double pos = fromBeats;
    while (pos < toBeats - 1e-12)
    {
        const double iterationIndex    = std::floor (pos / loopLen);
        const double iterationStartAbs = iterationIndex * loopLen;
        const double nextWrapAbs       = iterationStartAbs + loopLen;
        const double phaseStart        = pos - iterationStartAbs;
        const double segmentEndAbs     = std::min (toBeats, nextWrapAbs);
        const double phaseEnd          = segmentEndAbs - iterationStartAbs;

        for (const auto& ev : sortedEvents)
        {
            if (ev.beats >= phaseStart - 1e-12 && ev.beats < phaseEnd - 1e-12)
            {
                EmittedEvent e { iterationStartAbs + ev.beats, ev.message };
                result.push_back (e);
                trackActiveNote (ev.message);
            }
        }

        // If we actually reached a seam within the window, close hanging notes there.
        if (std::abs (segmentEndAbs - nextWrapAbs) < 1e-9 && nextWrapAbs <= toBeats + 1e-12)
        {
            for (const auto& key : activeNotes)
                result.push_back ({ nextWrapAbs, juce::MidiMessage::noteOff (key.first, key.second) });
            activeNotes.clear();
        }

        pos = segmentEndAbs;
    }

    return result;
}
