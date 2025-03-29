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

void MidiDevicesDataBase::jsonEnsureExistance()
{
	juce::File jsonFile = getJsonFile();
	if (!jsonFile.existsAsFile())
	{
		juce::DynamicObject* rootObject = new juce::DynamicObject();
		rootObject->setProperty("devices", juce::Array<juce::var>{});
		jsonData = juce::var(rootObject);
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

void MidiDevicesDataBase::addDeviceJson(const juce::String& vid, const::juce::String& pid, const::juce::String& name, int numKeys)
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

juce::File MidiDevicesDataBase::getJsonFile()
{
	return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("myDevices.json");
}