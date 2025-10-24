#pragma once

#include <JuceHeader.h>
#include "MidiDevicesDB.h"
#include "InstrumentHandler.h"
#include "MidiHandlerAbstractSubject.h"
#include "displayGUI.h"

/**
 * @class MidiDevice
 * @brief Handles MIDI input/output devices, volume, reverb, and note range.
 */

class MidiDevice {
public:

	/**
 	* @brief Default constructor.
 	*
	* Initializes volume and reverb to 50, minNote to 0, maxNote to 127.
 	* Adds "PC Keyboard" as a default input device.
 	*/
	MidiDevice();

	/**
 	* @brief Destructor.
 	*
 	* Clears cached devices, stops any active input device,
 	* and releases input/output shared pointers.
 	*/
	~MidiDevice();

	/**
 	* @brief Populates a vector with available MIDI input devices.
 	* @param devices Vector to store device names.
 	*/
	void getAvailableDevicesMidiIN(std::vector<std::string>& devices);

	
	/**
 	* @brief Populates a vector with available MIDI output devices.
 	* @param devices Vector to store device names.
 	*/
	void getAvailableDevicesMidiOUT(std::vector<std::string>& devices);

	/**
 	* @brief Returns the name of the device based on index.
	* @param Index Index of the device.
 	* @param choice 0 for input, 1 for output.
 	* @return Device name as juce::String.
 	*/
	juce::String getDeviceNameBasedOnIndex(int Index, int choice = 0);

	/**
 	* @brief Returns the unique identifier of the device based on index.
	* @param Index Index of the device.
 	* @param choice 0 for input, 1 for output.
 	* @return Device identifier string.
 	*/
	juce::String getDeviceIdentifierBasedOnIndex(int Index, int choice = 0);

	/**
 	* @brief Opens a MIDI input device.
 	* @param DeviceIndex Index of the input device.
 	* @param CallBack Midi input callback pointer.
 	* @return true if successfully opened, false otherwise.
 	*/
	bool deviceOpenIN(int DeviceIndex, juce::MidiInputCallback* CallBack);

	
	/**
 	* @brief Opens a MIDI output device.
 	* @param DeviceIndex Index of the output device.
 	* @return true if successfully opened, false otherwise.
 	*/
	bool deviceOpenOUT(int DeviceIndex);

	/**
 	* @brief Returns whether the input device is open.
 	* @return true if input is open.
 	*/
	bool isOpenIN() const;

	/**
 	* @brief Returns whether the output device is open.
 	* @return true if output is open.
 	*/
	bool isOpenOUT() const;

	/**
 	* @brief Closes the input device.
 	*/
	void deviceCloseIN();

	
	/**
 	* @brief Closes the output device.
 	*/
	void deviceCloseOUT();

	/**
 	* @brief Gets the current input device index.
 	*/
	int getDeviceIndexIN() const;

	/**
 	* @brief Sets the current input device index.
 	* @param index Input device index.
 	*/
	void setDeviceIN(const int index);

	/**
 	* @brief Sets the current output device index.
 	* @param index Output device index.
 	*/
	int getDeviceIndexOUT() const;

	/**
 	* @brief Sets the current output device index.
 	* @param index Output device index.
 	*/
	void setDeviceOUT(const int index);

	/**
 	* @brief Returns a weak pointer to the current input device.
 	*/
	std::weak_ptr<juce::MidiInput> getDeviceIN() const;

	/**
 	* @brief Returns a weak pointer to the current output device.
 	*/
	std::weak_ptr<juce::MidiOutput> getDeviceOUT() const; 

	/**
 	* @brief Sets device volume.
 	* @param vValue Volume in 0-100 range.
 	*/
	void setVolume(const float vValue);

	/**
 	* @brief Returns device volume.
 	* @return Volume as float.
 	*/
	float getVolume() const;

	/**
	* @brief Sets device reverb.
 	* @param rValue Reverb in 0-100 range.
 	*/
	void setReverb(const float rValue) ;

	/**
 	* @brief Returns device reverb.
 	* @return Reverb as float.
 	*/
	float getReverb() const;

	/**
 	* @brief Returns minimum note.
 	*/
	int get_minNote() const;

	/**
 	* @brief Returns maximum note.
 	*/
	int get_maxNote() const;

	/**
 	* @brief Set minimum note for the device.
 	* @param minNoteReceived Minimum note number.
 	*/
	void set_minNote(const int minNoteReceived);

	/**
 	* @brief Set maximum note for the device.
 	* @param maxNoteReceived Maximum note number.
 	*/
	void set_maxNote(int maxNoteReceived);

	/**
 	* @brief Sends a MIDI volume change to the output device.
 	*/
	void changeVolumeInstrument();

	/**
 	* @brief Sends a MIDI reverb change to the output device.
 	*/
	void changeReverbInstrument();

	/**
 	* @brief Returns the device identifier.
 	*/
	const juce::String& get_identifier() const;

	/**
 	* @brief Extracts the PID from a device identifier string.
 	* @param identifierReceived Identifier string.
 	* @return PID as juce::String.
 	*/
	juce::String extractPID(const juce::String& identifierReceived);

	/**
 	* @brief Extracts the VID from a device identifier string.
	* @param identifierReceived Identifier string.
 	* @return VID as juce::String.
 	*/
	juce::String extractVID(const juce::String& indentifierReceived);

private:
	friend class MidiHandler;

	
	/**
 	* @brief Refreshes the cached list of MIDI devices.
 	* @param choice 0 for input devices, 1 for output devices.
 	*/
	void refreshDeviceList(int choice = 0);

	std::vector<std::string> currentDevicesIN;
	juce::Array<juce::MidiDeviceInfo> CachedDevicesIN;


	std::vector<std::string> currentDevicesOUT;
	juce::Array<juce::MidiDeviceInfo> CachedDevicesOUT;

	int currentDeviceIDin;
	int currentDeviceIDout;


	std::shared_ptr<juce::MidiInput> currentDeviceUSEDin = nullptr;

	std::shared_ptr<juce::MidiOutput> currentDeviceUSEDout=nullptr;

	bool devicesChange = false;
	bool isdeviceOpenIN = false;
	bool isdeviceOpenOUT = false;
	juce::String identifier;

	float reverb{ 0.0f };
	float volume{ 0.0f };
	int minNote, maxNote;
};


/**
 * @class MidiHandler
 * @brief Handles incoming and outgoing MIDI messages for a MidiDevice.
 */

class MidiHandler :public juce::MidiInputCallback, public DisplayListener 
{
public:
	std::function<void()> onStartNoteSetting;
	std::function<void()> onEndNoteSetting;

	/**
 	* @brief Constructor.
 	* @param device MidiDevice reference.
 	*/
	MidiHandler(MidiDevice& device);

	/**
 	* @brief Destructor.
 	*/
	~MidiHandler();

	/**
 	* @brief Processes an incoming MIDI message.
 	* @param source MIDI input source.
 	* @param message MIDI message to handle.
 	*/
	void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

	/**
 	* @brief Sets playable range according to VID/PID.
 	*/
	void handlePlayableRange(const juce::String& vid, const juce:: String& pid);

	/**
 	* @brief Adds a listener to receive MIDI events.
 	* @param listener Reference to an object implementing MidiHandlerListener.
 	*
 	* The listener will be notified of:
 	* - Note On/Off events
 	* - Incoming MIDI messages
 	* - Other relevant MIDI events.
 	*/
	void addListener(MidiHandlerListener* listener) { listeners.add(listener); }

	/**
 	* @brief Removes a previously added listener.
 	* @param listener Reference to the MidiHandlerListener to remove.
	*
 	* After removal, the listener will no longer receive MIDI events.
 	*/
	void removeListener(MidiHandlerListener* listener) { listeners.remove(listener); }

	/**
 	* @brief Adds pending MIDI messages to the output buffer.
 	* @param destBuffer Destination MIDI buffer.
 	* @param startSample Start sample index.
 	* @param numSamples Number of samples to process.
 	*/
	void getNextMidiBlock(juce::MidiBuffer& destBuffer, int startSample, int numSamples);

	/**
 	* @brief Sends a note-on message from the keyboard.
	* @param note Note number.
 	* @param velocity Velocity byte.
 	*/
	void noteOnKeyboard(int note, juce::uint8 velocity);

	/**
 	* @brief Sends a note-off message from the keyboard.
 	* @param note Note number.
 	* @param velocity Velocity byte.
 	*/
	void noteOffKeyboard(int note, juce::uint8 velocity);

	
	/**
 	* @brief Sends note-off for all 128 MIDI notes.
 	*/
	void allOffKeyboard();

	/**
 	* @brief Sets the MIDI program (instrument) for left or right hand.
	* @param toSetNumber Program number.
 	* @param choice "left" or "right" hand.
 	*/
	void setProgramNumber(int toSetNumber, const juce::String& choice="");

	/**
 	* @brief Returns the program number for left hand.
 	*/
	int getProgramNumberLeftHand();

	
	/**
 	* @brief Returns the program number for right hand.
 	*/
	int getProgramNumberRightHand();

	/**
 	* @brief Sets start and end notes for playable range.
 	*/
	void set_start_end_notes(int start, int end);

	/**
 	* @brief Sets left and right hand bounds for channel assignment.
 	*/
	void set_left_right_bounds(int left, int right);

	/**
 	* @brief Updates playable range based on playback settings.
 	*/
	void playBackSettingsChanged(const PlayBackSettings& settings) override;


	/**
 	* @brief Sets the MIDI channel based on the hand (left/right) using note.
 	*/
	void setCorrectChannelBasedOnHand(int note);

private:
	//void sendCCifChanged(int ccNumber, int value, int& lastSentValue);

	/**
	 * @brief Applies instrument preset including CC values to the device.
 	*/
	void applyInstrumentPreset(int programNumber, std::vector<std::pair<int, int>> ccValues, const juce::String& choice="");

	/**
 	* @brief Sets the note range for the device based on number of keys.
 	* @param nrKeys Number of keys on the instrument.
 	*/
	void setPlayableRange(int nrKeys);
	juce::ListenerList<MidiHandlerListener> listeners;

	MidiDevice& midiDevice;
	MidiDevicesDataBase dataBase;
	InstrumentHandler instrumentHandler{};

	bool receivedValidNote = false;
	juce::MidiBuffer incomingMidiMessages;
	juce::CriticalSection midiMutex;
	int programNumberLeftHand = 0;
	int programNumberRightHand = 0;
	int startNoteSetting=-1;
	int endNoteSetting=-1;
	int leftHandBoundSetting = -1;
	int rightHandBoundSetting = -1;
	int channel = 1;

	int lastCC91=-1; //reverb
	int lastCC74 = -1;  //brightness
	int lastCC11 = -1; //expression
	int lastCC93 = -1; //chours
	int lastCC71 = -1; //resonance
	int lastCC64 = -1; //sustain pedal;


};
