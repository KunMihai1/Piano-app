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

    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(true);
}

NoteLayer::~NoteLayer()
{
    keyBoardUI.midiHandler.removeListener(this);
    openGLContext.detach();
}

void NoteLayer::paint(juce::Graphics& g)
{
    for (const auto& [midiNote, note] : activeNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.fillRoundedRectangle(note.bounds.toFloat(), 6.0f);
    }
    for (const auto& note : fallingNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(note.alpha));
        g.fillRoundedRectangle(note.bounds.toFloat(), 6.0f);
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

                newNote.height = 10.0f;
                newNote.yPosition = static_cast<float>(getHeight()) - newNote.height;

                newNote.bounds.setHeight(static_cast<int>(round(newNote.height)));
                newNote.bounds.setY(static_cast<int>(round(newNote.yPosition)));

                activeNotes[midiNote] = newNote;

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

void NoteLayer::newOpenGLContextCreated()
{
    const char* vertexSource = R"VERT(
        attribute vec2 position;
        attribute float pointSize;
        attribute vec4 colour;
        varying vec4 vColour;
        void main()
        {
            gl_PointSize = pointSize;
            gl_Position = vec4(position, 0.0, 1.0);
            vColour = colour;
        }
    )VERT";

    const char* fragmentSource = R"FRAG(
        #ifdef GL_ES
        precision mediump float;
        #endif
        varying vec4 vColour;
        void main()
        {
            float dist = length(gl_PointCoord - vec2(0.5));
            float alpha = smoothstep(0.5, 0.0, dist);
            gl_FragColor = vColour * alpha;
        }
    )FRAG";
}

void NoteLayer::renderOpenGL()
{
    
}

void NoteLayer::openGLContextClosing()
{
    
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
    const float riseSpeed = 350.0f;
    const float fadeSpeed = 250.0f;
    const float fadeTime = 5.0f;
    const float fadeRate = 1.0f / fadeTime;

    for (auto& pair : activeNotes)
    {
        AnimatedNote& n = pair.second;

        n.yPosition -= riseSpeed * dt;
        n.height += riseSpeed * dt;

        n.bounds.setY(static_cast<int>(round(n.yPosition)));
        n.bounds.setHeight(static_cast<int>(round(n.height)));
    }

    for (auto it = fallingNotes.begin(); it != fallingNotes.end();)
    {
        AnimatedNote& n = *it;

        n.alpha -= dt * fadeRate;

        n.yPosition -= fadeSpeed * dt;
        n.bounds.setY(static_cast<int>(round(n.yPosition)));

        if (it->alpha <= 0.0f)
            it = fallingNotes.erase(it);
        else ++it;
    }
    repaint();
}
