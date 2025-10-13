#pragma once

#include <JuceHeader.h>
#include "MidiDevicesDB.h"
#include "InstrumentHandler.h"
#include "MidiHandlerAbstractSubject.h"
#include "displayGUI.h"

class MidiDevice {
public:
	MidiDevice();
	~MidiDevice();
	void getAvailableDevicesMidiIN(std::vector<std::string>& devices);
	void getAvailableDevicesMidiOUT(std::vector<std::string>& devices);

	juce::String getDeviceNameBasedOnIndex(int Index, int choice = 0);
	juce::String getDeviceIdentifierBasedOnIndex(int Index, int choice = 0);
	bool deviceOpenIN(int DeviceIndex, juce::MidiInputCallback* CallBack);
	bool deviceOpenOUT(int DeviceIndex);
	bool isOpenIN() const;
	bool isOpenOUT() const;
	void deviceCloseIN();
	void deviceCloseOUT();

	int getDeviceIndexIN() const;
	void setDeviceIN(const int index);
	int getDeviceIndexOUT() const;
	void setDeviceOUT(const int index);

	std::weak_ptr<juce::MidiInput> getDeviceIN() const;
	std::weak_ptr<juce::MidiOutput> getDeviceOUT() const; 


	void setVolume(const float vValue);
	float getVolume() const;
	void setReverb(const float rValue) ;
	float getReverb() const;
	int get_minNote() const;
	int get_maxNote() const;
	void set_minNote(const int minNoteReceived);
	void set_maxNote(int maxNoteReceived);
	void changeVolumeInstrument();
	void changeReverbInstrument();

	const juce::String& get_identifier() const;
	juce::String extractPID(const juce::String& identifierReceived);
	juce::String extractVID(const juce::String& indentifierReceived);

private:
	friend class MidiHandler;
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

class MidiHandler :public juce::MidiInputCallback, public DisplayListener 
{
public:
	std::function<void()> onStartNoteSetting;
	std::function<void()> onEndNoteSetting;

	MidiHandler(MidiDevice& device);
	~MidiHandler();
	void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

	void handlePlayableRange(const juce::String& vid, const juce:: String& pid);


	void addListener(MidiHandlerListener* listener) { listeners.add(listener); }
	void removeListener(MidiHandlerListener* listener) { listeners.remove(listener); }
	void getNextMidiBlock(juce::MidiBuffer& destBuffer, int startSample, int numSamples);

	void noteOnKeyboard(int note, juce::uint8 velocity);
	void noteOffKeyboard(int note, juce::uint8 velocity);
	void allOffKeyboard();
	void setProgramNumber(int toSetNumber, const juce::String& name="");
	int getProgramNumber();
	void set_start_end_notes(int start, int end);
	void set_left_right_bounds(int left, int right);

	void playBackSettingsChanged(const PlayBackSettings& settings) override;

private:
	//void sendCCifChanged(int ccNumber, int value, int& lastSentValue);

	void applyInstrumentPreset(int programNumber, std::vector<std::pair<int, int>> ccValues);

	void setPlayableRange(int nrKeys);
	juce::ListenerList<MidiHandlerListener> listeners;

	MidiDevice& midiDevice;
	MidiDevicesDataBase dataBase;
	InstrumentHandler instrumentHandler{};

	bool receivedValidNote = false;
	juce::MidiBuffer incomingMidiMessages;
	juce::CriticalSection midiMutex;
	int programNumber = 0;
	int startNoteSetting=-1;
	int endNoteSetting=-1;
	int leftHandBoundSetting = -1;
	int rightHandBoundSetting = -1;


	int lastCC91=-1; //reverb
	int lastCC74 = -1;  //brightness
	int lastCC11 = -1; //expression
	int lastCC93 = -1; //chours
	int lastCC71 = -1; //resonance
	int lastCC64 = -1; //sustain pedal;


};