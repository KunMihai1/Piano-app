#pragma once
#include <JuceHeader.h>
#include "ArrangerModel.h"
#include "ArrangerScheduler.h"
#include "ArrangerSectionSequencer.h"
#include <atomic>
#include <vector>

/**
 * Real-time arranger playback: loops the active section of a style to MIDI-out and/or
 * an inject callback (SFZ). Parallel to MultipleTrackPlayer; the classic player is untouched.
 */
class ArrangerEngine : private juce::HighResolutionTimer
{
public:
    std::function<void (const juce::MidiMessage&)> onMidiMessage; // SFZ inject
    std::function<void (double)> onElapsedBeats;                  // for the beat bar

    explicit ArrangerEngine (std::weak_ptr<juce::MidiOutput> out);
    ~ArrangerEngine() override;

    void setDeviceOutput (std::weak_ptr<juce::MidiOutput> out);
    void setStyle (ArrangerStyle newStyle);   // builds the scheduler from section 0
    void setBpm (double bpm);

    /** Queue a section change (applied at the next bar boundary while playing). */
    void queueSection (ArrangerSectionType type, const juce::String& name);
    /** Choose which section a subsequent start() begins on (used while stopped). */
    void selectStartSection (ArrangerSectionType type, const juce::String& name);

    int getActiveSectionIndex() const;
    int peekPendingStartIndex() const { return pendingStartIndex; } // test seam

    void start();
    void stop();
    bool isPlaying() const { return playing.load(); }

    double getLoopLengthBeats() const { return loopLengthBeats; }

    /** Render and dispatch events for the monotonic beat window [fromBeats, toBeats). Public for tests. */
    void renderRange (double fromBeats, double toBeats);

private:
    void hiResTimerCallback() override;
    void dispatch (const juce::MidiMessage& m);
    void sendAllNotesOff();
    void sendInstrumentSetup();   // program-change + volume per channel, like the classic player
    void rebuildFromStyle();
    void updateActiveLoopLength();
    void haltAudio();             // silence + reset state, WITHOUT stopping the timer
    int  indexOfSection (ArrangerSectionType type, const juce::String& name) const;

    std::weak_ptr<juce::MidiOutput> outputDevice;
    ArrangerStyle style;
    std::vector<ArrangerScheduler> schedulers;       // one per section
    ArrangerSectionSequencer       sequencer;
    int  currentSchedulerIndex = -1;                 // which scheduler is currently sounding
    int  pendingStartIndex     = -1;                 // section start() should begin on (-1 = default)
    bool timerShouldStop       = false;              // set on the timer thread, honoured on the msg thread

    double currentBpm = 120.0;
    double loopLengthBeats = 0.0;
    double playheadBeats = 0.0;     // monotonic beat position since start()
    double lastNowSeconds = 0.0;    // wall-clock of previous tick (for delta accumulation)
    std::atomic<bool> playing { false };
};
