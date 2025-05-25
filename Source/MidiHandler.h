#pragma once

#include <JuceHeader.h>
#include "MidiDevicesDB.h"
#include "InstrumentHandler.h"
#include "MidiRecordPlayer.h"

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

	const juce::MidiInput& getDeviceIN() const;
	juce::MidiOutput* getDeviceOUT();


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
	std::unique_ptr<juce::MidiInput> currentDeviceUSEDin = nullptr;
	std::unique_ptr<juce::MidiOutput> currentDeviceUSEDout = nullptr;

	bool devicesChange = false;
	bool isdeviceOpenIN = false;
	bool isdeviceOpenOUT = false;
	juce::String identifier;

	float reverb{ 0.0f };
	float volume{ 0.0f };
	int minNote, maxNote;
};

class MidiHandler :public juce::MidiInputCallback {
public:
	MidiHandler(MidiDevice& device);
	~MidiHandler();
	void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

	void handlePlayableRange(const juce::String& vid, const juce:: String& pid);

	struct Listener {
		virtual void noteOnReceived(int midiNote) = 0;
		virtual void noteOffReceived(int midiNote) = 0;
	};

	void addListener(Listener* listener) { listeners.add(listener); }
	void removeListener(Listener* listener) { listeners.remove(listener); }
	void getNextMidiBlock(juce::MidiBuffer& destBuffer, int startSample, int numSamples);

	void noteOnKeyboard(int note, juce::uint8 velocity);
	void noteOffKeyboard(int note, juce::uint8 velocity);
	void setProgramNumber(int toSetNumber);
	void setOutputRecorder(juce::MidiOutput* output);

	void startRecord();
	void stopRecord();
	void startPlayback();

private:
	void setPlayableRange(int nrKeys);
	juce::ListenerList<Listener> listeners;

	MidiDevice& midiDevice;
	MidiDevicesDataBase dataBase;
	InstrumentHandler instrumentHandler;
	MidiRecordPlayer recordPlayer{midiDevice.getDeviceOUT()};

	bool receivedValidNote = false;
	juce::MidiBuffer incomingMidiMessages;
	juce::CriticalSection midiMutex;
	int programNumber = 0;

};