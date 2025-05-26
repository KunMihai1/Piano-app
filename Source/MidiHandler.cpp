#include "MidiHandler.h"
#include "InstrumentHandler.h"
#include "MidiDevicesDB.h"

MidiDevice::MidiDevice() : reverb{ 50.0f }, volume{ 50.0f }, currentDeviceIDin{ 0 }, currentDeviceIDout{ 0 }, identifier{ "" }, minNote{ 0 }, maxNote{ 127 } {
	this->currentDevicesIN.push_back("PC Keyboard");
}

MidiDevice::~MidiDevice()
{
	this->CachedDevicesIN.clear();
	this->currentDevicesIN.clear();
	this->CachedDevicesOUT.clear();
	this->currentDevicesOUT.clear();
	if (this -> currentDeviceUSEDin)
	{
		this->currentDeviceUSEDin->stop();
		currentDeviceUSEDin.reset();
	}
	if (this->currentDeviceUSEDout)
		this->currentDeviceUSEDout.reset();
}

void MidiDevice::getAvailableDevicesMidiIN(std::vector<std::string>& devices)
{
	refreshDeviceList();
	devices = currentDevicesIN;
	if (devices.empty())
	{
		devices.clear();
		devices.push_back("No devices found!");
	}
}

void MidiDevice::getAvailableDevicesMidiOUT(std::vector<std::string>& devices) {
	refreshDeviceList(1);
	devices = currentDevicesOUT;
	if (devices.empty())
	{
		devices.clear();
		devices.push_back("No devices found!");
	}
}

juce::String MidiDevice::getDeviceNameBasedOnIndex(int Index, int choice)
{
	if (choice == 0)
	{
		if (Index >= 0 && Index < this->currentDevicesIN.size())
			return juce::String(this->currentDevicesIN[Index]);
	}
	else if (choice == 1)
	{
		if (Index >= 0 && Index < this->currentDevicesOUT.size())
			return juce::String(this->currentDevicesOUT[Index]);
	}
	return juce::String();
}

juce::String MidiDevice::getDeviceIdentifierBasedOnIndex(int Index, int choice)
{
	if (choice == 0)
	{
		if ((Index >= 0 && Index < this->currentDevicesIN.size()))
		{
			for (int i = 0; i < this->CachedDevicesIN.size(); i++)
			{
				if (i == Index)
					return juce::String(this->CachedDevicesIN[i].identifier);
			}
		}
	}
	else if (choice == 1)
	{
		if ((Index >= 0 && Index < this->currentDevicesOUT.size()))
		{
			for (int i = 0; i < this->CachedDevicesOUT.size(); i++)
			{
				if (i == Index)
					return juce::String(this->CachedDevicesOUT[i].identifier);
			}
		}
	}
	return juce::String();
}


void MidiDevice::refreshDeviceList(int choice)
{
	if (choice == 0)
	{
		juce::Array<juce::MidiDeviceInfo> newDevices = juce::MidiInput::getAvailableDevices();
		if (newDevices != this->CachedDevicesIN)
		{
			this->CachedDevicesIN = newDevices;
			this->currentDevicesIN.clear();
			for (int i = 0; i < newDevices.size(); i++)
			{
				if (newDevices[i].name.isNotEmpty())
					this->currentDevicesIN.push_back(newDevices[i].name.toStdString());
			}
			this->devicesChange = true;
			this->currentDevicesIN.push_back("PC Keyboard");
		}
		else
			this->devicesChange = false;

	}
	else if (choice == 1)
	{
		juce::Array<juce::MidiDeviceInfo> newDevices = juce::MidiOutput::getAvailableDevices();
		if (newDevices != this->CachedDevicesOUT)
		{
			this->CachedDevicesOUT = newDevices;
			this->currentDevicesOUT.clear();
			for (int i = 0; i < newDevices.size(); i++)
			{
				if (newDevices[i].name.isNotEmpty())
					this->currentDevicesOUT.push_back(newDevices[i].name.toStdString());
			}
			this->devicesChange = true;
		}
		else
			this->devicesChange = false;
	}
}

void MidiDevice::set_minNote(const int minNoteReceived)
{
	this->minNote = minNoteReceived;
}

void MidiDevice::set_maxNote(const int maxNoteReceived)
{
	this->maxNote = maxNoteReceived;
}

juce::String MidiDevice::extractPID(const juce::String& identifierReceived)
{
	int pidStart = identifierReceived.indexOf("pid_");
	if (pidStart != -1)
		return identifierReceived.substring(pidStart + 4, pidStart + 8);
	return juce::String();
}

juce::String MidiDevice::extractVID(const juce::String& identifierReceived)
{
	int vidStart = identifierReceived.indexOf("vid_");
	if (vidStart != -1)
		return identifierReceived.substring(vidStart + 4, vidStart + 8);
	return juce::String();
}



bool MidiDevice::deviceOpenIN(int DeviceIndex, juce::MidiInputCallback* CallBack)
{
	if (DeviceIndex < 0 || DeviceIndex >= this->currentDevicesIN.size())
	{
		this->isdeviceOpenIN = false;
		return false;
	}
	juce::String identifierReceived = getDeviceIdentifierBasedOnIndex(DeviceIndex);
	this->currentDeviceUSEDin = juce::MidiInput::openDevice(identifierReceived, CallBack);
	
	if (currentDeviceUSEDin != nullptr)
	{
		currentDeviceUSEDin->start();
		this->identifier = identifierReceived;
		isdeviceOpenIN = true;
		return true;
	}
	this->isdeviceOpenIN = false;
	return false;
}

bool MidiDevice::deviceOpenOUT(int DeviceIndex) {
	if (DeviceIndex < 0 || DeviceIndex >= this->currentDevicesOUT.size())
	{
		this->isdeviceOpenOUT = false;
		return false;
	}
	juce::String identifierReceived = getDeviceIdentifierBasedOnIndex(DeviceIndex, 1);
	this->currentDeviceUSEDout = juce::MidiOutput::openDevice(identifierReceived);
	if (currentDeviceUSEDout != nullptr)
	{
		this->isdeviceOpenOUT = true;
		return true;
	}
	this->isdeviceOpenOUT = false;
	return false;
}

bool MidiDevice::isOpenIN() const
{
	return this->isdeviceOpenIN;
}

bool MidiDevice::isOpenOUT() const
{
	return this->isdeviceOpenOUT;
}

void MidiDevice::deviceCloseIN()
{
	if (this->currentDeviceUSEDin != nullptr)
	{
		this->currentDeviceUSEDin->stop();
		this->currentDeviceUSEDin = nullptr;
		this->isdeviceOpenIN = false;
	}
}

void MidiDevice::deviceCloseOUT()
{
	if (this->currentDeviceUSEDout != nullptr)
	{
		this->currentDeviceUSEDout = nullptr;
		this->isdeviceOpenOUT = false;
	}
}

int MidiDevice::getDeviceIndexIN() const
{
	return this->currentDeviceIDin;
}

void MidiDevice::setDeviceIN(const int index) {
	this->currentDeviceIDin = index;
}

int MidiDevice::getDeviceIndexOUT() const {
	return this->currentDeviceIDout;
}

void MidiDevice::setDeviceOUT(const int index) {
	this->currentDeviceIDout = index;
}

const juce::MidiInput& MidiDevice::getDeviceIN() const
{
	return *this->currentDeviceUSEDin;
}

juce::MidiOutput* MidiDevice::getDeviceOUT()
{
	return currentDeviceUSEDout.get();
}

void MidiDevice::setVolume(const float vValue) 
{
	this->volume = vValue;
}

float MidiDevice::getVolume() const
{
	return this->volume;
}

void MidiDevice::setReverb(const float rValue) 
{
	this->reverb = rValue;
}

float MidiDevice::getReverb() const
{
	return this->reverb;
}

int MidiDevice::get_minNote() const
{
	return this->minNote;
}

int MidiDevice::get_maxNote() const
{
	return this->maxNote;
}

void MidiDevice::changeVolumeInstrument()
{
	float volume1 = this->getVolume();
	float normalizedVolume = juce::jlimit(0.0f, 1.0f, volume1 / 100.0f);
	juce::uint8 midiValue = static_cast<juce::uint8>(juce::jlimit(0, 127, int(normalizedVolume * 127.0f)));

	juce::MidiMessage volumeMessage = juce::MidiMessage::controllerEvent(1, 7, midiValue);
	//DBG("Changed to:" + juce::String(midiValue));
	this->currentDeviceUSEDout->sendMessageNow(volumeMessage);
}

void MidiDevice::changeReverbInstrument()
{
	float reverb1 = this->getReverb();
	float normalizedReverb = juce::jlimit(0.0f, 1.0f, reverb1 / 100.0f);
	juce::uint8 midiValue= static_cast<juce::uint8>(juce::jlimit(0, 127, int(normalizedReverb * 127.0f)));
	juce::MidiMessage reverbMessage = juce::MidiMessage::controllerEvent(1, 91, midiValue);
	this->currentDeviceUSEDout->sendMessageNow(reverbMessage);
}

const juce::String& MidiDevice::get_identifier() const
{
	return this->identifier;
}

MidiHandler::MidiHandler(MidiDevice& device) : midiDevice{ device }, dataBase{} {
}

MidiHandler::~MidiHandler()
{
}

void MidiHandler::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
	const juce::ScopedLock lock(midiMutex);

	juce::MidiMessage processedMessage = message;
	DBG("Incoming MIDI: " + message.getDescription());
	if (message.isNoteOn())
	{
		int note = message.getNoteNumber();
		float velocity = message.getFloatVelocity();

		float volume = this->midiDevice.getVolume();
		//DBG("VOLUME="+ juce::String(volume));
		float scaledVelocity = juce::jlimit(0.0f, 1.0f, velocity * volume);

		juce::uint8 velocityByte = juce::MidiMessage::floatValueToMidiByte(scaledVelocity);

		//sendCCifChanged(91, 80, lastCC91);
		//sendCCifChanged(74, 100, lastCC74);
;		//sendCCifChanged(11, midiDevice.getReverb(), lastCC11);
		//sendCCifChanged(93, 70, lastCC93);
		//sendCCifChanged(71, 30, lastCC71);

		if (auto midiOut = midiDevice.getDeviceOUT())
		{
			midiOut->sendMessageNow(juce::MidiMessage::pitchWheel(1, 0x2000));
			midiOut->sendMessageNow(juce::MidiMessage::noteOn(1, note, velocityByte));
		}

		listeners.call(&MidiHandlerListener::noteOnReceived, note);
		//listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::controllerEvent(1, 91, 80));
		//listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::controllerEvent(1, 74, 100));
		listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOn(1, note, velocityByte));

	}

	incomingMidiMessages.addEvent(processedMessage, 0);

	if (message.isNoteOff())
	{
		int note = message.getNoteNumber();
		juce::uint8 velocityByte = juce::MidiMessage::floatValueToMidiByte(message.getFloatVelocity());

		if (auto midiOut = midiDevice.getDeviceOUT())
		{
			midiOut->sendMessageNow(juce::MidiMessage::noteOff(1, note, velocityByte));
		}

		listeners.call(&MidiHandlerListener::noteOffReceived, note);
		listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOff(1, note, velocityByte));
	}
}

void MidiHandler::getNextMidiBlock(juce::MidiBuffer& destBuffer, int startSample, int numSamples) {
	DBG("getNextMidiBlock called");
	const juce::ScopedLock lock(midiMutex);
	destBuffer.addEvents(incomingMidiMessages, startSample, numSamples, 0);
	incomingMidiMessages.clear();
	DBG("getNextMidiBlock: " << destBuffer.getNumEvents());

}

void MidiHandler::noteOnKeyboard(int note, juce::uint8 velocity) {
	auto midiOut = midiDevice.getDeviceOUT();
	if (midiOut)
	{
		midiOut->sendMessageNow(juce::MidiMessage::noteOn(1, note, velocity));
	}
	listeners.call(&MidiHandlerListener::noteOnReceived, note);
	listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOn(1, note, velocity));
}

void MidiHandler::noteOffKeyboard(int note, juce::uint8 velocity) {
	auto midiOut = midiDevice.getDeviceOUT();
	if (midiOut)
	{
		midiOut->sendMessageNow(juce::MidiMessage::noteOff(1, note,velocity));

	}
	listeners.call(&MidiHandlerListener::noteOffReceived,note);
	listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOff(1, note));
}

void MidiHandler::allOffKeyboard()
{
	for (int i = 0; i < 128; i++)
		midiDevice.currentDeviceUSEDout->sendMessageNow(juce::MidiMessage::noteOff(1, i));
}

void MidiHandler::setProgramNumber(int toSetNumber, const juce::String& name) {
	this->programNumber = toSetNumber;
	auto midiOut = this->midiDevice.getDeviceOUT();
	if (midiOut)
	{

		const auto& preset = instrumentHandler.getPreset(programNumber);

		applyInstrumentPreset(programNumber, preset);
		juce::Thread::sleep(30);
	}
}

int MidiHandler::getProgramNumber()
{
	return programNumber;
}

void MidiHandler::handlePlayableRange(const juce::String& vid, const juce::String& pid)
{
	int nrKeys = dataBase.getNrKeysPidVid(vid, pid);
	setPlayableRange(nrKeys);
}


void MidiHandler::applyInstrumentPreset(int programNumber, std::vector<std::pair<int, int>> ccValues)
{
	auto midiOut = midiDevice.getDeviceOUT();
	if (midiOut)
	{
		for (int i = 0; i < 128; ++i)
			midiOut->sendMessageNow(juce::MidiMessage::noteOff(1, i));

		midiOut->sendMessageNow(juce::MidiMessage::controllerEvent(1, 64, 0));

		midiOut->sendMessageNow(juce::MidiMessage::programChange(1, programNumber));
		listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::programChange(1, programNumber));

		for (auto& cc : ccValues)
		{
			int ccNumber = cc.first;
			int ccValue = cc.second;
			if (ccNumber == 91 && ccValue == -1)
				ccValue = midiDevice.getReverb();

			juce::MidiMessage msg = juce::MidiMessage::controllerEvent(1, ccNumber, ccValue);
			midiOut->sendMessageNow(msg);
			listeners.call(&MidiHandlerListener::handleIncomingMessage, msg);
		}
	}
}

void MidiHandler::setPlayableRange(int nrKeys)
{
	if (nrKeys == 25)
	{
		midiDevice.set_minNote(36);
		midiDevice.set_maxNote(60);
	}
	else if (nrKeys == 49)
	{
		midiDevice.set_minNote(40);
		midiDevice.set_maxNote(88);
	}
	else if (nrKeys == 61)
	{
		midiDevice.set_minNote(36);
		midiDevice.set_maxNote(96);
	}
	else if (nrKeys == 88)
	{
		midiDevice.set_minNote(21);
		midiDevice.set_maxNote(108);
	}
}
