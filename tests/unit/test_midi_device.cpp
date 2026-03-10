#include <juce_core/juce_core.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiHandler.h"

class MidiDeviceTest : public juce::UnitTest
{
public:
    MidiDeviceTest() : juce::UnitTest("MidiDevice", "Unit") {}

    void runTest() override
    {
        // ---- Constructor Defaults ----

        beginTest("Constructor - default volume and reverb");
        {
            MidiDevice device;
            expect(device.getVolume(1) == 50.0f);
            expect(device.getVolume(16) == 50.0f);
            expect(device.getReverb(1) == 50.0f);
            expect(device.getReverb(16) == 50.0f);
        }

        beginTest("Constructor - default note range");
        {
            MidiDevice device;
            expect(device.get_minNote() == 0);
            expect(device.get_maxNote() == 127);
        }

        beginTest("Constructor - device not open by default");
        {
            MidiDevice device;
            expect(device.isOpenIN() == false);
            expect(device.isOpenOUT() == false);
        }

        beginTest("Constructor - default device indices");
        {
            MidiDevice device;
            expect(device.getDeviceIndexIN() == 0);
            expect(device.getDeviceIndexOUT() == 0);
        }

        // ---- min/max Note ----

        beginTest("set_minNote and get_minNote");
        {
            MidiDevice device;
            device.set_minNote(36);
            expect(device.get_minNote() == 36);
            device.set_minNote(0);
            expect(device.get_minNote() == 0);
        }

        beginTest("set_maxNote and get_maxNote");
        {
            MidiDevice device;
            device.set_maxNote(96);
            expect(device.get_maxNote() == 96);
            device.set_maxNote(108);
            expect(device.get_maxNote() == 108);
        }

        beginTest("getNrKeysAfterInitialized - full range");
        {
            MidiDevice device;
            // default: min=0, max=127 => 128 keys
            expect(device.getNrKeysAfterInitialized() == 128);
        }

        beginTest("getNrKeysAfterInitialized - custom range");
        {
            MidiDevice device;
            device.set_minNote(36);
            device.set_maxNote(96);
            expect(device.getNrKeysAfterInitialized() == 61);
        }

        // ---- Volume per channel ----

        beginTest("setVolume / getVolume - channel 1");
        {
            MidiDevice device;
            device.setVolume(75.0f, 1);
            expect(device.getVolume(1) == 75.0f);
            expect(device.getVolume(16) == 50.0f);  // channel 16 unchanged
        }

        beginTest("setVolume / getVolume - channel 16");
        {
            MidiDevice device;
            device.setVolume(30.0f, 16);
            expect(device.getVolume(16) == 30.0f);
            expect(device.getVolume(1) == 50.0f);  // channel 1 unchanged
        }

        beginTest("getVolume - unknown channel returns 50.0");
        {
            MidiDevice device;
            expect(device.getVolume(5) == 50.0f);
        }

        // ---- Reverb per channel ----

        beginTest("setReverb / getReverb - channel 1");
        {
            MidiDevice device;
            device.setReverb(80.0f, 1);
            expect(device.getReverb(1) == 80.0f);
            expect(device.getReverb(16) == 50.0f);
        }

        beginTest("setReverb / getReverb - channel 16");
        {
            MidiDevice device;
            device.setReverb(25.0f, 16);
            expect(device.getReverb(16) == 25.0f);
            expect(device.getReverb(1) == 50.0f);
        }

        beginTest("getReverb - unknown channel returns 50.0");
        {
            MidiDevice device;
            expect(device.getReverb(7) == 50.0f);
        }

        // ---- Device Index ----

        beginTest("setDeviceIN / getDeviceIndexIN");
        {
            MidiDevice device;
            device.setDeviceIN(3);
            expect(device.getDeviceIndexIN() == 3);
        }

        beginTest("setDeviceOUT / getDeviceIndexOUT");
        {
            MidiDevice device;
            device.setDeviceOUT(5);
            expect(device.getDeviceIndexOUT() == 5);
        }

        // ---- VID / PID / Name ----

        beginTest("setVID / getVID");
        {
            MidiDevice device;
            device.setVID("07cf");
            expect(device.getVID() == "07cf");
        }

        beginTest("setPID / getPID");
        {
            MidiDevice device;
            device.setPID("6803");
            expect(device.getPID() == "6803");
        }

        beginTest("setDeviceName / getName");
        {
            MidiDevice device;
            device.setDeviceName("CT s-300");
            expect(device.getName() == "CT s-300");
        }

        // ---- extractPID ----

        beginTest("extractPID - valid identifier");
        {
            MidiDevice device;
            juce::String identifier = "\\\\?\\SWD#MMDEVAPI#MIDII_VID&pid_6803&SomethingElse";
            expect(device.extractPID("some_prefix_pid_6803_suffix") == "6803");
        }

        beginTest("extractPID - standard USB identifier");
        {
            MidiDevice device;
            expect(device.extractPID("usb_vid_07cf&pid_ABCD&rev_0100") == "ABCD");
        }

        beginTest("extractPID - no pid_ in identifier");
        {
            MidiDevice device;
            expect(device.extractPID("no_product_id_here") == "");
        }

        beginTest("extractPID - empty string");
        {
            MidiDevice device;
            expect(device.extractPID("") == "");
        }

        // ---- extractVID ----

        beginTest("extractVID - valid identifier");
        {
            MidiDevice device;
            expect(device.extractVID("usb_vid_07cf&pid_6803") == "07cf");
        }

        beginTest("extractVID - no vid_ in identifier");
        {
            MidiDevice device;
            expect(device.extractVID("no_vendor_id_here") == "");
        }

        beginTest("extractVID - empty string");
        {
            MidiDevice device;
            expect(device.extractVID("") == "");
        }

        // ---- deviceOpenIN invalid index ----

        beginTest("deviceOpenIN - negative index returns false");
        {
            MidiDevice device;
            expect(device.deviceOpenIN(-1, nullptr) == false);
            expect(device.isOpenIN() == false);
        }

        beginTest("deviceOpenIN - out of range index returns false");
        {
            MidiDevice device;
            expect(device.deviceOpenIN(999, nullptr) == false);
            expect(device.isOpenIN() == false);
        }

        // ---- deviceOpenOUT invalid index ----

        beginTest("deviceOpenOUT - negative index returns false");
        {
            MidiDevice device;
            expect(device.deviceOpenOUT(-1) == false);
            expect(device.isOpenOUT() == false);
        }

        beginTest("deviceOpenOUT - out of range index returns false");
        {
            MidiDevice device;
            expect(device.deviceOpenOUT(999) == false);
            expect(device.isOpenOUT() == false);
        }
    }
};

static MidiDeviceTest midiDeviceTest;
