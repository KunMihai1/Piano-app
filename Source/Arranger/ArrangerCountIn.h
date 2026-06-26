#pragma once
#include <vector>
#include <cmath>

/**
 * Pure helper for the Phase-6b Count-In: one bar of metronome clicks before the groove starts.
 *
 * A click fires on each integer beat in a half-open window [fromBeats, toBeats); beat 0 of the bar is
 * accented (louder). Clicks are GM percussion (channel 10, side-stick note 37). No JUCE/threads — so
 * the "which clicks fire when" logic is unit-testable in isolation; the engine just dispatches them.
 */
struct ClickEvent
{
    int    channel;
    int    note;
    int    velocity;
    double beat;
};

struct ArrangerCountIn
{
    static constexpr int  kChannel       = 10;   // GM drums
    static constexpr int  kNote          = 37;   // side stick
    static constexpr int  kAccentVel     = 110;  // beat 1
    static constexpr int  kVel           = 70;   // other beats

    /** Clicks whose beat lands in [fromBeats, toBeats), for a count-in bar of `beatsPerBar` beats.
        Half-open so adjacent windows never double-fire a beat at the seam. */
    static std::vector<ClickEvent> clicksInWindow (double fromBeats, double toBeats, double beatsPerBar)
    {
        std::vector<ClickEvent> out;
        const int bars = (int) std::lround (beatsPerBar);
        const int first = (int) std::ceil (fromBeats - 1e-9);
        const int last  = (int) std::ceil (toBeats   - 1e-9);   // exclusive
        for (int b = first; b < last; ++b)
        {
            if (b < 0 || b >= bars)
                continue;
            out.push_back ({ kChannel, kNote, (b == 0 ? kAccentVel : kVel), (double) b });
        }
        return out;
    }
};
