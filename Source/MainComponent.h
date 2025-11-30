/*
  ==============================================================================

    MainComponent.h
    Created: 29 Oct 2025
    Author:  Kisuke

  ==============================================================================
*/

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
#include "addDeviceWindow.h"
#include "SectionsComponent.h"

/**
 * @class SmoothRotarySlider
 * @brief Custom rotary slider with smooth dragging behavior.
 *
 * Overrides mouse drag behavior to allow fine adjustments.
 */
class SmoothRotarySlider : public juce::Slider
{
public:
    /** @brief Constructor sets rotary style */
    SmoothRotarySlider();

    /** @brief Handles mouse down to store starting value and position */
    void mouseDown(const juce::MouseEvent& e) override;

    /** @brief Handles mouse drag for smooth value changes */
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    float dragStartValue = 0.0f;          ///< Slider value at drag start
    juce::Point<float> dragStartPos;      ///< Mouse position at drag start
};

/**
 * @class KnobLookAndFeel
 * @brief Custom LookAndFeel for rotary knob using an image.
 *
 * Draws a knob from BinaryData image and rotates it according to slider value.
 */
class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    /** @brief Constructor loads knob image and sets angles */
    KnobLookAndFeel();

    /**
     * @brief Draws the rotary slider
     * @param g Graphics context
     * @param x X position
     * @param y Y position
     * @param width Width of slider
     * @param height Height of slider
     * @param sliderPosProportional Proportional slider position [0..1]
     * @param rotaryStart Start angle (ignored, using internal angles)
     * @param rotaryEnd End angle (ignored, using internal angles)
     * @param slider Reference to slider
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStart, float rotaryEnd,
                          juce::Slider& slider) override;

private:
    juce::Image rawKnobImage; ///< Knob image
    float rotaryStartAngle;   ///< Start angle of rotation
    float rotaryEndAngle;     ///< End angle of rotation
};

/**
 * @class CustomLookAndFeel
 * @brief LookAndFeel customization for buttons and text.
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    /** @brief Returns font for text buttons */
    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;

    /**
     * @brief Draws button background
     * @param g Graphics context
     * @param button Button reference
     * @param backgroundColour Button background color
     * @param isMouseOverButton True if mouse is over button
     * @param isButtonDown True if button is pressed
     */
    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton,
                              bool isButtonDown) override;
};

/**
 * @class MainComponent
 * @brief Main UI component containing all controls and displays.
 *
 * Handles MIDI input/output, keyboard UI, instrument selection, recording/playback,
 * and all window controls.
 */
class MainComponent : public juce::Component,
    public juce::ChangeListener,
    public juce::KeyListener,
    public juce::Timer,
    public juce::ComboBox::Listener
{
public:
    /** @brief Constructor initializes UI and MIDI */
    MainComponent();

    /** @brief Destructor */
    ~MainComponent() override;

    /** @brief Paints the main window */
    void paint(juce::Graphics& g) override;

    /** @brief Handles component resizing */
    void resized() override;

    /** @brief Handles key presses
     *  @param key Pressed key
     *  @param component Component that received the key
     */
    bool keyPressed(const juce::KeyPress& key, juce::Component* component) override;

    /** @brief Callback for change events */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /** @brief Timer callback for periodic updates */
    void timerCallback() override;

    /** @brief Checks if the currently selected MIDI input device is valid */
    void checkMidiInputDeviceValid();

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void populateUpdateComboBoxDevices();

private:

    int initialWidth = 0;  ///< Initial window width
    int initialHeight = 0; ///< Initial window height

    int xPlay = 750;       ///< X coordinate of play button
    int yPlay = 690;       ///< Y coordinate of play button

    bool hasBeenResized = false; ///< True if window has been resized

    /** @brief Initializes save file for the user */
    void initalizeSaveFileForUser();

    /** @brief Loads settings from disk */
    void loadSettings();

    /** @brief Called when component gains focus */
    void focusGained(FocusChangeType) override;

    // UI toggles
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
    void toggleParticleToggle();
    void toggleHandInstrumentToggle();
    void toggleUpdateKeysButton();
    void toggleDevicesCBUpdate();
    void toggleSections();


    // Initialization functions
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
    void toggleButtonInit();
    void toggleHandButtonsInit();
    void updateKeysButtonInit();
    void devicesCBUpdateInit();
    void sectionsInit();
   

    // Button click callbacks
    void settingsButtonOnClick();
    void midiButtonOnClick();
    void homeButtonOnClick();
    void playButtonOnClick();
    void updateKeysButtonOnClick();

    // Instrument/UI helpers
    juce::Image getImageForInstruments(const std::string& type);
    void buildTree();
    InstrumentTreeItem* createInstrumentItem(const juce::Image& img, const juce::String& name, int program);
    bool openingDevicesForPlay();
    void showColourSelector();
    void showInstrumentSelector();
    void saveRecordingToFile(double tempo = 120.0);
    void playRecordingFromFile(double tempo = 120.0);

    //==========================================================================
    // Member variables
    juce::ApplicationProperties appProperties; ///< Application properties manager
    juce::PropertiesFile* propertiesFile = nullptr; ///< Pointer to properties file

    CustomLookAndFeel customLookAndFeel; ///< Custom look and feel
    juce::Image cachedImageMainWindow;   ///< Cached main window image
    juce::Image playBackground;          ///< Background when playing
    juce::Image currentBackground;       ///< Currently displayed background
    juce::Label helpIcon;                ///< Help icon label
    juce::TooltipWindow tooltipWindow{ this, 200 }; ///< Tooltip manager

    std::weak_ptr<juce::MidiInput> deviceOpenedIN; ///< Currently opened MIDI input device
    std::weak_ptr<juce::MidiOutput> deviceOpenedOUT; ///< Currently opened MIDI output device

    MidiDevice MIDIDevice{};       ///< MIDI device instance
    MidiHandler midiHandler{ MIDIDevice }; ///< MIDI event handler
    KeyboardListener keyListener{ midiHandler }; ///< Keyboard listener
    MidiRecordPlayer recordPlayer{}; ///< MIDI recording/playback

    std::vector<std::string> devicesIN;  ///< List of input devices
    std::vector<std::string> devicesOUT; ///< List of output devices

    int last=-1, lastOfLast=-1;

    // UI buttons and toggles
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton midiButton{ "MIDI Settings" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton homeButton{ "Home" };
    juce::ToggleButton particleToggle;
    juce::TextButton colourSelectorButton{ "Select colour" };
    juce::TextButton instrumentSelectorButton{ "Select instrument" };
    juce::TextButton saveRecordingButton{ "Save recording" };
    juce::TextButton playRecordingFileButton{ "Play recording file" };
    juce::ToggleButton leftHandInstrumentToggle, rightHandInstrumentToggle;
    std::unique_ptr<StyleSectionComponent> introsEndings, variationsFills;


    std::unordered_map<juce::String, juce::String> updateDevicesMap;

    juce::TextButton updateNumberOfKeysDevice{"Update number of keys"};
    juce::ComboBox devicesCBUpdate;

    juce::DrawableButton startRecording{ "Record", juce::DrawableButton::ImageFitted };
    juce::DrawableButton stopRecording{ "Stop", juce::DrawableButton::ImageFitted };
    juce::DrawableButton startPlayback{ "Play", juce::DrawableButton::ImageFitted };
    std::unique_ptr<juce::Drawable> recordDrawable, stopDrawable, playDrawable;
    juce::Slider volumeKnob, reverbKnob;
    KnobLookAndFeel customKnobLookAndFeel;

    juce::ColourSelector* colourSelector; ///< Colour selector pointer
    std::unique_ptr<juce::TreeView> treeView = nullptr; ///< Instrument tree

    // UI windows
    std::unique_ptr<MIDIWindow> midiWindow = nullptr;
    KeyboardUI keyboard{ midiHandler };
    std::unique_ptr<NoteLayer> noteLayer;
    bool keyboardInitialized = false; ///< True if keyboard is initialized
    bool usingKeyboardInput = false;  ///< True if using keyboard input
    std::unique_ptr<TemporaryMessage> temporaryPopup;
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<Display> display = nullptr;
    std::unique_ptr<AddDeviceWindow> addDeviceWindow=nullptr;



    //==========================================================================
    // Panels
    struct Panel : public juce::Component
    {
        /** @brief Paints panel background and borders */
        void paint(juce::Graphics& g) override;
    };
    Panel settingsPanel;

    struct headerPanel : public juce::Component
    {
        /** @brief Paints header gradient */
        void paint(juce::Graphics& g) override;
    };
    headerPanel headerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
