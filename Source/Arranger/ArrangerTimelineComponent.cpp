#include "ArrangerTimelineComponent.h"
#include "ArrangerEnums.h"
#include <cmath>

using namespace ArrangerTimelineGeometry;

Layout ArrangerTimelineComponent::layout() const
{
    return Layout { juce::jmax (1, totalBars), getWidth(), marginPx };
}

int ArrangerTimelineComponent::getPreferredWidth() const
{
    return marginPx * 2 + juce::jmax (1, totalBars) * kPixelsPerBar;
}

int ArrangerTimelineComponent::xForBar (double bar) const
{
    return (int) std::lround (barToX (bar, layout()));
}

juce::Colour ArrangerTimelineComponent::colourForType (ArrangerSectionType t) const
{
    switch (t)
    {
        case ArrangerSectionType::Intro:     return juce::Colour (0xff4fc1ff);
        case ArrangerSectionType::Variation: return juce::Colour (0xff6a9955);
        case ArrangerSectionType::Fill:      return juce::Colour (0xffdcdcaa);
        case ArrangerSectionType::Break:     return juce::Colour (0xffc586c0);
        case ArrangerSectionType::Ending:    return juce::Colour (0xffd16969);
        case ArrangerSectionType::CountIn:   return juce::Colour (0xff808080);
    }
    return juce::Colours::grey;
}

void ArrangerTimelineComponent::setTotalBars (int bars)     { totalBars = juce::jmax (1, bars); repaint(); }
void ArrangerTimelineComponent::setWindows (const std::vector<SectionWindow>& w)
{
    windows = w;
    if (selectedIndex >= (int) windows.size())   // a removed/shrunk list can leave a stale selection
        selectedIndex = -1;
    repaint();
}
void ArrangerTimelineComponent::setPlayheadBar (double bar) { playheadBar = bar; repaint(); }

juce::String ArrangerTimelineComponent::getTooltip()
{
    const auto pos = getMouseXYRelative();
    const auto L = layout();
    const int regionTop = 18, regionH = getHeight() - 26;
    for (int i = (int) windows.size() - 1; i >= 0; --i)   // topmost (last drawn) first
    {
        auto r = regionBounds (windows[i].startBar, windows[i].lengthBars, L, regionH).withY (regionTop);
        if (r.contains (pos))
        {
            const int endBar = windows[i].startBar + windows[i].lengthBars - 1;
            return windows[i].name + "  (bars " + juce::String (windows[i].startBar)
                 + "-" + juce::String (endBar) + ")";
        }
    }
    return {};
}

void ArrangerTimelineComponent::paint (juce::Graphics& g)
{
    const auto L = layout();
    const double ppb = pixelsPerBar (L);
    const auto clip = g.getClipBounds();   // only paint what's actually visible (cheap when scrolled)
    g.fillAll (juce::Colour (0xff1e1e1e));

    // bar grid + numbers — restricted to the visible bar range
    int firstBar = (int) std::floor (((double) clip.getX()     - marginPx) / ppb) + 1;
    int lastBar  = (int) std::ceil  (((double) clip.getRight() - marginPx) / ppb) + 1;
    firstBar = juce::jlimit (1, totalBars + 1, firstBar);
    lastBar  = juce::jlimit (1, totalBars + 1, lastBar);

    for (int bar = firstBar; bar <= lastBar; ++bar)
    {
        const int x = (int) std::lround (barToX ((double) bar, L));
        g.setColour (juce::Colour (0xff3a3a3a));
        g.drawVerticalLine (x, 0.0f, (float) getHeight());
        if (bar <= totalBars)
        {
            g.setColour (juce::Colour (0xff777777));
            g.drawText (juce::String (bar), x + 3, 0, kPixelsPerBar - 4, 14, juce::Justification::left);
        }
    }

    // section regions (skip those outside the visible clip)
    const int regionTop = 18, regionH = getHeight() - 26;
    for (int i = 0; i < (int) windows.size(); ++i)
    {
        auto r = regionBounds (windows[i].startBar, windows[i].lengthBars, L, regionH)
                     .withY (regionTop);
        if (r.getRight() < clip.getX() || r.getX() > clip.getRight())
            continue;
        auto c = colourForType (windows[i].type);
        g.setColour (c.withAlpha (i == selectedIndex ? 0.85f : 0.55f));
        g.fillRoundedRectangle (r.toFloat(), 4.0f);
        g.setColour (i == selectedIndex ? juce::Colours::white : c);
        g.drawRoundedRectangle (r.toFloat(), 4.0f, i == selectedIndex ? 2.0f : 1.0f);
        g.setColour (juce::Colours::white);
        g.drawText (windows[i].name, r.reduced (6, 2), juce::Justification::centredLeft, true);
    }

    // playhead
    const int px = (int) std::lround (barToX (playheadBar, L));
    g.setColour (juce::Colours::orange);
    g.drawVerticalLine (px, (float) regionTop, (float) (regionTop + regionH));
}

void ArrangerTimelineComponent::mouseDown (const juce::MouseEvent& e)
{
    const auto L = layout();
    const int regionTop = 18, regionH = getHeight() - 26;
    selectedIndex = -1;
    dragMode = Hit::None;
    windowsChangedByDrag = false;   // a bare click (no drag) must NOT rebuild the style

    for (int i = (int) windows.size() - 1; i >= 0; --i)
    {
        auto r = regionBounds (windows[i].startBar, windows[i].lengthBars, L, regionH).withY (regionTop);
        if (e.y >= r.getY() && e.y <= r.getBottom())
        {
            const auto hit = ArrangerTimelineGeometry::hitTest (e.x, r);
            if (hit != Hit::None)
            {
                selectedIndex      = i;
                dragMode           = hit;
                dragStartLenValue  = windows[i].lengthBars;
                dragGrabOffsetBars = ArrangerTimelineGeometry::xToSnappedBar ((double) e.x, L) - windows[i].startBar;
                juce::Desktop::getInstance().beginDragAutoRepeat (40); // keep firing mouseDrag while held at an edge
                break;
            }
        }
    }

    if (onSectionSelected) onSectionSelected (selectedIndex);
    repaint();
}

void ArrangerTimelineComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (selectedIndex < 0 || dragMode == Hit::None) return;
    const auto L = layout();
    auto& w = windows[selectedIndex];

    const int oldStart = w.startBar, oldLen = w.lengthBars;   // detect a *real* edit (not jitter)
    const int barUnderCursor = xToSnappedBar ((double) e.x, L);

    if (dragMode == Hit::Body)
    {
        // Absolute positioning via the grab offset stays correct while the view auto-scrolls.
        w.startBar = juce::jlimit (1, totalBars + 1 - dragStartLenValue, barUnderCursor - dragGrabOffsetBars);
    }
    else if (dragMode == Hit::LeftEdge)
    {
        const int newStart = juce::jlimit (1, w.startBar + w.lengthBars - 1, barUnderCursor);
        w.lengthBars = (w.startBar + w.lengthBars) - newStart;
        w.startBar   = newStart;
    }
    else if (dragMode == Hit::RightEdge)
    {
        const int newEnd = juce::jlimit (w.startBar + 1, totalBars + 1, barUnderCursor);
        w.lengthBars = newEnd - w.startBar;
    }

    // Only count this gesture as a window edit if a bar value actually moved. Sub-bar mouse jitter
    // during a plain click leaves the snapped values unchanged -> stays a "click to jump", not an edit
    // (an edit triggers rebuildPreview, which resets the sequencer and would drop a queued switch).
    if (w.startBar != oldStart || w.lengthBars != oldLen)
        windowsChangedByDrag = true;

    // Drag near a viewport edge -> scroll horizontally to follow.
    if (auto* vp = findParentComponentOfClass<juce::Viewport>())
    {
        const auto p = vp->getLocalPoint (this, e.getPosition());
        vp->autoScroll (p.x, p.y, 30, 30);
    }

    repaint();
}

void ArrangerTimelineComponent::mouseUp (const juce::MouseEvent&)
{
    juce::Desktop::getInstance().beginDragAutoRepeat (0);   // stop the auto-scroll repeat
    // Only rebuild the style if a real drag edited the windows. A bare click is a "jump to section"
    // gesture (it queues a switch); rebuilding here would reset the sequencer and drop that switch.
    if (windowsChangedByDrag && onWindowsChanged)
        onWindowsChanged (windows);
    dragMode = Hit::None;
    windowsChangedByDrag = false;
}
