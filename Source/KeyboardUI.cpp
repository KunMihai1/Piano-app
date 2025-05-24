/*
  ==============================================================================

    keyboardUI.cpp
    Created: 26 Mar 2025 9:45:11pm
    Author:  Kisuke

  ==============================================================================
*/

#include "KeyboardUI.h"

KeyboardUI::KeyboardUI(MidiHandler& midiHandler) : midiHandler{ midiHandler }
{
    setSize(getWidth(), getHeight());
    midiHandler.addListener(this);
}

KeyboardUI::~KeyboardUI()
{
    midiHandler.removeListener(this);
}

void KeyboardUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintKeyboard(g);
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
            keys[midiNote].isActive = true;
            repaint(keys[midiNote].bounds);
        });
}

void KeyboardUI::noteOffReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            keys[midiNote].isActive = false;
            repaint(keys[midiNote].bounds);
        });
}

void KeyboardUI::setIsDrawn(bool state)
{
    this->isDrawn = state;
}

void KeyboardUI::paintKeyboard(juce::Graphics& g)
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
            int ok = 0;
            if (MidiNote.isActive)
            {
                g.setColour(juce::Colours::green);
            }
            else
            {
                if (MidiNote.type == "white")
                {
                    if (key<min_draw || key>max_draw)
                    {
                        g.setColour(juce::Colours::white.withAlpha(0.5f));
                        ok = 1;
                    }
                    else {
                        g.setColour(juce::Colours::white);
                        ok = 0;
                    }
                }
                else {
                    if (key<min_draw || key>max_draw)
                    {
                        g.setColour(juce::Colours::black.withAlpha(0.5f));
                        ok = 1;
                    }
                    else
                    {
                        g.setColour(juce::Colours::black);
                        ok = 0;
                    }
                }
            }
            g.fillRect(MidiNote.bounds);
            if (ok == 0)
            {
                g.setColour(juce::Colours::black);
                g.drawRect(MidiNote.bounds);
            }
        }
    }
}
