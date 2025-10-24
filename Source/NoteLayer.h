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

/**
 * @class NoteLayer
 * @brief Renders and animates piano notes in real-time, including particle effects.
 * 
 * This class handles both active and falling notes, integrates with OpenGL for rendering,
 * and reacts to MIDI events from a KeyboardUI reference.
 */
class NoteLayer : public juce::Component, public MidiHandlerListener , public juce::Timer, private juce::OpenGLRenderer
{
public:
    /**
     * @brief Constructs a NoteLayer with a reference to the keyboard UI.
     * @param referenceKeyboard Reference to the KeyboardUI to sync note positions.
     */
    NoteLayer(KeyboardUI& referenceKeyboard);

    /**
     * @brief Destructor; detaches OpenGL context.
     */
    ~NoteLayer();

    /**
     * @brief Handles visibility changes; stops timer if hidden.
     */
    void visibilityChanged() override;

    /**
     * @brief Timer callback to update note animations and particles.
     */
    void timerCallback() override;

    /**
     * @brief Handles component resizing; updates OpenGL viewport.
     */
    void resized() override;

    /**
     * @brief Paints active and falling notes using JUCE Graphics API.
     * @param g Reference to the Graphics context.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Callback for when a MIDI note-on is received.
     * @param midiNote The MIDI note number that was pressed.
     */
    void noteOnReceived(int midiNote) override;

    /**
     * @brief Callback for when a MIDI note-off is received.
     * @param midiNote The MIDI note number that was released.
     */
    void noteOffReceived(int midiNote) override;

    /**
     * @brief Called when the OpenGL context is created. Sets up shaders and buffers.
     */
    void newOpenGLContextCreated() override;

    /**
     * @brief Renders the OpenGL content each frame.
     */
    void renderOpenGL() override;

    /**
     * @brief Called when the OpenGL context is closing; releases resources.
     */
    void openGLContextClosing() override;

    /**
     * @brief Resets all note states and clears particles.
     */
    void resetState();

    /**
     * @brief Resets only the active notes, converting them to falling notes.
     */
    void resetStateActiveNotes();

    /**
     * @brief Sets the particle color for notes.
     * @param colour The JUCE colour to apply to spawned particles.
     */
    void setColourParticle(juce::Colour& colour);

    /**
     * @brief Enables or disables particle spawning for note events.
     * @param state True to spawn particles, false to disable.
     */
    void setSpawnParticleState(bool state);

private:
    /**
     * @brief Updates all particles positions, velocities, and life.
     */
    void updateParticles();

    /**
     * @brief Spawns particles for a given note.
     * @param midiNote The MIDI note number to spawn particles for.
     */
    void spawnParticlesForNote(int midiNote);

    struct Particle {
        juce::Point<float> pos;
        juce::Point<float> velocity;
        float life = 1.0f;
        float size = 6.0f;
        juce::Colour colour;
    };

    KeyboardUI& keyBoardUI;
    juce::Colour noteColourUser;
    juce::Colour particleColourUser;

    juce::OpenGLContext openGLContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    juce::OpenGLShaderProgram::Attribute* positionAttr = nullptr;
    juce::OpenGLShaderProgram::Attribute* pointSizeAttr = nullptr;
    juce::OpenGLShaderProgram::Attribute* colourAttr = nullptr;

    GLuint particleVBO = 0;
    std::vector<Particle> particles;


    struct AnimatedNote {
        juce::Rectangle<int> bounds = { 0, 0, 0, 0 };
        float yPosition = 0.0f;
        float height = 0.0f;
        float initialHeight = 0.0f;

        float alpha = 1.0f;
        bool isFalling = false;

    };

    std::unordered_map<int, AnimatedNote> activeNotes;
    std::vector<AnimatedNote> fallingNotes;

    bool isActive;
    bool spawnParticleState = false;
};
