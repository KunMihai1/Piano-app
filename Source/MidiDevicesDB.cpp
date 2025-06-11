/*
  ==============================================================================

    midiDevicesDB.cpp
    Created: 26 Mar 2025 2:01:51am
    Author:  Kisuke

  ==============================================================================
*/

#include "midiDevicesDB.h"

MidiDevicesDataBase::MidiDevicesDataBase()
{
	jsonEnsureExistance();
	loadJsonFile();
}

MidiDevicesDataBase::~MidiDevicesDataBase()
{
}

void MidiDevicesDataBase::populateInitialDevices()
{
	addDeviceJson("07cf", "6803", "CT s-300", 61);
	addDeviceJson("0667", "1310", "Korg PA100", 61);
	addDeviceJson("0667", "1311", "Korg PA200", 61);
	addDeviceJson("0667", "1312", "Korg PA300", 61);
	addDeviceJson("0667", "1313", "Korg PA4X", 61);
	addDeviceJson("0667", "1314", "Korg PA4X", 76);
	addDeviceJson("0667", "1315", "Korg PA4X", 88);
	addDeviceJson("0667", "1316", "Korg PA5X", 61);
	addDeviceJson("0667", "1317", "Korg PA5X", 76);
	addDeviceJson("0667", "1318", "Korg PA5X", 88);
	addDeviceJson("0582", "7010", "Roland FP-30", 88);
	addDeviceJson("0582", "7011", "Roland FP-60", 88);
	addDeviceJson("0582", "10B0", "Roland A-49", 49);
	addDeviceJson("0582", "10b1", "Roland A-61", 61);
	addDeviceJson("0582", "10b2", "Roland A-88", 88);
	addDeviceJson("0582", "10f0", "Roland RD-2000", 88);
	addDeviceJson("0582", "1000", "Roland Juno-DS61", 61);
	addDeviceJson("0582", "1001", "Roland Juno-DS88", 88);
	addDeviceJson("0582", "7060", "Roland FA-06", 61);
	addDeviceJson("0582", "7070", "Roland FA-08", 88);
	addDeviceJson("0582", "1110", "Roland GO:KEYS", 61);
	addDeviceJson("0582", "1120", "Roland GO:PIANO", 61);
	addDeviceJson("09c4", "0100", "Casio Privia PX-160", 88);
	addDeviceJson("09c4", "0101", "Casio Privia PX-860", 88);
	addDeviceJson("09c4", "0111", "Casio CTK-3500", 61);
	addDeviceJson("09c4", "0122", "Casio WK-8000", 88);
	addDeviceJson("09c4", "0131", "Casio LK-280", 61);
	addDeviceJson("0763", "0001", "M-Audio Keystation 88 Mk3", 88);
	addDeviceJson("0763", "1010", "M-Audio Oxygen 61 Mk5", 61);
	addDeviceJson("1235", "1000", "Novation Launchkey 49 Mk3", 49);
}

void MidiDevicesDataBase::jsonEnsureExistance()
{
	juce::File jsonFile = getJsonFile();
	if (!jsonFile.existsAsFile())
	{
		juce::DynamicObject* rootObject = new juce::DynamicObject();
		juce::Array<juce::var> devicesArray;

		rootObject->setProperty("devices", juce::Array<juce::var>{});
		jsonData = juce::var(rootObject);
		populateInitialDevices();
		saveJsonFile();
	}
}

void MidiDevicesDataBase::loadJsonFile()
{
	juce::File jsonFile = getJsonFile();
	if (!jsonFile.existsAsFile())
		return;
	juce::String jsonString = jsonFile.loadFileAsString();
	jsonData = juce::JSON::parse(jsonString);
	if (!jsonData.isObject())
		jsonData = juce::var{};
}

void MidiDevicesDataBase::saveJsonFile()
{
	juce::File jsonFile = getJsonFile();
	jsonFile.replaceWithText(juce::JSON::toString(jsonData));
}

void MidiDevicesDataBase::addDeviceJson(const juce::String& vid, const juce::String& pid, const juce::String& name, int numKeys)
{
	if (!jsonData.isObject())
		return;

	juce::DynamicObject* rootObject = jsonData.getDynamicObject();

	if (!rootObject)
		return;

	juce::var devicesVar = rootObject->getProperty("devices");
	if (!devicesVar.isArray())
	{
		juce::Array<juce::var> emptyArray;
		rootObject->setProperty("devices", emptyArray);
		devicesVar = rootObject->getProperty("devices");
	}
	auto* devicesArrayPtr = devicesVar.getArray();
	if (devicesArrayPtr == nullptr) {
		DBG("Devices array pointer is null!");
		return;
	}
	juce::Array<juce::var>& devicesArray = *devicesArrayPtr;

	for (const auto& device : devicesArray)
	{
		if (device.isObject())
		{
			auto* obj = device.getDynamicObject();
			if (obj->getProperty("vid").toString() == vid && obj->getProperty("pid").toString() == pid)
				return;
		}
	}

	juce::DynamicObject* newDevice = new juce::DynamicObject{};
	newDevice->setProperty("vid", vid);
	newDevice->setProperty("pid", pid);
	newDevice->setProperty("name", name);
	newDevice->setProperty("keys", numKeys);
	devicesArray.add(newDevice);
	saveJsonFile();
}

int MidiDevicesDataBase::getNrKeysPidVid(const juce::String& vid, const juce::String& pid)
{
	if (jsonData.isObject())
	{
		auto* rootObject = jsonData.getDynamicObject();
		juce::var devicesVar = rootObject->getProperty("devices");

		auto devicesArrayPtr = devicesVar.getArray();
		if (devicesArrayPtr == nullptr)
			return -1;

		juce::Array<juce::var>& devicesArray = *devicesArrayPtr;

		for (const auto& device : devicesArray)
		{
			auto* obj = device.getDynamicObject();
			if (device.isObject())
			{
				if (obj->getProperty("vid").toString() == vid && obj->getProperty("pid").toString() == pid)
					return static_cast<int>(obj->getProperty("keys"));
			}
		}
	}
	return -1;
}

juce::File MidiDevicesDataBase::getAppDataFolder()
{
	juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
		.getChildFile("Piano Synth2");

	if (!appDataFolder.exists())
		appDataFolder.createDirectory();

	return appDataFolder;
}

juce::File MidiDevicesDataBase::getJsonFile()
{
	return getAppDataFolder().getChildFile("myDevices.json");
}