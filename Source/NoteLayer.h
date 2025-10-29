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
 * @brief Visual layer displaying active and falling MIDI notes with particle effects.
 *
 * This component listens to MIDI note events, draws active notes and falling notes,
 * and manages particle effects for note-on events. It supports OpenGL rendering
 * for performant particle animations and visual effects.
 */
class NoteLayer : public juce::Component, public MidiHandlerListener, public juce::Timer, private juce::OpenGLRenderer
{
public:
    /**
     * @brief Constructor
     * @param referenceKeyboard Reference to the KeyboardUI to sync note positions.
     */
    NoteLayer(KeyboardUI& referenceKeyboard);

    /** Destructor */
    ~NoteLayer();

    /** @brief Handles visibility changes (stops timer if hidden) */
    void visibilityChanged() override;

    /** @brief Timer callback for note animation and particle updates */
    void timerCallback() override;

    /** @brief Called when the component is resized */
    void resized() override;

    /** @brief Paints the component (active and falling notes) */
    void paint(juce::Graphics& g) override;

    /** @brief Called when a MIDI note-on is received */
    void noteOnReceived(int midiNote) override;

    /** @brief Called when a MIDI note-off is received */
    void noteOffReceived(int midiNote) override;

    /** @brief Called when OpenGL context is created */
    void newOpenGLContextCreated() override;

    /** @brief OpenGL rendering callback */
    void renderOpenGL() override;

    /** @brief Called before OpenGL context is destroyed */
    void openGLContextClosing() override;

    /** @brief Resets all visual states including active and falling notes */
    void resetState();

    /** @brief Resets only active notes (triggers note-off for each) */
    void resetStateActiveNotes();

    /** @brief Sets the particle color for note effects */
    void setColourParticle(juce::Colour& colour);

    /** @brief Enables or disables spawning of particles */
    void setSpawnParticleState(bool state);

private:
    /** @brief Updates all active particles' positions and life */
    void updateParticles();

    /** @brief Spawns particles for a specific MIDI note */
    void spawnParticlesForNote(int midiNote);

    /** @struct Particle
     *  @brief Represents a particle in the OpenGL particle system.
     */
    struct Particle {
        juce::Point<float> pos;     /**< Particle position in NDC coordinates */
        juce::Point<float> velocity;/**< Particle velocity */
        float life = 1.0f;          /**< Remaining life (0 = dead) */
        float size = 6.0f;          /**< Particle size */
        juce::Colour colour;        /**< Particle color */
    };

    /** @struct AnimatedNote
     *  @brief Stores visual state of a note (active or falling).
     */
    struct AnimatedNote {
        juce::Rectangle<int> bounds = {0,0,0,0}; /**< Note bounds for rendering */
        float yPosition = 0.0f;                  /**< Current vertical position */
        float height = 0.0f;                     /**< Current height */
        float initialHeight = 0.0f;             /**< Original height at release */
        float alpha = 1.0f;                      /**< Transparency for fading */
        bool isFalling = false;                  /**< Whether note is falling */
    };

    KeyboardUI& keyBoardUI;                    /**< Reference to keyboard UI */
    juce::Colour noteColourUser;               /**< Base note color */
    juce::Colour particleColourUser;           /**< Particle color */

    juce::OpenGLContext openGLContext;         /**< OpenGL rendering context */
    std::unique_ptr<juce::OpenGLShaderProgram> shader; /**< Shader program for particles */
    juce::OpenGLShaderProgram::Attribute* positionAttr = nullptr;
    juce::OpenGLShaderProgram::Attribute* pointSizeAttr = nullptr;
    juce::OpenGLShaderProgram::Attribute* colourAttr = nullptr;

    GLuint particleVBO = 0;                    /**< Vertex buffer object for particles */
    std::vector<Particle> particles;           /**< Active particles */

    std::unordered_map<int, AnimatedNote> activeNotes; /**< Notes currently pressed */
    std::vector<AnimatedNote> fallingNotes;           /**< Notes released and falling */

    bool isActive;                             /**< Whether the layer is active */
    bool spawnParticleState = false;           /**< Whether particle spawning is enabled */
};
