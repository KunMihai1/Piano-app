/*
  ==============================================================================

    SoundEffectWindowComponent.cpp
    Created: 5 Apr 2026 11:24:18pm
    Author:  Kisuke

  ==============================================================================
*/

#include "SoundEffectWindowComponent.h"
#include "AppColours.h"

using namespace AppColours;

//==============================================================================
//  ArcKnobLookAndFeel
//==============================================================================
void EffectKnobComponent::ArcKnobLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius    = juce::jmin(width, height) * 0.38f;
    const float centreX   = (float)x + (float)width  * 0.5f;
    const float centreY   = (float)y + (float)height * 0.5f;
    const float arcWidth  = 3.5f;

    // --- background track ---
    {
        juce::Path track;
        track.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(knobTrack);
        g.strokePath(track, juce::PathStrokeType(arcWidth + 1.0f,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // --- filled arc ---
    const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    {
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                          rotaryStartAngle, toAngle, true);
        g.setColour(arcColour);
        g.strokePath(arc, juce::PathStrokeType(arcWidth,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // --- glow behind the thumb ---
    {
        const float glowRadius = 10.0f;
        const float thumbX = centreX + radius * std::cos(toAngle - juce::MathConstants<float>::halfPi);
        const float thumbY = centreY + radius * std::sin(toAngle - juce::MathConstants<float>::halfPi);

        juce::ColourGradient glow(arcColour.withAlpha(0.35f), thumbX, thumbY,
                                  arcColour.withAlpha(0.0f),
                                  thumbX + glowRadius, thumbY + glowRadius, true);
        g.setGradientFill(glow);
        g.fillEllipse(thumbX - glowRadius, thumbY - glowRadius,
                      glowRadius * 2.0f, glowRadius * 2.0f);
    }

    // --- thumb dot ---
    {
        const float thumbRadius = 5.0f;
        const float thumbX = centreX + radius * std::cos(toAngle - juce::MathConstants<float>::halfPi);
        const float thumbY = centreY + radius * std::sin(toAngle - juce::MathConstants<float>::halfPi);

        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbX - thumbRadius, thumbY - thumbRadius,
                      thumbRadius * 2.0f, thumbRadius * 2.0f);
    }

    // --- inner circle ---
    {
        const float innerR = radius * 0.55f;
        g.setColour(cardBg.brighter(0.05f));
        g.fillEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f);

        g.setColour(knobTrack);
        g.drawEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);
    }
}


//==============================================================================
//  EffectKnobComponent
//==============================================================================
EffectKnobComponent::EffectKnobComponent(const juce::String& name,
                                         int ccNumber,
                                         juce::Colour accentColour)
    : accent(accentColour), cc(ccNumber)
{
    arcLnf.arcColour = accentColour;
    knob.setLookAndFeel(&arcLnf);

    knob.setSliderStyle(juce::Slider::Rotary);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    knob.setRange(0.0, 127.0, 1.0);
    knob.setValue(64.0, juce::dontSendNotification);
    knob.setDoubleClickReturnValue(true, 64.0);
    knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                             juce::MathConstants<float>::pi * 2.8f,
                             true);  // stopAtEnd prevents wrapping
    knob.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(knob);

    knob.onValueChange = [this]()
    {
        valueLabel.setText(juce::String((int)knob.getValue()), juce::dontSendNotification);

        if (onValueChanged)
            onValueChanged(cc, (int)knob.getValue());
    };

    nameLabel.setText(name, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId, subtitleText);
    nameLabel.setFont(juce::FontOptions(13.0f));
    addAndMakeVisible(nameLabel);

    valueLabel.setText(juce::String((int)knob.getValue()), juce::dontSendNotification);
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, titleText);
    valueLabel.setFont(juce::FontOptions(14.0f));
    addAndMakeVisible(valueLabel);
}

void EffectKnobComponent::setValue(double newValue, juce::NotificationType notify)
{
    knob.setValue(newValue, notify);
    valueLabel.setText(juce::String((int)knob.getValue()), juce::dontSendNotification);
}

EffectKnobComponent::~EffectKnobComponent()
{
    knob.setLookAndFeel(nullptr);
}

double EffectKnobComponent::getValue() const { return knob.getValue(); }

void EffectKnobComponent::paint(juce::Graphics& g)
{
    // subtle card background
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    g.setColour(cardBg);
    g.fillRoundedRectangle(bounds, 10.0f);

    // very faint border
    g.setColour(separator);
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);
}

void EffectKnobComponent::resized()
{
    auto area = getLocalBounds().reduced(8);

    // The knob takes up most of the space
    auto knobArea = area.removeFromTop(area.getHeight() - 42);
    knob.setBounds(knobArea.reduced(4));

    // Value readout
    valueLabel.setBounds(area.removeFromTop(18));

    // Name label at bottom
    nameLabel.setBounds(area);
}


//==============================================================================
//  SustainToggleComponent
//==============================================================================
SustainToggleComponent::SustainToggleComponent()
{
    toggle.setButtonText("");
    toggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(toggle);

    toggle.onClick = [this]()
    {
        repaint();
        if (onValueChanged)
            onValueChanged(64, toggle.getToggleState() ? 127 : 0);
    };

    nameLabel.setText("Sustain Pedal", juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId,subtitleText);
    nameLabel.setFont(juce::FontOptions(13.0f));
    addAndMakeVisible(nameLabel);
}

bool SustainToggleComponent::isOn() const { return toggle.getToggleState(); }

void SustainToggleComponent::setOn(bool shouldBeOn)
{
    toggle.setToggleState(shouldBeOn, juce::dontSendNotification);
    repaint();
}

void SustainToggleComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // card background
    g.setColour(cardBg);
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(separator);
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);

    // LED circle
    const bool on = toggle.getToggleState();
    const float ledRadius = 8.0f;
    const float centreX = bounds.getCentreX() - 50.0f;
    const float centreY = bounds.getCentreY() - 6.0f;

    // glow
    if (on)
    {
        juce::ColourGradient glow(sustainOn.withAlpha(0.4f), centreX, centreY,
                                  sustainOn.withAlpha(0.0f),
                                  centreX + 18.0f, centreY + 18.0f, true);
        g.setGradientFill(glow);
        g.fillEllipse(centreX - 18.0f, centreY - 18.0f, 36.0f, 36.0f);
    }

    g.setColour(on ? sustainOn : sustainOff);
    g.fillEllipse(centreX - ledRadius, centreY - ledRadius,
                  ledRadius * 2.0f, ledRadius * 2.0f);

    // ON / OFF label next to LED
    g.setColour(titleText);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText(on ? "ON" : "OFF",
               (int)(centreX + ledRadius + 10), (int)(centreY - 10), 50, 20,
               juce::Justification::centredLeft);
}

void SustainToggleComponent::resized()
{
    auto area = getLocalBounds().reduced(8);

    // The toggle button covers the top area (invisible, just for click handling)
    auto toggleArea = area.removeFromTop(area.getHeight() - 22);
    toggle.setBounds(toggleArea);
    toggle.setAlpha(0.0f);   // invisible — we draw our own LED

    nameLabel.setBounds(area);
}


//==============================================================================
//  SoundEffectContentComponent
//==============================================================================
SoundEffectContentComponent::SoundEffectContentComponent()
    : brightnessKnob ("Brightness",  74, accent1),
      expressionKnob ("Expression",  11, accent2),
      chorusKnob     ("Chorus",      93, accent3),
      resonanceKnob  ("Resonance",   71, accent4),
      sustainToggle  ()
{
    // --- header ---
    headerLabel.setText("Sound Effects", juce::dontSendNotification);
    headerLabel.setFont(juce::FontOptions(22.0f));
    headerLabel.setColour(juce::Label::textColourId, titleText);
    headerLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(headerLabel);

    // --- channel selector ---
    channelLabel.setText("Channel", juce::dontSendNotification);
    channelLabel.setFont(juce::FontOptions(13.0f));
    channelLabel.setColour(juce::Label::textColourId, subtitleText);
    channelLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(channelLabel);

    channelSelector.addItem("First instrument",  1);
    channelSelector.addItem("Second instrument", 2);
    channelSelector.setSelectedId(1, juce::dontSendNotification);
    channelSelector.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    channelSelector.setColour(juce::ComboBox::backgroundColourId, cardBg);
    channelSelector.setColour(juce::ComboBox::outlineColourId,    separator);
    channelSelector.setColour(juce::ComboBox::textColourId,       titleText);
    channelSelector.setColour(juce::ComboBox::arrowColourId,      accent1);

    channelSelector.onChange = [this]()
    {
        int ch = (channelSelector.getSelectedId() == 2) ? 16 : 1;
        if (onChannelChanged)
            onChannelChanged(ch);
    };
    addAndMakeVisible(channelSelector);

    // --- knobs ---
    addAndMakeVisible(brightnessKnob);
    addAndMakeVisible(expressionKnob);
    addAndMakeVisible(chorusKnob);
    addAndMakeVisible(resonanceKnob);

    // --- sustain ---
    addAndMakeVisible(sustainToggle);
}

int SoundEffectContentComponent::getSelectedChannel() const
{
    return (channelSelector.getSelectedId() == 2) ? 16 : 1;
}

void SoundEffectContentComponent::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(panelBg);

    // Subtle top-to-bottom gradient overlay for depth
    juce::ColourGradient grad(background.withAlpha(0.3f), 0, 0,
                              juce::Colours::transparentBlack, 0, (float)getHeight(), false);
    g.setGradientFill(grad);
    g.fillRect(getLocalBounds());

    // Horizontal separator below header row
    g.setColour(separator);
    g.fillRect(16, 52, getWidth() - 32, 1);

    // Horizontal separator above sustain
    auto knobsBottom = 60 + (int)((getHeight() - 60 - 80) * 0.85f);
    g.fillRect(16, knobsBottom + 4, getWidth() - 32, 1);
}

void SoundEffectContentComponent::resized()
{
    auto area = getLocalBounds().reduced(16);

    // --- header row: title on left, channel selector on right ---
    auto headerRow = area.removeFromTop(36);
    headerLabel.setBounds(headerRow.removeFromLeft(180));

    auto selectorArea = headerRow.removeFromRight(200);
    channelLabel.setBounds(selectorArea.removeFromLeft(60));
    channelSelector.setBounds(selectorArea.reduced(0, 4));

    area.removeFromTop(20);  // spacing

    // --- 4 knobs in a row ---
    auto sustainHeight = 70;
    auto knobsArea = area.removeFromTop(area.getHeight() - sustainHeight - 12);
    int knobWidth = knobsArea.getWidth() / 4;

    brightnessKnob.setBounds (knobsArea.removeFromLeft(knobWidth));
    expressionKnob.setBounds (knobsArea.removeFromLeft(knobWidth));
    chorusKnob.setBounds     (knobsArea.removeFromLeft(knobWidth));
    resonanceKnob.setBounds  (knobsArea);

    area.removeFromTop(12);  // spacing

    // --- sustain toggle centred ---
    auto sustainArea = area;
    int sustainWidth = juce::jmin(280, sustainArea.getWidth());
    sustainToggle.setBounds(sustainArea.withSizeKeepingCentre(sustainWidth, sustainArea.getHeight()));
}


//==============================================================================
//  SoundEffectWindow  (DocumentWindow)
//==============================================================================
SoundEffectWindow::SoundEffectWindow()
    : juce::DocumentWindow("Sound Effects",
                           background,
                           DocumentWindow::closeButton)
{
    setUsingNativeTitleBar(false);
    setResizable(false, false);
    setTitleBarTextCentred(true);

    // colour the title bar
    setColour(juce::DocumentWindow::backgroundColourId, background);
    setColour(juce::DocumentWindow::textColourId, titleText);

    content = new SoundEffectContentComponent();
    setContentOwned(content, false);

    setBounds(250, 150, 520, 380);
    setVisible(true);

    addKeyListener(this);
}

SoundEffectWindow::~SoundEffectWindow()
{
    removeKeyListener(this);
}

bool SoundEffectWindow::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (key == juce::KeyPress::escapeKey)
    {
        closeButtonPressed();
        return true;
    }
    return false;
}

void SoundEffectWindow::closeButtonPressed()
{
    setVisible(false);

    if (onWindowClosed)
        onWindowClosed();
}