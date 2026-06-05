#include "ArrangerSectionSequencer.h"
#include "ArrangerTime.h"
#include <algorithm>
#include <cmath>

void ArrangerSectionSequencer::setStyle (const ArrangerStyle& style)
{
    sections.clear();
    for (const auto& s : style.sections)
        sections.push_back ({ s.id, s.name, s.type, juce::jmax (1, s.lengthBars), s.afterComplete });

    beatsPerBar = ArrangerTime::beatsPerBar (style.timeSigNum, style.timeSigDenom);
    reset();
}

int ArrangerSectionSequencer::firstVariationIndex() const
{
    for (int i = 0; i < (int) sections.size(); ++i)
        if (sections[i].type == ArrangerSectionType::Variation)
            return i;
    return sections.empty() ? -1 : 0;
}

int ArrangerSectionSequencer::findSection (ArrangerSectionType type, const juce::String& name) const
{
    int firstOfType = -1, nameMatch = -1;
    for (int i = 0; i < (int) sections.size(); ++i)
    {
        if (sections[i].type != type) continue;
        if (firstOfType < 0) firstOfType = i;
        if (sections[i].name == name) nameMatch = i;
    }
    return (nameMatch >= 0) ? nameMatch : firstOfType;
}

void ArrangerSectionSequencer::reset()
{
    activeIndex    = juce::jmax (0, firstVariationIndex());
    returnIndex    = activeIndex;
    activeStartAbs = 0.0;
    pendingIndex   = -1;
    stopped        = false;
}

void ArrangerSectionSequencer::startAt (int sectionIndex)
{
    if (sections.empty()) { reset(); return; }
    activeIndex    = juce::jlimit (0, (int) sections.size() - 1, sectionIndex);
    returnIndex    = juce::jmax (0, firstVariationIndex());
    activeStartAbs = 0.0;
    pendingIndex   = -1;
    stopped        = false;
}

bool ArrangerSectionSequencer::queue (ArrangerSectionType type, const juce::String& name)
{
    const int target = findSection (type, name);
    if (target < 0) return false;
    pendingIndex = target;
    return true;
}

void ArrangerSectionSequencer::applyBoundary (double boundaryAbs, SequencerStep& step)
{
    // 1) A queued user switch always wins.
    if (pendingIndex >= 0)
    {
        const int from = activeIndex;
        if (sections[pendingIndex].type == ArrangerSectionType::Fill)
        {
            // Remember the variation/break to return to after the fill.
            if (sections[from].type == ArrangerSectionType::Variation
                || sections[from].type == ArrangerSectionType::Break)
                returnIndex = from;
        }
        activeIndex    = pendingIndex;
        pendingIndex   = -1;
        activeStartAbs = boundaryAbs;
        return;
    }

    // 2) One-shot completion (Intro/Fill/Ending) when the section's own length has elapsed.
    const auto& a = sections[activeIndex];
    if (a.after == ArrangerAfterComplete::Loop)
        return;

    const double sectionLenBeats = (double) a.lengthBars * beatsPerBar;
    if (boundaryAbs - activeStartAbs < sectionLenBeats - 1e-6)
        return; // multi-bar one-shot not finished yet

    if (a.after == ArrangerAfterComplete::Stop)
    {
        stopped = true;
        step.stopRequested = true;
        return;
    }

    // FallThrough: Intro/Fill -> the saved return variation.
    activeIndex    = juce::jlimit (0, (int) sections.size() - 1, returnIndex);
    activeStartAbs = boundaryAbs;
}

SequencerStep ArrangerSectionSequencer::advance (double fromBeats, double toBeats)
{
    SequencerStep step;
    if (sections.empty() || stopped || toBeats <= fromBeats || beatsPerBar <= 0.0)
        return step;

    double pos = fromBeats;
    bool   startChanged = false;

    while (pos < toBeats - 1e-12)
    {
        // Next global bar line strictly after pos.
        double nextBoundary = (std::floor (pos / beatsPerBar + 1e-9) + 1.0) * beatsPerBar;
        if (nextBoundary <= pos + 1e-9)
            nextBoundary += beatsPerBar;

        const double segEnd = std::min (toBeats, nextBoundary);

        SectionSegment seg;
        seg.sectionIndex   = activeIndex;
        seg.localFromBeats = pos - activeStartAbs;
        seg.localToBeats   = segEnd - activeStartAbs;
        seg.sectionChanged = startChanged;
        step.segments.push_back (seg);
        startChanged = false;

        pos = segEnd;

        // Reached a bar boundary inside the window: apply transitions there.
        if (std::abs (segEnd - nextBoundary) < 1e-9 && segEnd <= toBeats + 1e-12)
        {
            const int before = activeIndex;
            applyBoundary (nextBoundary, step);
            if (step.stopRequested)
                break;
            if (activeIndex != before)
                startChanged = true;
        }
    }

    return step;
}
