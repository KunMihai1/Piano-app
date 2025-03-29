/*
  ==============================================================================

    keyboardUI.cpp
    Created: 26 Mar 2025 9:45:11pm
    Author:  Kisuke

  ==============================================================================
*/

#include "keyboardUI.h"

KeyboardUI::KeyboardUI(MidiHandler& midiHandler) : midiHandler{ midiHandler }
{
    setSize(getWidth(), getHeight());
    midiHandler.addListener(this);
}

KeyboardUI::~KeyboardUI()
{
    midiHandler.removeListener(this);
    stopTimer();
}

void KeyboardUI::paint(juce::Graphics& g)
{
    float whiteKeyWidth = static_cast<float>(getWidth() / 52) + 0.92f;
    float keyHeight = static_cast<float>(getHeight());
    juce::Array<int> whiteKeyPositions;
    int noteAccount = 0;
    int refresh = 0;

    for (int i = 0; i < 2; i++) // A0 and B0
    {
        float x = i * whiteKeyWidth;
        whiteKeyPositions.add(static_cast<int>(x));

        if (21 + noteAccount >= min_draw && 21 + noteAccount <= max_draw) {
            if (isDrawn && keys[21 + noteAccount].isActive == true)
                g.setColour(juce::Colours::green);
            else g.setColour(juce::Colours::white);
        }
        else {
            g.setColour(juce::Colours::white.withAlpha(0.5f));
        }

        juce::Rectangle<int> keyRectangle(x, 0, whiteKeyWidth, keyHeight);
        g.fillRect(keyRectangle);
        g.setColour(juce::Colours::black);
        g.drawRect(static_cast<int>(x), 0, static_cast<int>(whiteKeyWidth), static_cast<int>(keyHeight));

        keys[21 + noteAccount].bounds = keyRectangle;
        if (!isDrawn)
            keys[21 + noteAccount].isActive = false;
        keys[21 + noteAccount].type = "white";

        if ((noteAccount == 2 && refresh == 1)) {
            noteAccount--;
            refresh = 0;
        }
        else {
            refresh++;
        }

        noteAccount += 2;
    }

    float blackKeyWidth = whiteKeyWidth * 0.6f;
    float blackKeyHeight = keyHeight * 0.6f;
    float firstBlackKeyX = whiteKeyPositions[0] + whiteKeyWidth - blackKeyWidth * 0.5f; // A#0 position

    if (22 >= min_draw && 22 <= max_draw) {
        if (isDrawn && keys[22].isActive == true)
            g.setColour(juce::Colours::green);
        else  g.setColour(juce::Colours::black);
    }
    else
        g.setColour(juce::Colours::black.withAlpha(0.5f));

    g.fillRect(static_cast<int>(firstBlackKeyX), 0, static_cast<int>(blackKeyWidth), static_cast<int>(blackKeyHeight));
    keys[22].bounds = juce::Rectangle<int>(firstBlackKeyX, 0, blackKeyWidth, blackKeyHeight);
    if (!isDrawn)
        keys[22].isActive = false;
    keys[22].type = "black";

    for (int i = 2; i < 52; i++)
    {
        float x = i * whiteKeyWidth;
        whiteKeyPositions.add(static_cast<int>(x));

        if (21 + noteAccount >= min_draw && 21 + noteAccount <= max_draw) {
            if (isDrawn && keys[21 + noteAccount].isActive == true)
                g.setColour(juce::Colours::green);
            else g.setColour(juce::Colours::white);
        }
        else {
            g.setColour(juce::Colours::white.withAlpha(0.5f));
        }

        juce::Rectangle<int> keyRectangle(x, 0, whiteKeyWidth, keyHeight);
        g.fillRect(keyRectangle);
        g.setColour(juce::Colours::black);
        g.drawRect(keyRectangle);

        keys[21 + noteAccount].bounds = juce::Rectangle<int>(x, 0, whiteKeyWidth, keyHeight);
        if (!isDrawn)
            keys[21 + noteAccount].isActive = false;
        keys[21 + noteAccount].type = "white";

        if ((noteAccount == 7 && refresh == 2) || (refresh == 3) || (refresh == 2 && noteAccount % 2 == 1)) {
            noteAccount--;
            refresh = 0;
        }
        else {
            refresh++;
        }

        noteAccount += 2;
    }

    const float blackOffsets[5] = { 0.7f, 1.7f, 3.2f, 4.2f, 5.2f };
    int octaveCount = 52 / 7;

    noteAccount = 4;
    refresh = 0;

    for (int octave = 0; octave < octaveCount; octave++)
    {
        int octaveStartIndex = 2 + octave * 7;
        for (int j = 0; j < 5; j++)
        {
            int whiteIndex = octaveStartIndex + static_cast<int>(blackOffsets[j]);
            if (whiteIndex < whiteKeyPositions.size() - 1) {
                float x = whiteKeyPositions[whiteIndex] + whiteKeyWidth - (blackKeyWidth * 0.5f);

                if (21 + noteAccount >= min_draw && 21 + noteAccount <= max_draw) {
                    if (isDrawn && keys[21 + noteAccount].isActive == true)
                        g.setColour(juce::Colours::green);
                    else g.setColour(juce::Colours::black);
                }
                else
                    g.setColour(juce::Colours::black.withAlpha(0.5f));

                g.fillRect(static_cast<int>(x), 0, static_cast<int>(blackKeyWidth), static_cast<int>(blackKeyHeight));
                keys[21 + noteAccount].bounds = juce::Rectangle<int>(x, 0, blackKeyWidth, blackKeyHeight);
                if (!isDrawn)
                    keys[21 + noteAccount].isActive = false;
                keys[21 + noteAccount].type = "black";
            }

            if ((refresh == 1 && noteAccount % 2 == 0) || (refresh == 3) || (refresh == 2 && noteAccount % 2 == 1)) {
                noteAccount++;
                refresh = 0;
            }
            else {
                refresh++;
            }

            noteAccount += 2;
        }
    }

    if (!isDrawn)
        isDrawn = true;
    else {
        for (const auto& [key, MidiNote] : keys)
        {
            if (MidiNote.isActive) 
            {
                g.setColour(juce::Colours::green);
            }
            else 
            {
                if (MidiNote.type == "white")
                {
                    if (key<min_draw || key>max_draw)
                        g.setColour(juce::Colours::white.withAlpha(0.5f));
                    else g.setColour(juce::Colours::white);
                }
                else {
                    if (key<min_draw || key>max_draw)
                        g.setColour(juce::Colours::black.withAlpha(0.5f));
                    else g.setColour(juce::Colours::black);
                }
            }
            g.fillRect(MidiNote.bounds);
            g.setColour(juce::Colours::black);
            g.drawRect(MidiNote.bounds);
        }
    }
    for (const auto& [midiNote, note] : activeNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.drawRect(note.bounds);
    }
    for (const auto& note : fallingNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(note.alpha));
        g.drawRect(note.bounds);
    }
}

    

void KeyboardUI::set_min_and_max(const int min, const int max)
{
    this->min_draw = min;
    this->max_draw = max;
}

void KeyboardUI::noteOnReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            if (activeNotes.find(midiNote) == activeNotes.end())
            {
                animatedNote newNote;
                newNote.bounds = keys[midiNote].bounds;
                activeNotes[midiNote] = newNote;
            }
            activeNotes[midiNote].bounds.setHeight(10);
            activeNotes[midiNote].bounds.setY(keys[midiNote].bounds.getY()-50);

            keys[midiNote].isActive = true;
            repaint(keys[midiNote].bounds);
        });
}

void KeyboardUI::noteOffReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            auto it = activeNotes.find(midiNote);
            if (it!= activeNotes.end())
            {
                it->second.isFalling = true;
                fallingNotes.push_back(it->second);
                activeNotes.erase(it);
            }
            keys[midiNote].isActive = false;
            repaint(keys[midiNote].bounds);
        });
}

void KeyboardUI::visibilityChanged()
{
    //if (!isVisible())
    //   stopTimer();
    //else startTimerHz(2);
}

void KeyboardUI::timerCallback()
{
    bool needsRepaint = false;
    for (auto it = activeNotes.begin(); it != activeNotes.end();)
    {
        juce::Rectangle<int> oldBoundsActive = it->second.bounds;
        oldBoundsActive.setHeight(oldBoundsActive.getHeight() +2);
        oldBoundsActive.setY(oldBoundsActive.getY() -2);
        repaint(oldBoundsActive);
    }
    for (auto it = fallingNotes.begin(); it != fallingNotes.end();)
    {
        juce::Rectangle<int> oldBounds = it->bounds;
        it->bounds.translate(0, -1);
        it->alpha -= 0.01f;

        if (it->alpha <= 0.0f)
        {
            repaint(oldBounds);
            it = fallingNotes.erase(it);
        }
        else {
            repaint(it->bounds);
            ++it;
        }
        needsRepaint = true;
    }
    if (needsRepaint)
        repaint();
}
