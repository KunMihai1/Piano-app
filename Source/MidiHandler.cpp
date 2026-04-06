#include "MidiHandler.h"
#include "InstrumentHandler.h"
#include "MidiDevicesDB.h"

MidiDevice::MidiDevice() : currentDeviceIDin { 0 }, currentDeviceIDout{ 0 }, identifier{ "" }, minNote{ 0 }, maxNote{ 127 } {
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

std::vector<std::pair<juce::String, juce::String>> MidiDevice::getAvailableInputDevicesNameIdentifier()
{
	std::vector<std::pair<juce::String, juce::String>> vec;
	refreshDeviceListNew(vec);
	if (vec.empty())
	{
		vec.clear();
		vec.push_back({ "No devices found!",""});
	}
	return vec;
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
			this->deviceCheckedForUpdateAtLeastOnce = false;
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

void MidiDevice::refreshDeviceListNew(std::vector<std::pair<juce::String, juce::String>>& vec, int choice)
{
	if (choice == 0)
	{
		juce::Array<juce::MidiDeviceInfo> newDevices = juce::MidiInput::getAvailableDevices();
		vec.clear();
		for (int i = 0; i < newDevices.size(); i++)
		{
			if (newDevices[i].name.isNotEmpty())
				vec.push_back({ newDevices[i].name,newDevices[i].identifier });
		}

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

void MidiDevice::setVID(const juce::String& VID)
{
	this->VID = VID;
}

void MidiDevice::setPID(const juce::String& PID)
{
	this->PID = PID;
}

void MidiDevice::setDeviceName(const juce::String& name)
{
	this->name = name;
}

juce::String MidiDevice::getVID()
{
	return this->VID;
}

juce::String MidiDevice::getPID()
{
	return this->PID;
}

juce::String MidiDevice::getName()
{
	return this->name;
}

int MidiDevice::getNrInputActualDevices()
{
	return juce::MidiInput::getAvailableDevices().size();
}

int MidiDevice::getNrKeysAfterInitialized()
{
	return maxNote - minNote+1;
}

bool MidiDevice::deviceOpenIN(int DeviceIndex, juce::MidiInputCallback* CallBack)
{
	if (DeviceIndex < 0 || DeviceIndex >= this->currentDevicesIN.size())
	{
		this->isdeviceOpenIN = false;
		return false;
	}

	if (currentDeviceUSEDin != nullptr)
	{
		currentDeviceUSEDin->stop();
		currentDeviceUSEDin.reset();  
		juce::Thread::sleep(50);      
	}

	juce::String identifierReceived = getDeviceIdentifierBasedOnIndex(DeviceIndex);
	auto uniqueIn = juce::MidiInput::openDevice(identifierReceived, CallBack);
	currentDeviceUSEDin = std::shared_ptr<juce::MidiInput>(std::move(uniqueIn));
	
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
	auto uniqueOut = juce::MidiOutput::openDevice(identifierReceived);
	currentDeviceUSEDout = std::shared_ptr<juce::MidiOutput>(std::move(uniqueOut));
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

std::weak_ptr<juce::MidiInput> MidiDevice::getDeviceIN() const
{
	return this->currentDeviceUSEDin;
}

std::weak_ptr<juce::MidiOutput> MidiDevice::getDeviceOUT() const
{
	return this->currentDeviceUSEDout;
}

void MidiDevice::setVolume(const int vValue, int channel) 
{
	if (channel == 1)
		this-> firstInstrument.volume = vValue;
	else if (channel == 16)
		this->secondInstrument.volume = vValue;
}

int MidiDevice::getVolume(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.volume;
	else if (channel == 16)
		return this->secondInstrument.volume;

	return 50.0;
}

void MidiDevice::setReverb(const int rValue, int channel) 
{
	if (channel == 1)
		this->firstInstrument.reverb = rValue;
	else if (channel == 16)
		this->secondInstrument.reverb = rValue;
}

int MidiDevice::getReverb(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.reverb;
	else if (channel == 16)
		return this->secondInstrument.reverb;

	return 50.0;
}

void MidiDevice::setBrightness(const int rValue, int channel)
{
	if (channel == 1)
		this->firstInstrument.brightness = rValue;
	else if (channel == 16)
		this->secondInstrument.brightness = rValue;
}

int MidiDevice::getBrightness(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.brightness;
	else if (channel == 16)
		return this->secondInstrument.brightness;
}

void MidiDevice::setExpression(const int rValue, int channel)
{
	if (channel == 1)
		this->firstInstrument.expression = rValue;
	else if (channel == 16)
		this->secondInstrument.expression = rValue;
}

int MidiDevice::getExpression(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.expression;
	else if (channel == 16)
		return this->secondInstrument.expression;
}

void MidiDevice::setChorus(const int rValue, int channel)
{
	if (channel == 1)
		this->firstInstrument.chours = rValue;
	else if (channel == 16)
		this->secondInstrument.chours = rValue;
}

int MidiDevice::getChorus(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.chours;
	else if (channel == 16)
		return this->secondInstrument.chours;
}

void MidiDevice::setResonance(const int rValue, int channel)
{
	if (channel == 1)
		this->firstInstrument.resonance = rValue;
	else if (channel == 16)
		this->secondInstrument.resonance = rValue;
}

int MidiDevice::getResonance(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.resonance;
	else if (channel == 16)
		return this->secondInstrument.resonance;
}

void MidiDevice::setSustainToggle(const bool rValue, int channel)
{
	if (channel == 1)
		this->firstInstrument.sustainToggle = rValue;
	else if (channel == 16)
		this->secondInstrument.sustainToggle = rValue;
}

bool MidiDevice::getSustainToggle(int channel) const
{
	if (channel == 1)
		return this->firstInstrument.sustainToggle;
	else if (channel == 16)
		return this->secondInstrument.sustainToggle;
}


void MidiDevice::setAttack(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.attack = rValue;
	else if (channel == 16) secondInstrument.attack = rValue;
}

int MidiDevice::getAttack(int channel) const
{
	if (channel == 1) return firstInstrument.attack;
	else if (channel == 16) return secondInstrument.attack;
	return 0;
}


void MidiDevice::setDecay(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.decay = rValue;
	else if (channel == 16) secondInstrument.decay = rValue;
}

int MidiDevice::getDecay(int channel) const
{
	if (channel == 1) return firstInstrument.decay;
	else if (channel == 16) return secondInstrument.decay;
	return 0;
}


void MidiDevice::setRelease(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.release = rValue;
	else if (channel == 16) secondInstrument.release = rValue;
}

int MidiDevice::getRelease(int channel) const
{
	if (channel == 1) return firstInstrument.release;
	else if (channel == 16) return secondInstrument.release;
	return 0;
}


void MidiDevice::setVibrato(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.vibrato = rValue;
	else if (channel == 16) secondInstrument.vibrato = rValue;
}

int MidiDevice::getVibrato(int channel) const
{
	if (channel == 1) return firstInstrument.vibrato;
	else if (channel == 16) return secondInstrument.vibrato;
	return 0;
}



void MidiDevice::setDelay(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.delay = rValue;
	else if (channel == 16) secondInstrument.delay = rValue;
}

int MidiDevice::getDelay(int channel) const
{
	if (channel == 1) return firstInstrument.delay;
	else if (channel == 16) return secondInstrument.delay;
	return 0;
}


void MidiDevice::setPan(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.pan = rValue;
	else if (channel == 16) secondInstrument.pan = rValue;
}

int MidiDevice::getPan(int channel) const
{
	if (channel == 1) return firstInstrument.pan;
	else if (channel == 16) return secondInstrument.pan;
	return 0;
}


void MidiDevice::setDistortion(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.distortion = rValue;
	else if (channel == 16) secondInstrument.distortion = rValue;
}

int MidiDevice::getDistortion(int channel) const
{
	if (channel == 1) return firstInstrument.distortion;
	else if (channel == 16) return secondInstrument.distortion;
	return 0;
}


void MidiDevice::setFilterTrack(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.filterTrack = rValue;
	else if (channel == 16) secondInstrument.filterTrack = rValue;
}

int MidiDevice::getFilterTrack(int channel) const
{
	if (channel == 1) return firstInstrument.filterTrack;
	else if (channel == 16) return secondInstrument.filterTrack;
	return 0;
}


void MidiDevice::setTremolo(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.tremolo = rValue;
	else if (channel == 16) secondInstrument.tremolo = rValue;
}

int MidiDevice::getTremolo(int channel) const
{
	if (channel == 1) return firstInstrument.tremolo;
	else if (channel == 16) return secondInstrument.tremolo;
	return 0;
}


void MidiDevice::setRandomMod(const int rValue, int channel)
{
	if (channel == 1) firstInstrument.randomMod = rValue;
	else if (channel == 16) secondInstrument.randomMod = rValue;
}

int MidiDevice::getRandomMod(int channel) const
{
	if (channel == 1) return firstInstrument.randomMod;
	else if (channel == 16) return secondInstrument.randomMod;
	return 0;
}



int MidiDevice::get_minNote() const
{
	return this->minNote;
}

int MidiDevice::get_maxNote() const
{
	return this->maxNote;
}

void MidiDevice::sendMidiCC(int channel, int ccNumber, int value)
{
	juce::uint8 midiValue = static_cast<juce::uint8>(
		juce::jlimit(0, 127, value)
		);

	if (channel < 1 || channel > 16)
		return;

	juce::MidiMessage msg = juce::MidiMessage::controllerEvent(channel, ccNumber, midiValue);
	currentDeviceUSEDout->sendMessageNow(msg);
}

const juce::String& MidiDevice::get_identifier() const
{
	return this->identifier;
}

MidiHandler::MidiHandler(MidiDevice& device, InstrumentHandler* instrumentH) : midiDevice{ device }, instrumentHandler{instrumentH}
{
}

MidiHandler::~MidiHandler()
{
}

void MidiHandler::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
	const juce::ScopedLock lock(midiMutex);

	juce::MidiMessage processedMessage = message;

	if (message.isNoteOn())
	{
		int note = message.getNoteNumber();

		setCorrectChannelBasedOnHand(note);
		int transposedNote = juce::jlimit(0, 127, note + transposeValue);

		float velocity = message.getFloatVelocity();

		float volume = this->midiDevice.getVolume(message.getChannel());
		//DBG("VOLUME="+ juce::String(volume));
		float scaledVelocity = juce::jlimit(0.0f, 1.0f, velocity * volume);

		juce::uint8 velocityByte = juce::MidiMessage::floatValueToMidiByte(scaledVelocity);

		int ok = 0;

		if (auto midiOut = midiDevice.getDeviceOUT().lock())
		{
			if (note != this->startNoteSetting && note != this->endNoteSetting)
			{
				ok = 1;
				//midiOut->sendMessageNow(juce::MidiMessage::noteOn(2, note+9, velocityByte));
				midiOut->sendMessageNow(juce::MidiMessage::pitchWheel(channel, 0x2000));
				midiOut->sendMessageNow(juce::MidiMessage::noteOn(channel, transposedNote, velocityByte));
				//midiOut->sendMessageNow(juce::MidiMessage::noteOn(2, note+10, velocityByte));
			}
			else if (note == this->startNoteSetting)
			{
				if (onStartNoteSetting)
					onStartNoteSetting();
			}
			else if (note == this->endNoteSetting)
			{
				if (onEndNoteSetting)
					onEndNoteSetting();
			}
		}

		if (ok)
		{
			listeners.call(&MidiHandlerListener::noteOnReceived, note);
			//listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::controllerEvent(1, 91, 80));
			//listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::controllerEvent(1, 74, 100));
			listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOn(channel, note, velocityByte));
		}

	}

	if (message.isNoteOff())
	{
		int note = message.getNoteNumber();
		setCorrectChannelBasedOnHand(note);
		int transposedNote = juce::jlimit(0, 127, note + transposeValue);
		juce::uint8 velocityByte = juce::MidiMessage::floatValueToMidiByte(message.getFloatVelocity());

		if (auto midiOut = midiDevice.getDeviceOUT().lock())
		{
			midiOut->sendMessageNow(juce::MidiMessage::noteOff(channel, transposedNote, velocityByte));
		}

		listeners.call(&MidiHandlerListener::noteOffReceived, note);
		listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOff(channel, note, velocityByte));
	}

	incomingMidiMessages.addEvent(processedMessage, 0);
}

void MidiHandler::getNextMidiBlock(juce::MidiBuffer& destBuffer, int startSample, int numSamples) {
	//DBG("getNextMidiBlock called");
	const juce::ScopedLock lock(midiMutex);
	destBuffer.addEvents(incomingMidiMessages, startSample, numSamples, 0);
	incomingMidiMessages.clear();
	//DBG("getNextMidiBlock: " << destBuffer.getNumEvents());

}

void MidiHandler::noteOnKeyboard(int note, juce::uint8 velocity) {
	int ok = 0;
	setCorrectChannelBasedOnHand(note);
	int transposedNote = juce::jlimit(0, 127, note + transposeValue);
	if (auto midiOut = midiDevice.getDeviceOUT().lock())
	{
		//here we need to add and if the note received is by chance, start note or end note, do to the according things.
		//DBG("The note is" + juce::String(note) + " the setting one is:" + juce::String(startNoteSetting));
		if (note != this->startNoteSetting && note!=this->endNoteSetting)
		{
			ok = 1;

			midiOut->sendMessageNow(juce::MidiMessage::pitchWheel(channel, 0x2000));
			midiOut->sendMessageNow(juce::MidiMessage::noteOn(channel, transposedNote, velocity));
		}
		else if(note==this->startNoteSetting)
		{
			if(onStartNoteSetting)
				onStartNoteSetting();
		}
		else if (note == this->endNoteSetting)
		{
			if (onEndNoteSetting)
				onEndNoteSetting();
		}
	}
	if (ok)
	{
		listeners.call(&MidiHandlerListener::noteOnReceived, note);
		listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOn(channel, note, velocity));
	}
}

void MidiHandler::noteOffKeyboard(int note, juce::uint8 velocity) {
	setCorrectChannelBasedOnHand(note);
	int transposedNote = juce::jlimit(0, 127, note + transposeValue);
	if (auto midiOut = midiDevice.getDeviceOUT().lock())
	{
		midiOut->sendMessageNow(juce::MidiMessage::noteOff(channel, transposedNote,velocity));

	}
	listeners.call(&MidiHandlerListener::noteOffReceived,note);
	listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::noteOff(channel, note));
} 

void MidiHandler::allOffKeyboard()
{
	for (int i = 0; i < 128; i++)
	{
		if (midiDevice.currentDeviceUSEDout == nullptr) return;

		midiDevice.currentDeviceUSEDout->sendMessageNow(juce::MidiMessage::noteOff(1, i));
		midiDevice.currentDeviceUSEDout->sendMessageNow(juce::MidiMessage::noteOff(16, i));
	}
}

void MidiHandler::setProgramNumber(int toSetNumber, const juce::String& choice) {
	if (choice.toLowerCase() == "left")
		this->programNumberLeftHand = toSetNumber;
	else if (choice.toLowerCase() == "right")
		this->programNumberRightHand = toSetNumber;

	if (auto midiOut = midiDevice.getDeviceOUT().lock())
	{
		if (instrumentHandler)
		{
			const auto& preset = instrumentHandler->getPreset(toSetNumber);

			applyInstrumentPreset(toSetNumber, preset, choice);
			juce::Thread::sleep(5);
		}
	}
}

int MidiHandler::getProgramNumberLeftHand()
{
	return programNumberLeftHand;
}

int MidiHandler::getProgramNumberRightHand()
{
	return programNumberRightHand;
}

void MidiHandler::set_start_end_notes(int start, int end)
{
	this->startNoteSetting = start;
	this->endNoteSetting = end;
}

void MidiHandler::set_left_right_bounds(int left, int right)
{
	this->leftHandBoundSetting = left;
	this->rightHandBoundSetting = right;
}

void MidiHandler::playBackSettingsChanged(const PlayBackSettings& settings)
{
	set_start_end_notes(settings.startNote, settings.endNote);
	set_left_right_bounds(settings.leftHandBound, settings.rightHandBound);
}

void MidiHandler::playBackSettingsTransposeChanged(int transposeValue)
{
	this->transposeValue = transposeValue;
	allOffKeyboard();
}

void MidiHandler::setCorrectChannelBasedOnHand(int note)
{
	if (rightHandBoundSetting != -1 && note >= rightHandBoundSetting)
		this->channel = 16;
	else this->channel = 1;
}

int MidiHandler::handlePlayableRange(const juce::String& vid, const juce::String& pid, int nrKeys, bool isKeyboardInput)
{
	
	int isK = isKeyboardInput;
	if (nrKeys<0 && !isKeyboardInput)
	{
		if (onAddCallBack)
		{
			onAddCallBack(vid, pid, [this, vid, pid](const juce::String& name, int keys)
				{
					setPlayableRange(keys);
				});
		}
		return -1;
	}
	else {
		setPlayableRange(nrKeys);
		return 1;
	}
}


void MidiHandler::applyInstrumentPreset(int programNumber, std::vector<std::pair<int, int>> ccValues, const juce::String& choice)
{
	int channel = 1;
	if (choice.toLowerCase() == "right")
		channel = 16;
	if (auto midiOut = midiDevice.getDeviceOUT().lock())
	{
		for (int i = 0; i < 128; ++i)
			midiOut->sendMessageNow(juce::MidiMessage::noteOff(channel, i));

		midiOut->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 64, 0));
		//juce::MidiMessage localOff = juce::MidiMessage::controllerEvent(1, 122, 0);
		//midiOut->sendMessageNow(localOff);

		midiOut->sendMessageNow(juce::MidiMessage::programChange(channel, programNumber));
		listeners.call(&MidiHandlerListener::handleIncomingMessage, juce::MidiMessage::programChange(channel, programNumber));

		for (auto& cc : ccValues)
		{
			int ccNumber = cc.first;
			int ccValue = cc.second;
			if (ccNumber == 91 && ccValue == -1)
				ccValue = midiDevice.getReverb(channel);

			juce::MidiMessage msg = juce::MidiMessage::controllerEvent(channel, ccNumber, ccValue);
			midiOut->sendMessageNow(msg);
			listeners.call(&MidiHandlerListener::handleIncomingMessage, msg);
		}
	}

	//applying some of these will see for keyboard gs synth so it actually improves something because these take effect
	// Mod Wheel (CC 1) - moderate
	//auto msg = juce::MidiMessage::controllerEvent(1, 1, 64);
	//midiOut->sendMessageNow(msg);
	//listeners.call(&MidiHandlerListener::handleIncomingMessage, msg);


	// Pan (CC 10) - center pan
	{
		//auto msg = juce::MidiMessage::controllerEvent(1, 10, 64);
		//midiOut->sendMessageNow(msg);
		//listeners.call(&MidiHandlerListener::handleIncomingMessage, msg);
	}


	// Aftertouch (Channel Pressure) - moderate
	{
		//auto msg = juce::MidiMessage::channelPressureChange(1, 80);
		//midiOut->sendMessageNow(msg);
		//listeners.call(&MidiHandlerListener::handleIncomingMessage, msg);
	}

	// Pitch Bend - neutral (8192)
	{
		//auto msg = juce::MidiMessage::pitchWheel(1, 8192);
		//midiOut->sendMessageNow(msg);
		//listeners.call(&MidiHandlerListener::handleIncomingMessage, msg);
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
