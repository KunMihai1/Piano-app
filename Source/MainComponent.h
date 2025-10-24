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
/**
 * @class MainComponent
* @brief Main GUI component for the application.
*
* Handles UI elements, MIDI input/output, playback, recording, 
* and keyboard shortcuts.
*/
class MainComponent : public juce::Component, public juce::ChangeListener, public juce::KeyListener, public juce::Timer
{
public:
    //==============================================================================

    /**
     * @brief Constructor.
     * Initializes UI components, MIDI devices, background images,
     * listeners, and settings.
     */
    MainComponent();

    /**
     * @brief Destructor.
     * Safely closes MIDI devices, removes listeners, and cleans up UI components.
     */
    ~MainComponent() override;

    //==============================================================================
    /**
     * @brief Paint callback.
     * Draws the background image and sets colors/fonts.
     * @param g Graphics context.
     */
    void paint(juce::Graphics&) override;

    /**
     * @brief Resize callback.
     * Dynamically positions all child components and UI elements.
     */
    void resized() override;
    bool isMouseDownInsideLabel = false;

    /**
     * @brief Change listener callback.
     * Handles changes from colour selectors and updates visual effects.
     * @param source The broadcaster that triggered the change.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /**
     * @brief Key press handler.
     * Handles keyboard shortcuts for adjusting MIDI keyboard ranges.
     * @param key The key that was pressed.
     * @param component The component that received the key press.
     * @return True if key was handled, false otherwise.
     */
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

    /**
     * @brief Timer callback.
     * Periodically checks if MIDI input device is still connected.
     */
    void timerCallback() override;

    /**
     * @brief Checks if the currently selected MIDI input device is valid.
     * Resets active notes and closes devices if disconnected.
     */
    void checkMidiInputDeviceValid();

private:
    int initialWidth = 0;
    int initialHeight = 0;

    int xPlay = 750, yPlay = 690;

    bool hasBeenResized = false;

    /**
     * @brief Initializes a user-specific save file for storing application settings.
     */
    void initalizeSaveFileForUser();

    
    /**
     * @brief Loads saved settings from the user properties file.
     *
     * Restores MIDI volume, reverb, and selected instruments.
     */
    void loadSettings();

    /**
     * @brief Callback triggered when the component gains focus.
     * Brings the noteLayer to the front.
     * @param type Focus change type.
     */
    void focusGained(FocusChangeType) override;

    /**
     * @brief Toggles the visibility of the settings panel.
     */
    void toggleSettingsPanel();

    /**
     * @brief Toggles the visibility of the settings button.
     */
    void toggleSettingsButton();

    /**
     * @brief Toggles the visibility of the MIDI button.
     */
    void toggleMIDIButton();

    /**
     * @brief Toggles the visibility of the play button.
     */
    void togglePlayButton();

    /**
     * @brief Toggles the visibility of the MIDI settings/help icon.
     */
    void toggleMIDIsettingsIcon();

    /**
     * @brief Toggles the visibility of the header panel and all its subcomponents.
     */
    void toggleHPanel();

    
    /**
     * @brief Toggles the visibility of the home button.
     */
    void toggleHomeButton();

    /**
     * @brief Hides multiple UI components to prepare the interface for playing mode.
     */
    void toggleForPlaying();

    /**
     * @brief Toggles the visibility of the colour selector button.
     */
    void toggleColourSelectorButton();

    /**
     * @brief Toggles the visibility of the instrument selector button.
     */
    void toggleInstrumentSelectorButton();

    /**
     * @brief Toggles the visibility of the record, stop, and playback buttons.
     */
    void toggleRecordButtons();

    /**
     * @brief Toggles the visibility of the save recording button.
     */
    void toggleSaveRecordingButton();

    /**
     * @brief Toggles the visibility of the play recording button.
     */
    void togglePlayRecordingButton();

    /**
     * @brief Toggles the visibility of volume and other knobs.
     */
    void toggleKnobs();

    /**
     * @brief Toggles the visibility of the main display.
     */
    void toggleDisplay();

    /**
     * @brief Toggles the visibility of the particle effects toggle button.
     */
    void toggleParticleToggle();

    /**
     * @brief Toggles the visibility of left and right hand instrument toggles.
     */
    void toggleHandInstrumentToggle();

    /**
     * @brief Initializes the settings button and related panels.
     */
    void settingsInit();

    /**
     * @brief Initializes the play button.
     */
    void playButtonInit();

    /**
     * @brief Initializes the on-screen keyboard UI and note layer.
     *
     * @param min Minimum MIDI note
     * @param max Maximum MIDI note
     *
     * Adds the keyboard and noteLayer to the component and sets bounds.
     */
    void keyBoardUIinit(int min, int max);

    /**
     * @brief Initializes the settings panel.
     *
     *Adds it to the component, brings it to front, and hides it initially.
     */
    void panelInit();

    /**
     * @brief Initializes the settings panel.
     *
     * Adds it to the component, brings it to front, and hides it initially.
     */
    void midiIconInit();

    /**
     * @brief Initializes the MIDI settings button.
     *
     * Sets text, click callback, cursor, adds to settings panel, hides it initially.
     */
    void midiSettingsInit();

    /**
     * @brief Initializes the header panel and all its child components.
     *
     * Adds header panel, brings to front, hides it initially, and calls init for:
     * home button, colour selector, instrument selector, record buttons,
     * save/play recording buttons, particle toggle, hand toggles, and knobs.
     */
    void headerPanelInit();

    /**
     * @brief Initializes the home button.
     *
     * Sets text, cursor, click callback, adds to header panel, hides it initially.
     */
    void homeButtonInit();

    /**
     * @brief Initializes the colour selector button inside the header panel.
     */
    void colourSelectorButtonInit();

    /**
     * @brief Initializes the instrument selector button inside the header panel.
     */
    void instrumentSelectorButtonInit();

    /**
     * @brief Initializes the record, stop, and playback buttons.
     *
     * Configures SVG images, mouse cursors, click callbacks,
     * and temporary messages for user feedback.
     */
    void recordButtonsInit();

    /**
     * @brief Initializes the save recording button.
     *
     * Configures click callback and adds it to the header panel.
     */
    void saveRecordingButtonInit();

    /**
     * @brief Initializes the play recording file button.
     *
     * Sets the button text, click callback, cursor, adds it to the header panel,
     * and hides it initially.
     */
    void playRecordingButtonInit();

    /**
     * @brief Initializes volume and other knobs.
     *
     * Configures the rotary slider style, look and feel, adds to the header panel,
     * and hides it initially.
     */
    void knobsInit();

    /**
     * @brief Initializes the main display component.
     *
     * Creates a display, adds it to the header panel, and hides it initially.
     */
    void displayInit();

    
    /**
     * @brief Initializes the particle toggle button.
     *
     * Configures text, cursor, click callback to control particle spawning,
     * sets initial toggle state, adds to header panel, and hides it.
     */
    void toggleButtonInit();

    /**
     * @brief Callback when settings button is clicked.
     *
     * Toggles settings panel, MIDI button, and help icon visibility.
     */
    void toggleHandButtonsInit();

    
    /**
     * @brief Callback when MIDI button is clicked.
     *
     * Opens a MIDI settings window if not already opened, otherwise brings it to front.
     */
    void settingsButtonOnClick();

    /**
     * @brief Callback when MIDI button is clicked.
     *
     * Opens a MIDI settings window if not already opened, otherwise brings it to front.
     */
    void midiButtonOnClick();

    
    /**
     * @brief Callback when play button is clicked.
     *
     * Opens MIDI devices for playback, sets up keyboard, note layer, display,
     * MIDI handler, record player, and hides unnecessary UI components.
     */
    void homeButtonOnClick();

    /**
     * @brief Loads and returns an image for a given instrument type.
     *
     * @param type The instrument type string identifier
     *@return juce::Image corresponding to the instrument type, or an empty image if type unknown
     */
    void playButtonOnClick();

    /**
     * @brief Builds the instrument tree for selection UI.
     *
     * Adds categories like Pianos, Basses, Guitars, Woodwinds, Brass, Strings, Reeds, Organs,
     * creates instrument items, and attaches them to the tree view.
     */
    juce::Image getImageForInstruments(const std::string& type);

    /**
     * @brief Builds the instrument tree for selection UI.
     *
     * Adds categories like Pianos, Basses, Guitars, Woodwinds, Brass, Strings, Reeds, Organs,
     * creates instrument items, and attaches them to the tree view.
     */
    void buildTree();

    /**
     * @brief Creates a tree item representing an instrument.
     *
     * @param img Image of the instrument
     * @param name Name of the instrument
     * @param program Program number for MIDI
     * @return Pointer to a new InstrumentTreeItem
     *
     * Sets up the onProgramSelected callback to update MIDI handler,
     * record player, and properties file when instrument is selected.
     */
    InstrumentTreeItem* createInstrumentItem(const juce::Image& img, const juce::String& name, int program);

    /**
     * @brief Opens the selected MIDI input and output devices for playback.
     *
     * Checks for available devices and opens them.
     * If the PC keyboard is selected as input, sets keyboard input mode.
     * @return true if devices successfully opened, false otherwise.
     */
    bool openingDevicesForPlay();

    /**
     * @brief Displays the colour selector UI for particle effects.
     *
     * Hides the note layer, disables keyboard drawing,
     * and launches the colour selector in a CallOutBox.
     */
    void showColourSelector();

    /**
     * @brief Displays the instrument selector tree view UI.
     *
     * Builds the instrument tree, hides the note layer,
     * disables keyboard drawing, and launches the tree view in a CallOutBox.
     */
    void showInstrumentSelector();

    /**
     * @brief Saves the current recording to a MIDI file.
     *
     * Opens a FileChooser to select the save location.
     * @param tempo Optional tempo value to save with the recording.
     */
    void saveRecordingToFile(double tempo=120.0);

    /**
     * @brief Plays a recording from a MIDI file.
     *
     * Opens a FileChooser to select the MIDI file.
     * Parses the file and starts playback if valid.
     * @param tempo Optional tempo value for playback.
     */
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


    std::weak_ptr<juce::MidiInput> deviceOpenedIN;
    std::weak_ptr<juce::MidiOutput> deviceOpenedOUT;

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
    juce::ToggleButton particleToggle;
    juce::TextButton colourSelectorButton{ "Select colour" };
    juce::TextButton instrumentSelectorButton{ "Select instrument" };
    juce::TextButton saveRecordingButton{ "Save recording" };
    juce::TextButton playRecordingFileButton{ "Play recording file" };

    juce::ToggleButton leftHandInstrumentToggle,rightHandInstrumentToggle;


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
            juce::Colour translucentGray = juce::Colour::fromRGBA(169, 169, 169, 204);
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

