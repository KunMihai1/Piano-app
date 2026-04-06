/*
  ==============================================================================

    SoundEffectWindowComponent.h
    Created: 5 Apr 2026 11:24:18pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 *  A single rotary knob with a custom arc-style paint, a label underneath,
 *  and a numeric value readout.  Designed for controlling one MIDI CC.
 */
class EffectKnobComponent : public juce::Component
{
public:
    EffectKnobComponent(const juce::String& name, int ccNumber,
                        juce::Colour accentColour);
    ~EffectKnobComponent() override;

    /** Hook: called with (ccNumber, newValue 0-127) whenever the user moves the knob. */
    std::function<void(int ccNumber, int value)> onValueChanged;

    void setValue(double newValue, juce::NotificationType notify = juce::sendNotification);
    double getValue() const;
    int getCCNumber() const { return cc; }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider knob;
    juce::Label  nameLabel;
    juce::Label  valueLabel;
    juce::Colour accent;
    int cc;

    /** Custom LookAndFeel that draws the arc-style rotary knob. */
    class ArcKnobLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        juce::Colour arcColour;

        void drawRotarySlider(juce::Graphics&, int x, int y, int width,
                              int height, float sliderPos,
                              float rotaryStartAngle, float rotaryEndAngle,
                              juce::Slider&) override;
    };

    ArcKnobLookAndFeel arcLnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectKnobComponent)
};

//==============================================================================
/**
 *  An LED-style toggle for Sustain Pedal (CC 64).
 */
class SustainToggleComponent : public juce::Component
{
public:
    SustainToggleComponent();

    /** Hook: called with (64, 127) on press, (64, 0) on release. */
    std::function<void(int ccNumber, int value)> onValueChanged;

    bool isOn() const;
    void setOn(bool shouldBeOn);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::ToggleButton toggle;
    juce::Label nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SustainToggleComponent)
};

//==============================================================================
/**
 *  The main content panel that lives inside the DocumentWindow.
 *  Contains channel selector, 4 rotary effect knobs, and a sustain toggle.
 */
class SoundEffectContentComponent : public juce::Component
{
public:
    SoundEffectContentComponent();
    ~SoundEffectContentComponent() override = default;

    /** Hook: called when the user picks a different channel (1 or 16). */
    std::function<void(int channel)> onChannelChanged;

    /** Returns the currently selected channel (1 or 16). */
    int getSelectedChannel() const;

    /** Access individual knobs for external wiring. */
    EffectKnobComponent& getBrightnessKnob()  { return brightnessKnob; }
    EffectKnobComponent& getExpressionKnob()  { return expressionKnob; }
    EffectKnobComponent& getChorusKnob()      { return chorusKnob; }
    EffectKnobComponent& getResonanceKnob()   { return resonanceKnob; }
    SustainToggleComponent& getSustainToggle() { return sustainToggle; }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Label       headerLabel;
    juce::ComboBox    channelSelector;
    juce::Label       channelLabel;

    EffectKnobComponent  brightnessKnob;   // CC 74
    EffectKnobComponent  expressionKnob;   // CC 11
    EffectKnobComponent  chorusKnob;       // CC 93
    EffectKnobComponent  resonanceKnob;    // CC 71
    SustainToggleComponent sustainToggle;   // CC 64

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundEffectContentComponent)
};

//==============================================================================
/**
 *  Top-level DocumentWindow wrapper.
 */
class SoundEffectWindow : public juce::DocumentWindow,
                          public juce::KeyListener
{
public:
    std::function<void()> onWindowClosed;

    SoundEffectWindow();
    ~SoundEffectWindow() override;

    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;
    void closeButtonPressed() override;

    /** Convenience accessor so callers can wire callbacks on the content. */
    SoundEffectContentComponent& getContent() { return *content; }

private:
    SoundEffectContentComponent* content = nullptr;   // owned by the window via setContentOwned

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundEffectWindow)
};