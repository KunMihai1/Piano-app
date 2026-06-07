#include "ArrangerStyleEditor.h"
#include "ArrangerSourceBuilder.h"
#include "ArrangerDefaults.h"
#include "ArrangerPatternBuilder.h"
#include "ArrangerStyleIOHelper.h"
#include "ArrangerEnums.h"
#include "ArrangerTime.h"
#include "IOHelper.h"
#include <cmath>

ArrangerStyleEditor::ArrangerStyleEditor (ArrangerEngine& e) : engine (e)
{
    timelineViewport.setViewedComponent (&timeline, false);
    timelineViewport.setScrollBarsShown (false, true);   // horizontal only
    timelineViewport.setScrollBarThickness (12);
    timelineViewport.getHorizontalScrollBar().addListener (this);  // detect manual scroll
    addAndMakeVisible (timelineViewport);
    addAndMakeVisible (nameLabel);
    addAndMakeVisible (nameEditor);
    nameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    nameEditor.setText ("New configuration", juce::dontSendNotification);
    for (auto* b : { &addIntroBtn, &addVariationBtn, &addFillBtn, &addBreakBtn, &addEndingBtn,
                     &removeBtn, &previewBtn, &stopBtn, &updateTracksBtn, &saveBtn, &closeBtn })
        addAndMakeVisible (b);

    updateTracksBtn.setEnabled (false);   // only meaningful when editing an existing saved config

    timeline.onWindowsChanged = [this] (const std::vector<SectionWindow>& w)
    {
        windows = w;
        rebuildPreview();
    };

    // Clicking a region while previewing switches to that section at the next bar line.
    timeline.onSectionSelected = [this] (int idx)
    {
        if (engine.isPlaying() && idx >= 0 && idx < (int) windows.size())
            engine.queueSection (windows[(size_t) idx].type, windows[(size_t) idx].name);
    };

    addIntroBtn.onClick     = [this] { addSectionOfType (ArrangerSectionType::Intro); };
    addVariationBtn.onClick = [this] { addSectionOfType (ArrangerSectionType::Variation); };
    addFillBtn.onClick      = [this] { addSectionOfType (ArrangerSectionType::Fill); };
    addBreakBtn.onClick     = [this] { addSectionOfType (ArrangerSectionType::Break); };
    addEndingBtn.onClick    = [this] { addSectionOfType (ArrangerSectionType::Ending); };
    removeBtn.onClick       = [this] { removeSelectedSection(); };
    previewBtn.onClick   = [this] { followPlayhead = true; rebuildPreview(); engine.setBpm (referenceBpm); engine.start(); };
    stopBtn.onClick      = [this] { engine.stop(); };
    updateTracksBtn.onClick = [this] { updateTracksFromRecording(); };
    saveBtn.onClick      = [this] { requestSave(); };
    closeBtn.onClick = [this] { engine.stop(); if (onClose) onClose(); };

    engine.onElapsedBeats = [this] (double /*globalBeats*/)   // always marshalled to the message thread
    {
        const double bpb = ArrangerTime::beatsPerBar (timeSigNum, timeSigDenom);

        // Show the arrow at the *active section's* position, not the global monotonic playhead, so a
        // jump (e.g. to the Ending) moves the arrow into that section's bars. A looping section wraps
        // its local beats within its own length so the arrow loops inside its window.
        const int    secIdx     = engine.getActiveSectionIndex();
        const double localBeats = juce::jmax (0.0, engine.getActiveSectionLocalBeats());
        double bar;
        if (secIdx >= 0 && secIdx < (int) windows.size())
        {
            const auto&  w          = windows[(size_t) secIdx];
            const double secLenBeats = (double) juce::jmax (1, w.lengthBars) * bpb;
            const double wrapped     = secLenBeats > 0.0 ? std::fmod (localBeats, secLenBeats) : 0.0;
            bar = (double) w.startBar + wrapped / juce::jmax (1.0, bpb);
        }
        else
        {
            bar = 1.0 + localBeats / juce::jmax (1.0, bpb);
        }
        timeline.setPlayheadBar (bar);

        // Auto-follow: keep the playhead in view, scrolling the viewport when it nears an edge.
        // Suspended once the user scrolls manually (so they can navigate to a far section); press
        // Preview to re-arm it.
        const int px    = timeline.xForBar (bar);
        const int viewX = timelineViewport.getViewPositionX();
        const int vw    = timelineViewport.getViewWidth();
        if (followPlayhead && vw > 0 && (px < viewX + 48 || px > viewX + vw - 48))
        {
            const int maxX = juce::jmax (0, timeline.getWidth() - vw);
            programmaticScroll = true;   // our own scroll: don't treat it as a manual one
            timelineViewport.setViewPosition (juce::jlimit (0, maxX, px - vw / 2), 0);
            programmaticScroll = false;
        }
    };
}

void ArrangerStyleEditor::recomputeTotalBars()
{
    const double bpb = ArrangerTime::beatsPerBar (timeSigNum, timeSigDenom);
    double lastBeat = 0.0;
    for (const auto& st : sourceTracks)
        if (! st.events.empty())
            lastBeat = juce::jmax (lastBeat, st.events.back().beats);

    int bars = juce::jmax (1, ArrangerTime::barsForBeats (lastBeat, bpb));
    for (const auto& w : windows)
        bars = juce::jmax (bars, w.startBar + w.lengthBars - 1);
    totalBars = juce::jmax (1, bars);
}

void ArrangerStyleEditor::loadRecording (const std::vector<TrackEntry>& tracks, double refBpm,
                                         int tsNum, int tsDenom, const juce::String& styleName)
{
    referenceBpm = (refBpm > 0.0 ? refBpm : 120.0);
    timeSigNum = tsNum; timeSigDenom = tsDenom;
    name = styleName.isNotEmpty() ? styleName : "New configuration";
    nameEditor.setText (name, juce::dontSendNotification);
    styleId = juce::Uuid().toString();

    updateTracksBtn.setEnabled (false);   // brand-new config: nothing on disk to update yet

    sourceTracks = ArrangerSourceBuilder::fromTrackEntries (tracks, referenceBpm);
    windows = ArrangerDefaults::defaultWindowsForBars (1);
    recomputeTotalBars();
    windows = ArrangerDefaults::defaultWindowsForBars (totalBars);

    timeline.setTotalBars (totalBars);
    timeline.setWindows (windows);
    layoutTimeline();
    rebuildPreview();
}

void ArrangerStyleEditor::loadFromFile (const ArrangerStyleFile& f)
{
    styleId = f.id.isNotEmpty() ? f.id : juce::Uuid().toString();
    name = f.name.isNotEmpty() ? f.name : "Style";
    nameEditor.setText (name, juce::dontSendNotification);
    referenceBpm = (f.originalTempo > 0.0 ? f.originalTempo : 120.0);
    timeSigNum = f.timeSigNum; timeSigDenom = f.timeSigDenom;
    sourceTracks = f.sourceTracks;
    windows = f.sections;
    if (windows.empty())
        windows = ArrangerDefaults::defaultWindowsForBars (1);
    renumberSectionsByType();   // ensure unique per-type names (older files had them all as "<type> 1")

    recomputeTotalBars();
    timeline.setTotalBars (totalBars);
    timeline.setWindows (windows);
    layoutTimeline();
    rebuildPreview();
}

ArrangerStyleFile ArrangerStyleEditor::toStyleFile() const
{
    ArrangerStyleFile f;
    f.id = styleId; f.name = name;
    f.originalTempo = referenceBpm;
    f.timeSigNum = timeSigNum; f.timeSigDenom = timeSigDenom;
    f.sourceTracks = sourceTracks;
    f.sections = windows;
    return f;
}

void ArrangerStyleEditor::rebuildPreview()
{
    auto style = ArrangerPatternBuilder::buildStyleFromWindows (
        sourceTracks, windows, timeSigNum, timeSigDenom, referenceBpm);
    engine.setStyle (style);
}

void ArrangerStyleEditor::addSectionOfType (ArrangerSectionType type)
{
    SectionWindow w;
    w.id = juce::Uuid().toString();
    w.type = type;
    // Drop the new section where the user is currently looking (the bar at the timeline viewport's
    // left edge), not back at bar 1. Clamp to the valid range, and trim its length so it stays in bounds.
    const int viewBar = timeline.barForX (timelineViewport.getViewPositionX());
    w.startBar = juce::jlimit (1, juce::jmax (1, totalBars), viewBar);
    const int desiredLen = (type == ArrangerSectionType::Fill ? 1 : 2);
    w.lengthBars = juce::jmin (desiredLen, juce::jmax (1, totalBars - w.startBar + 1));
    w.afterComplete = (type == ArrangerSectionType::Ending) ? ArrangerAfterComplete::Stop
                                                            : ArrangerAfterComplete::FallThrough;

    // Unique, stable per-type name (max existing + 1) so the section is identifiable and the engine
    // can jump to this exact one (it resolves by type + name). Stable across removals (gaps are fine).
    int maxN = 0;
    for (const auto& existing : windows)
        if (existing.type == type)
            maxN = juce::jmax (maxN, existing.name.fromLastOccurrenceOf (" ", false, false).getIntValue());
    w.name = ArrangerEnums::toString (type) + " " + juce::String (maxN + 1);

    windows.push_back (w);
    timeline.setWindows (windows);
    rebuildPreview();
}

void ArrangerStyleEditor::updateTracksFromRecording()
{
    // Replace the configuration's recorded notes with the app's current recording while keeping the
    // sections (Intro/Variation/Fill/...) exactly as they are, then overwrite the saved .style.
    if (! loadedFile.existsAsFile())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
            "Update tracks", "Save this configuration first, then Update Tracks can replace its notes.");
        return;
    }
    if (! onRequestCurrentTracks)
        return;

    juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon,
        "Update tracks",
        "Replace the tracks of \"" + name + "\" with the current recording and overwrite the saved "
        "configuration?\nYour sections (intros/variations/fills) stay as they are.",
        "Update", "Cancel", this,
        juce::ModalCallbackFunction::create ([this] (int result)
        {
            if (result != 1)   // 1 = Update, 0 = Cancel
                return;

            auto newTracks = onRequestCurrentTracks (referenceBpm);
            if (newTracks.empty())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
                    "Update tracks", "No recorded tracks to import.");
                return;
            }

            sourceTracks = std::move (newTracks);   // sections (windows) are intentionally untouched
            recomputeTotalBars();
            timeline.setTotalBars (totalBars);
            timeline.setWindows (windows);
            layoutTimeline();
            rebuildPreview();

            finishSave (loadedFile);   // overwrite the same file: keeps name + style id
        }));
}

void ArrangerStyleEditor::renumberSectionsByType()
{
    // Give every section a unique per-type number in list order (fixes legacy files where they were
    // all "<type> 1" and so couldn't be told apart or individually targeted).
    for (size_t i = 0; i < windows.size(); ++i)
    {
        int n = 0;
        for (size_t j = 0; j <= i; ++j)
            if (windows[j].type == windows[i].type)
                ++n;
        windows[i].name = ArrangerEnums::toString (windows[i].type) + " " + juce::String (n);
    }
}

void ArrangerStyleEditor::removeSelectedSection()
{
    const int idx = timeline.getSelectedIndex();
    if (idx < 0 || idx >= (int) windows.size())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
            "Remove section", "Click a section on the timeline to select it first.");
        return;
    }

    windows.erase (windows.begin() + idx);
    recomputeTotalBars();
    timeline.setTotalBars (totalBars);
    timeline.setWindows (windows);   // also clears the now-stale selection
    layoutTimeline();
    rebuildPreview();
}

void ArrangerStyleEditor::requestSave()
{
    name = nameEditor.getText().trim();
    if (name.isEmpty()) name = "New configuration";

    auto folder = IOHelper::getArrangerStylesFolder();
    auto target = folder.getChildFile (juce::File::createLegalFileName (name) + ".style");

    // A *different* existing config would be clobbered: confirm before overwriting it.
    if (target.existsAsFile() && target != loadedFile)
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon,
            "Save configuration",
            "A configuration named \"" + target.getFileNameWithoutExtension()
                + "\" already exists.\nOverwrite it?",
            "Overwrite", "Cancel", this,
            juce::ModalCallbackFunction::create ([this, target] (int result)
            {
                if (result == 1)            // 1 = Overwrite, 0 = Cancel
                    finishSave (target);
            }));
        return;
    }

    finishSave (target);
}

void ArrangerStyleEditor::finishSave (const juce::File& target)
{
    ArrangerStyleIOHelper::saveToFile (target, toStyleFile());
    if (! target.existsAsFile())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
            "Save configuration", "Failed to write " + target.getFullPathName());
        return;
    }

    // Rename: if opened from a different file, remove the old one so we don't leave a duplicate.
    const bool renamed = loadedFile.existsAsFile() && loadedFile != target;
    if (renamed)
        loadedFile.deleteFile();

    loadedFile = target;

    if (onSaved) onSaved (target);
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
        "Save configuration", (renamed ? "Renamed to: " : "Saved: ") + target.getFileName());
}

void ArrangerStyleEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff121212));
    g.setColour (juce::Colour (0xff2a2a2a));
    g.drawRect (getLocalBounds(), 2);
}

void ArrangerStyleEditor::resized()
{
    auto area = getLocalBounds().reduced (8);

    auto top = area.removeFromTop (30);
    nameLabel.setBounds (top.removeFromLeft (50));
    nameEditor.setBounds (top.removeFromLeft (180).reduced (0, 2));
    top.removeFromLeft (8);
    for (auto* b : { &addIntroBtn, &addVariationBtn, &addFillBtn, &addBreakBtn, &addEndingBtn,
                     &removeBtn, &previewBtn, &stopBtn, &updateTracksBtn, &saveBtn, &closeBtn })
        b->setBounds (top.removeFromLeft (84).reduced (2));

    area.removeFromTop (6);
    timelineViewport.setBounds (area);
    layoutTimeline();
}

void ArrangerStyleEditor::scrollBarMoved (juce::ScrollBar*, double)
{
    // A scroll the user initiated (not our auto-follow) suspends following so they can navigate
    // freely to a far section. Pressing Preview re-arms it.
    if (! programmaticScroll)
        followPlayhead = false;
}

void ArrangerStyleEditor::layoutTimeline()
{
    const int h = juce::jmax (40, timelineViewport.getHeight() - 14); // leave room for the scrollbar
    // Fill the viewport when there are few bars; grow + scroll when there are many.
    const int w = juce::jmax (timeline.getPreferredWidth(), timelineViewport.getWidth());
    timeline.setSize (w, h);
}
