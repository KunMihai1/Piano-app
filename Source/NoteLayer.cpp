/*
  ==============================================================================

    NoteLayer.cpp
    Created: 20 May 2025 1:36:54pm
    Author:  Kisuke

  ==============================================================================
*/

#include "NoteLayer.h"

NoteLayer::NoteLayer(KeyboardUI& referenceKeyboard) : keyBoardUI{ referenceKeyboard }
{
    setSize(getWidth(), getHeight());
    keyBoardUI.midiHandler.addListener(this);
}

NoteLayer::~NoteLayer()
{
    keyBoardUI.midiHandler.removeListener(this);
}

void NoteLayer::paint(juce::Graphics& g)
{
    for (const auto& [midiNote, note] : activeNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.fillRect(note.bounds);
    }
    for (const auto& note : fallingNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(note.alpha));
        g.fillRect(note.bounds);
    }
}

void NoteLayer::noteOnReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            if (activeNotes.find(midiNote) == activeNotes.end())
            {

                AnimatedNote newNote;
                newNote.bounds = keyBoardUI.keys[midiNote].bounds;
                activeNotes[midiNote] = newNote;
                activeNotes[midiNote].bounds.setHeight(10);
                activeNotes[midiNote].bounds.setY(keyBoardUI.keys[midiNote].bounds.getY() - 12);
            }
        });
}

void NoteLayer::noteOffReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            auto it = activeNotes.find(midiNote);
            if (it != activeNotes.end())
            {
                it->second.isFalling = true;
                fallingNotes.push_back(it->second);
                activeNotes.erase(it);
            }
        });
}

void NoteLayer::visibilityChanged()
{
    if (!isVisible())
        stopTimer();
    else startTimerHz(60);
}

void NoteLayer::timerCallback()
{
    const float dt = 1.0f / 60.0f;
    const float riseSpeed = 300.0f;
    const float fadeSpeed = 150.0f;
    const float fadeTime = 5.0f;
    const float fadeRate = 1.0f / fadeTime;

    for (auto& pair : activeNotes)
    {
        AnimatedNote& n = pair.second;

        int dy = static_cast<int>(round(riseSpeed * dt));
        n.bounds.translate(0, -dy);

        int dh = static_cast<int>(round(riseSpeed * dt));
        int newH = n.bounds.getHeight() + dh;
        n.bounds.setHeight(newH);
    }

    for (auto it = fallingNotes.begin(); it != fallingNotes.end();)
    {
        it->alpha -= dt * fadeRate;
        int dy = static_cast<int>(round(fadeSpeed * dt));
        it->bounds.translate(0, -dy);

        if (it->alpha <= 0.0f)
            it = fallingNotes.erase(it);
        else ++it;
    }
    repaint();
}
