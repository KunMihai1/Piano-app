#pragma once
#include <JuceHeader.h>
#include "ArrangerTimelineComponent.h"
#include "ArrangerStyleFile.h"
#include "ArrangerEngine.h"
#include "TrackEntry.h"
#include <vector>
#include <functional>

/** Authoring screen: turns a recording + timeline edits into a saved .style file,
    with live preview through a shared ArrangerEngine. Hosted as an overlay. */
class ArrangerStyleEditor : public juce::Component,
                            private juce::ScrollBar::Listener
{
public:
    /** Fired when the user dismisses the editor (Close button). */
    std::function<void()> onClose;
    /** Fired after a successful save, with the written file AND the freshly-built engine style, so the
        host can refresh the active config WITHOUT re-parsing the file (which would re-freeze the UI). */
    std::function<void (const juce::File&, const ArrangerStyle&)> onSaved;
    /** Supplies the app's current recording as raw track entries for the "Update Tracks" action. Set
        by the host; if unset, Update Tracks is a no-op. Returned raw so the heavy event-building can
        happen on the editor's background thread. */
    std::function<std::vector<TrackEntry>()> onRequestCurrentTracks;

    explicit ArrangerStyleEditor (ArrangerEngine& engineToPreviewWith);

    /** Seed the editor from the current recorded tracks at the given playback tempo. */
    void loadRecording (const std::vector<TrackEntry>& tracks, double referenceBpm,
                        int timeSigNum, int timeSigDenom, const juce::String& styleName);

    /** Seed the editor from an already-saved style file (for editing). */
    void loadFromFile (const ArrangerStyleFile& f);

    /** Remember the on-disk file this editor was opened from, so Save renames it (rather
        than leaving a duplicate) when the name changes. Leave unset for a brand-new config. */
    void setSourceFile (const juce::File& f) { loadedFile = f; updateTracksBtn.setEnabled (f.existsAsFile()); }

    /** Build the current ArrangerStyleFile from editor state (source tracks + windows). */
    ArrangerStyleFile toStyleFile() const;
    /** Build the engine-ready style from current editor state (same build rebuildPreview uses). */
    ArrangerStyle buildStyle() const;

    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    void requestSave();             // validate name, confirm on collision, then finishSave
    void finishSave (const juce::File& target);   // serialize + write off-thread, then refresh on the message thread
    void setBusy (bool busy, const juce::String& text = {});  // show/hide the in-editor "working" overlay + gate buttons
    void rebuildPreview();          // build style from current state and engine.setStyle
    void addSectionOfType (ArrangerSectionType type);
    void updateTracksFromRecording();  // replace source tracks with the app's current recording, keep sections, overwrite file
    void removeSelectedSection();   // remove the section currently selected on the timeline
    void renumberSectionsByType();  // assign unique per-type names (Intro 1, Intro 2, ...)
    void recomputeTotalBars();      // longest source track / furthest window, in whole bars
    void layoutTimeline();          // size the (scrollable) timeline to totalBars * kPixelsPerBar
    void scrollBarMoved (juce::ScrollBar*, double newRangeStart) override;  // detect manual scroll

    ArrangerEngine& engine;
    juce::Viewport  timelineViewport;          // horizontal scroll for many bars
    ArrangerTimelineComponent timeline;

    juce::Label      nameLabel { {}, "Name:" };
    juce::TextEditor nameEditor;
    juce::TextButton addIntroBtn { "Add Intro" }, addVariationBtn { "Add Var" }, addFillBtn { "Add Fill" },
                     addBreakBtn { "Add Break" }, addEndingBtn { "Add Ending" }, removeBtn { "Remove" },
                     previewBtn { "Preview" }, stopBtn { "Stop" }, updateTracksBtn { "Update Tracks" },
                     saveBtn { "Save" }, closeBtn { "Close" };

    juce::Label busyOverlay;   // full-screen dim + centered label shown while save/update runs off-thread

    std::vector<SourceTrackFile> sourceTracks;
    std::vector<SectionWindow>   windows;
    juce::File   loadedFile;        // file this editor was opened from ({} for a new config)
    juce::String name;
    double referenceBpm = 120.0;
    int    timeSigNum = 4, timeSigDenom = 4;
    int    totalBars  = 1;
    juce::String styleId = juce::Uuid().toString();
    bool   followPlayhead     = true;   // auto-scroll the viewport to keep the playhead in view
    bool   programmaticScroll = false;  // true while WE move the viewport (so it isn't seen as manual)
};
