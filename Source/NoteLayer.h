/*
  ==============================================================================

    NoteLayer.h
    Created: 20 May 2025 1:36:54pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "MidiHandler.h"
#include "KeyboardUI.h"
#include <juce_opengl/juce_opengl.h>

class NoteLayer : public juce::Component, public MidiHandler::Listener, public juce::Timer, private juce::OpenGLRenderer
{
public:
    NoteLayer(KeyboardUI& referenceKeyboard);
    ~NoteLayer();
    void visibilityChanged() override;
    void timerCallback() override;


    void paint(juce::Graphics& g) override;

    void noteOnReceived(int midiNote) override;
    void noteOffReceived(int midiNote) override;

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

private:
    struct Particle {
        juce::Point<float> pos;
        juce::Point<float> velocity;
        float life = 1.0f;
        float size = 6.0f;
        juce::Colour colour;
    };

    KeyboardUI& keyBoardUI;

    juce::OpenGLContext openGLContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    juce::OpenGLShaderProgram::Attribute* positionAttr = nullptr;
    juce::OpenGLShaderProgram::Attribute* pointSizeAttr = nullptr;
    juce::OpenGLShaderProgram::Attribute* colourAttr = nullptr;

    juce::OpenGLBuffer vbo;
    std::vector<Particle> particles;


    struct AnimatedNote {
        juce::Rectangle<int> bounds = { 0,0,0,0 };
        float yPosition = 0.0f;
        float height = 0.0f;

        float alpha = 1.0f;
        bool isFalling = false;
    };

    std::unordered_map<int, AnimatedNote> activeNotes;
    std::vector<AnimatedNote> fallingNotes;
};