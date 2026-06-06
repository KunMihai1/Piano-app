#include "ArrangerStyleEditor.h"
#include "ArrangerSourceBuilder.h"
#include "ArrangerDefaults.h"
#include "ArrangerPatternBuilder.h"
#include "ArrangerStyleIOHelper.h"
#include "ArrangerEnums.h"
#include "ArrangerTime.h"
#include "IOHelper.h"

ArrangerStyleEditor::ArrangerStyleEditor (ArrangerEngine& e) : engine (e)
{
    timelineViewport.setViewedComponent (&timeline, false);
    timelineViewport.setScrollBarsShown (false, true);   // horizontal only
    timelineViewport.setScrollBarThickness (12);
    addAndMakeVisible (timelineViewport);
    addAndMakeVisible (nameLabel);
    addAndMakeVisible (nameEditor);
    nameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    nameEditor.setText ("New configuration", juce::dontSendNotification);
    for (auto* b : { &addIntroBtn, &addFillBtn, &addEndingBtn, &previewBtn, &stopBtn, &saveBtn, &closeBtn })
        addAndMakeVisible (b);

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

    addIntroBtn.onClick  = [this] { addSectionOfType (ArrangerSectionType::Intro); };
    addFillBtn.onClick   = [this] { addSectionOfType (ArrangerSectionType::Fill); };
    addEndingBtn.onClick = [this] { addSectionOfType (ArrangerSectionType::Ending); };
    previewBtn.onClick   = [this] { rebuildPreview(); engine.setBpm (referenceBpm); engine.start(); };
    stopBtn.onClick      = [this] { engine.stop(); };
    saveBtn.onClick      = [this] { requestSave(); };
    closeBtn.onClick = [this] { engine.stop(); if (onClose) onClose(); };

    engine.onElapsedBeats = [this] (double beats)   // always marshalled to the message thread
    {
        const double bpb = ArrangerTime::beatsPerBar (timeSigNum, timeSigDenom);
        const double bar = 1.0 + beats / juce::jmax (1.0, bpb);
        timeline.setPlayheadBar (bar);

        // Auto-follow: keep the playhead in view, scrolling the viewport when it nears an edge.
        const int px    = timeline.xForBar (bar);
        const int viewX = timelineViewport.getViewPositionX();
        const int vw    = timelineViewport.getViewWidth();
        if (vw > 0 && (px < viewX + 48 || px > viewX + vw - 48))
        {
            const int maxX = juce::jmax (0, timeline.getWidth() - vw);
            timelineViewport.setViewPosition (juce::jlimit (0, maxX, px - vw / 2), 0);
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
    w.startBar = 1;
    w.lengthBars = juce::jmin (type == ArrangerSectionType::Fill ? 1 : 2, totalBars);
    w.afterComplete = (type == ArrangerSectionType::Ending) ? ArrangerAfterComplete::Stop
                                                            : ArrangerAfterComplete::FallThrough;
    w.name = ArrangerEnums::toString (type) + " 1";
    windows.push_back (w);
    timeline.setWindows (windows);
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
    for (auto* b : { &addIntroBtn, &addFillBtn, &addEndingBtn, &previewBtn, &stopBtn, &saveBtn, &closeBtn })
        b->setBounds (top.removeFromLeft (84).reduced (2));

    area.removeFromTop (6);
    timelineViewport.setBounds (area);
    layoutTimeline();
}

void ArrangerStyleEditor::layoutTimeline()
{
    const int h = juce::jmax (40, timelineViewport.getHeight() - 14); // leave room for the scrollbar
    // Fill the viewport when there are few bars; grow + scroll when there are many.
    const int w = juce::jmax (timeline.getPreferredWidth(), timelineViewport.getWidth());
    timeline.setSize (w, h);
}
