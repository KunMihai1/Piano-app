#pragma once
#include <JuceHeader.h>
#include "ArrangerModel.h"
#include <vector>
#include <set>
#include <utility>

/** One event to emit, positioned at a monotonic beat within the advance window. */
struct EmittedEvent
{
    double beats = 0.0;
    juce::MidiMessage message;
};

/**
 * Pure, deterministic loop scheduler. Holds a section's events (in beats within
 * [0, loopLengthBeats)) and, on each advance(from, to), returns the events to emit,
 * wrapping the loop and closing any still-sounding notes at every seam so nothing hangs.
 * No threading, no I/O — fully unit-testable.
 */
class ArrangerScheduler
{
public:
    void setLoop (std::vector<TimedBeatEvent> events, double loopLengthBeats);
    void reset();   // forget which notes are currently sounding

    /** Emit note-offs for every currently-sounding note (used when switching sections
        mid-loop, where the normal loop-seam close would not fire). Clears active-note state. */
    std::vector<EmittedEvent> flushActiveNotes (double atBeats);

    /** Advance the monotonic playhead from `fromBeats` to `toBeats` (from <= to). */
    std::vector<EmittedEvent> advance (double fromBeats, double toBeats);

private:
    void trackActiveNote (const juce::MidiMessage& m);

    std::vector<TimedBeatEvent> sortedEvents;          // sorted by beats
    double loopLen = 0.0;
    std::set<std::pair<int,int>> activeNotes;          // (channel, noteNumber) currently sounding
};
