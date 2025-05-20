/*
  ==============================================================================

    keyboardUI.h
    Created: 26 Mar 2025 9:45:11pm
    Author:  Kisuke

  ==============================================================================
*/
#include "MidiHandler.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <unordered_map>
#include <vector>

class KeyboardUI; // Forward declaration with correct capitalization

class KeyboardUI : public juce::Component, public MidiHandler::Listener
{
public:
    KeyboardUI(MidiHandler& midiHandler);
    ~KeyboardUI();

    void paint(juce::Graphics& g) override;
    void set_min_and_max(const int min, const int max);

    void noteOnReceived(int midiNote) override;
    void noteOffReceived(int midiNote) override;

private:
    friend class NoteLayer;

    void paintKeyboard(juce::Graphics& g);

    bool isDrawn = false;
    struct note {
        juce::Rectangle<int> bounds = { 0,0,0,0 };
        bool isActive = false;
        std::string type = "";
    };

    struct AnimatedNote {
        juce::Rectangle<int> bounds{ 0, 0, 0, 0 };
        float alpha = 1.0f;
        float velocityY = -50.0f;
        bool isFalling = false;
    };

    MidiHandler& midiHandler;
    std::unordered_map<int, note> keys;

    int min_draw = 0, max_draw = 127;
};