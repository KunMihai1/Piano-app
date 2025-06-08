#pragma once

#include <JuceHeader.h>
#include "settingsWindow.h"
#include "BinaryData.h"
#include "MidiHandler.h"
#include "KeyboardUI.h"
#include "NoteLayer.h"
#include "audioProcessor.h"
#include "keyListener.h"
#include "temporaryNotificationUI.h"
#include "InstrumentTreeItem.h"
#include "MidiRecordPlayer.h"
#include "displayGUI.h"

class SmoothRotarySlider : public juce::Slider
{
public:
    SmoothRotarySlider()
    {
        setSliderStyle(juce::Slider::Rotary);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        dragStartValue = getValue();
        dragStartPos = e.position;
        juce::Slider::mouseDown(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        float dx = e.position.x - dragStartPos.x;
        float dy = dragStartPos.y - e.position.y; // inverted Y-axis

        float sensitivity = 0.005f; // adjust as needed

        float delta = dx + dy;

        float newValue = dragStartValue + delta * sensitivity * (getMaximum() - getMinimum());

        setValue(newValue, juce::dontSendNotification);
    }

private:
    float dragStartValue = 0.0f;
    juce::Point<float> dragStartPos;
};

class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KnobLookAndFeel()
    {
        // Load knob image once (make sure your BinaryData::Knob_png is tightly cropped!)
        rawKnobImage = juce::ImageCache::getFromMemory(BinaryData::Knob_png, BinaryData::Knob_pngSize);

        // Standard JUCE rotary angle range (approx 270 degrees sweep)
        rotaryStartAngle = juce::MathConstants<float>::pi * 1.25f; // 225 degrees
        rotaryEndAngle = juce::MathConstants<float>::pi * 2.75f;   // 495 degrees
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStart, float rotaryEnd,
        juce::Slider& slider) override
    {
        // Use consistent rotary range to avoid glitches
        float startAngle = rotaryStartAngle;
        float endAngle = rotaryEndAngle;

        // Clamp sliderPosProportional just in case
        sliderPosProportional = juce::jlimit(0.0f, 1.0f, sliderPosProportional);

        float angle = startAngle + sliderPosProportional * (endAngle - startAngle);

        const float cx = x + width * 0.5f;
        const float cy = y + height * 0.5f;
        const int knobSize = juce::jmin(width, height);

        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

        if (rawKnobImage.isValid())
        {
            // Rescale knob image to fit knobSize
            juce::Image scaledKnob = rawKnobImage.rescaled(knobSize, knobSize);

            // Transform: translate origin to center, rotate, translate back to center position
            juce::AffineTransform transform =
                juce::AffineTransform::translation(-scaledKnob.getWidth() * 0.5f, -scaledKnob.getHeight() * 0.5f)
                .rotated(angle)
                .translated(cx, cy);

            g.drawImageTransformed(scaledKnob, transform);
        }
        else
        {
            // If image not available, draw a simple rotating line as fallback

            float radius = knobSize * 0.5f - 4.0f;

            // Draw base circle
            g.setColour(juce::Colours::darkgrey);
            g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

            // Draw rotating indicator line
            g.setColour(juce::Colours::orange);
            juce::Point<float> lineStart(cx, cy);
            juce::Point<float> lineEnd(cx + radius * std::cos(angle), cy + radius * std::sin(angle));
            g.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 3.0f);
        }
    }

private:
    juce::Image rawKnobImage;
    float rotaryStartAngle, rotaryEndAngle;
};

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override
    {
        juce::ignoreUnused(button, buttonHeight);
        return juce::Font(30.0f, juce::Font::bold);
    }

    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton,
        bool isButtonDown) override 
    {
        juce::ignoreUnused(button, isMouseOverButton, isButtonDown);
        g.fillAll(backgroundColour);
    }
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component, public juce::ChangeListener, public juce::KeyListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    bool isMouseDownInsideLabel = false;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

private:
    int initialWidth = 0;
    int initialHeight = 0;

    int xPlay = 750, yPlay = 690;

    bool hasBeenResized = false;
    void initalizeSaveFileForUser();
    void loadSettings();


    void focusGained(FocusChangeType) override;
    void toggleSettingsPanel();
    void toggleSettingsButton();
    void toggleMIDIButton();
    void togglePlayButton();
    void toggleMIDIsettingsIcon();
    void toggleHPanel();
    void toggleHomeButton();
    void toggleForPlaying();
    void toggleColourSelectorButton();
    void toggleInstrumentSelectorButton();
    void toggleRecordButtons();
    void toggleSaveRecordingButton();
    void togglePlayRecordingButton();
    void toggleKnobs();
    void toggleDisplay();

    void settingsInit();
    void playButtonInit();
    void keyBoardUIinit(int min, int max);
    void panelInit();
    void midiIconInit();
    void midiSettingsInit();
    void headerPanelInit();
    void homeButtonInit();
    void colourSelectorButtonInit();
    void instrumentSelectorButtonInit();
    void recordButtonsInit();
    void saveRecordingButtonInit();
    void playRecordingButtonInit();
    void knobsInit();
    void displayInit();
    
    void settingsButtonOnClick();
    void midiButtonOnClick();
    void homeButtonOnClick();
    void playButtonOnClick();

    juce::Image getImageForInstruments(const std::string& type);
    void buildTree();
    InstrumentTreeItem* createInstrumentItem(const juce::Image& img, const juce::String& name, int program);
    bool openingDevicesForPlay();
   
    void showColourSelector();
    void showInstrumentSelector();
    void saveRecordingToFile(double tempo=120.0);
    void playRecordingFromFile(double tempo = 120.0);
    
    //void 

    //==============================================================================
    // Your private member variables go here...
    juce::ApplicationProperties appProperties;
    juce::PropertiesFile* propertiesFile = nullptr;

    CustomLookAndFeel customLookAndFeel;
    juce::Image cachedImageMainWindow;
    juce::Image playBackground;
    juce::Image currentBackground;
    juce::Label helpIcon;
    juce::TooltipWindow tooltipWindow{ this, 200 };


    const juce::MidiInput* deviceOpenedIN = nullptr;
    const juce::MidiOutput* deviceOpenedOUT = nullptr;
    MidiDevice MIDIDevice{};
    MidiHandler midiHandler{ MIDIDevice };
    KeyboardListener keyListener{ midiHandler };
    MidiRecordPlayer recordPlayer{};
    //ReverbProcessor revProcessor{ midiHandler };

    std::vector<std::string> devicesIN;
    std::vector<std::string> devicesOUT;

    //UI->refresh button+label of AVAILABLE DEVICES

    //UI->settings
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton midiButton{ "MIDI Settings" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton homeButton{ "Home" };
    juce::TextButton colourSelectorButton{ "Select colour" };
    juce::TextButton instrumentSelectorButton{ "Select instrument" };
    juce::TextButton saveRecordingButton{ "Save recording" };
    juce::TextButton playRecordingFileButton{ "Play recording file" };

    juce::DrawableButton startRecording{"Record",juce::DrawableButton::ImageFitted};
    juce::DrawableButton stopRecording{ "Stop", juce::DrawableButton::ImageFitted };
    juce::DrawableButton startPlayback{ "Play", juce::DrawableButton::ImageFitted };
    std::unique_ptr<juce::Drawable> recordDrawable, stopDrawable, playDrawable;
    juce::Slider volumeKnob, reverbKnob;
    KnobLookAndFeel customKnobLookAndFeel;

    juce::ColourSelector* colourSelector;
    std::unique_ptr<juce::TreeView> treeView=nullptr;


    //UI->windows

    std::unique_ptr<MIDIWindow> midiWindow = { nullptr };

    KeyboardUI keyboard{ midiHandler };
    std::unique_ptr<NoteLayer> noteLayer;
    bool keyboardInitialized = false;

    bool usingKeyboardInput = false;

    std::unique_ptr<TemporaryMessage> temporaryPopup;
    std::unique_ptr<juce::FileChooser> fileChooser;

    std::unique_ptr<Display> display=nullptr;

private:
    struct Panel : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            juce::Colour translucentGray = juce::Colour::fromRGBA(169, 169, 169, 204); // Semi-transparent gray (80% opacity)
            juce::Colour option = juce::Colour::fromRGBA(50, 100, 95, 170);

            juce::Colour subtleBorderColor = juce::Colour::fromRGBA(169, 169, 169, 200);
            juce::Colour secondBorder = juce::Colour::fromRGBA(140, 140, 140, 220);

            g.fillAll(option); // Background of the panel
            g.setColour(subtleBorderColor);
            g.drawRect(getLocalBounds(), 3);

        }
    };

    Panel settingsPanel;

    struct headerPanel : public::juce::Component
    {
        void paint(juce::Graphics& g) override {
            juce::Colour startColour = juce::Colour(128, 0, 32);   // Deep Burgundy
            juce::Colour endColour = juce::Colour(212, 175, 55);  // Metallic Gold
            //juce::Colour startColour = juce::Colour(160, 40, 60); // Softer burgundy
            //juce::Colour endColour = juce::Colour(210, 165, 100); // Lighter gold
            juce::ColourGradient gradient(startColour, 0, 0, endColour, 0, 50, false);
            g.setGradientFill(gradient);
            g.fillAll();
        }
    };

    headerPanel headerPanel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

