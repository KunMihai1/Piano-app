#include <juce_core/juce_core.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiHandler.h"

class MidiDeviceHWTest : public juce::UnitTest
{
public:
    MidiDeviceHWTest() : juce::UnitTest("MidiDevice_HW", "Integration") {}

    void runTest() override
    {
        detectDevices();

        testInputDeviceDiscovery();
        testOutputDeviceDiscovery();
        testGetDeviceNameInput();
        testGetDeviceNameOutput();
        testGetDeviceIdentifier();

        testOpenInput();
        testOpenInputTwice();
        testCloseInputSafe();

        testOpenOutput();
        testCloseOutputSafe();

        testReopenInput();

        testChangeVolume();
        testChangeReverb();

        testNameIdentifierPairs();
    }

private:
    struct DummyCallback : public juce::MidiInputCallback {
        void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override {}
    };

    std::vector<std::string> inputDevices;
    std::vector<std::string> outputDevices;
    bool hasInputDevice = false;
    bool hasOutputDevice = false;
    int inputDeviceIndex = -1;
    int outputDeviceIndex = -1;

    void detectDevices()
    {
        MidiDevice hwDevice;
        hwDevice.getAvailableDevicesMidiIN(inputDevices);

        for (int i = 0; i < (int)inputDevices.size(); ++i)
        {
            juce::String name(inputDevices[i]);
            if (name != "PC Keyboard" && name != "No devices found!")
            {
                hasInputDevice = true;
                inputDeviceIndex = i;
                break;
            }
        }

        hwDevice.getAvailableDevicesMidiOUT(outputDevices);

        for (int i = 0; i < (int)outputDevices.size(); ++i)
        {
            juce::String name(outputDevices[i]);
            if (name != "No devices found!")
            {
                hasOutputDevice = true;
                outputDeviceIndex = i;
                break;
            }
        }
    }

    void testInputDeviceDiscovery()
    {
        beginTest("[HW] getAvailableDevicesMidiIN - finds at least one device");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else { expect(inputDevices.size() > 1); }
    }

    void testOutputDeviceDiscovery()
    {
        beginTest("[HW] getAvailableDevicesMidiOUT - finds at least one device");
        if (!hasOutputDevice) { logMessage("SKIPPED - no MIDI output device connected"); }
        else { expect(outputDevices.size() >= 1); }
    }

    void testGetDeviceNameInput()
    {
        beginTest("[HW] getDeviceNameBasedOnIndex - returns non-empty name for valid input device");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiIN(devs);
            juce::String name = dev.getDeviceNameBasedOnIndex(inputDeviceIndex);
            expect(name.isNotEmpty());
        }
    }

    void testGetDeviceNameOutput()
    {
        beginTest("[HW] getDeviceNameBasedOnIndex - returns non-empty name for valid out0put device");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiOUT(devs);
            juce::String name = dev.getDeviceNameBasedOnIndex(inputDeviceIndex, 1);
            expect(name.isNotEmpty());
        }
    }

    void testGetDeviceIdentifier()
    {
        beginTest("[HW] getDeviceIdentifierBasedOnIndex - returns non-empty identifier");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiIN(devs);
            juce::String id = dev.getDeviceIdentifierBasedOnIndex(inputDeviceIndex);
            expect(id.isNotEmpty());
        }
    }

    void testOpenInput()
    {
        beginTest("[HW] deviceOpenIN - opens a real device successfully");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiIN(devs);

            DummyCallback callback;
            bool opened = dev.deviceOpenIN(inputDeviceIndex, &callback);
            expect(opened == true);
            expect(dev.isOpenIN() == true);
            expect(dev.get_identifier().isNotEmpty());

            dev.deviceCloseIN();
            expect(dev.isOpenIN() == false);
        }
    }

    void testOpenInputTwice()
    {
        beginTest("[HW] deviceOpenIN - opens a real device successfully with 2 consecutive calls");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiIN(devs);

            DummyCallback callback;
            bool opened = dev.deviceOpenIN(inputDeviceIndex, &callback);
            expect(opened == true);
            expect(dev.isOpenIN() == true);
            expect(dev.get_identifier().isNotEmpty());

            bool openedS = dev.deviceOpenIN(inputDeviceIndex, &callback);
            expect(openedS == true);
            expect(dev.isOpenIN() == true);
            expect(dev.get_identifier().isNotEmpty());

            dev.deviceCloseIN();
            expect(dev.isOpenIN() == false);
        }
    }

    void testCloseInputSafe()
    {
        beginTest("[HW] deviceCloseIN - closing already closed device is safe");
        MidiDevice dev;
        dev.deviceCloseIN();
        expect(dev.isOpenIN() == false);
    }

    void testOpenOutput()
    {
        beginTest("[HW] deviceOpenOUT - opens a real output device successfully");
        if (!hasOutputDevice) { logMessage("SKIPPED - no MIDI output device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiOUT(devs);

            bool opened = dev.deviceOpenOUT(outputDeviceIndex);
            expect(opened == true);
            expect(dev.isOpenOUT() == true);

            dev.deviceCloseOUT();
            expect(dev.isOpenOUT() == false);
        }
    }

    void testCloseOutputSafe()
    {
        beginTest("[HW] deviceCloseOUT - closing already closed device is safe");
        MidiDevice dev;
        dev.deviceCloseOUT();
        expect(dev.isOpenOUT() == false);
    }

    void testReopenInput()
    {
        beginTest("[HW] deviceOpenIN - reopen after close works");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiIN(devs);

            DummyCallback callback;

            expect(dev.deviceOpenIN(inputDeviceIndex, &callback) == true);
            expect(dev.isOpenIN() == true);

            dev.deviceCloseIN();
            expect(dev.isOpenIN() == false);

            dev.getAvailableDevicesMidiIN(devs);
            expect(dev.deviceOpenIN(inputDeviceIndex, &callback) == true);
            expect(dev.isOpenIN() == true);

            dev.deviceCloseIN();
        }
    }

    void testChangeVolume()
    {
        beginTest("[HW] changeVolumeInstrument - sends volume CC to output device");
        if (!hasOutputDevice) { logMessage("SKIPPED - no MIDI output device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiOUT(devs);
            dev.deviceOpenOUT(outputDeviceIndex);

                dev.setVolume(75, 1);
                dev.sendMidiCC(1,7,dev.getVolume(1));  // should not crash, sends CC7

                dev.setVolume(50, 16);
                dev.sendMidiCC(16,7,dev.getVolume(16));  // sends CC7 on channel 16

            dev.deviceCloseOUT();
        }
    }

    void testChangeReverb()
    {
        beginTest("[HW] changeReverbInstrument - sends reverb CC to output device");
        if (!hasOutputDevice) { logMessage("SKIPPED - no MIDI output device connected"); }
        else
        {
            MidiDevice dev;
            std::vector<std::string> devs;
            dev.getAvailableDevicesMidiOUT(devs);
            dev.deviceOpenOUT(outputDeviceIndex);

                dev.setReverb(60, 1);
                dev.sendMidiCC(1,91,dev.getReverb(1));  // should not crash, sends CC91

                dev.setReverb(30, 16);
                dev.sendMidiCC(16,91,dev.getReverb(16));  // sends CC91 on channel 16

            dev.deviceCloseOUT();
        }
    }

    void testNameIdentifierPairs()
    {
        beginTest("[HW] getAvailableInputDevicesNameIdentifier - returns name+identifier pairs");
        if (!hasInputDevice) { logMessage("SKIPPED - no MIDI input device connected"); }
        else
        {
            MidiDevice dev;
            auto pairs = dev.getAvailableInputDevicesNameIdentifier();
            expect((int)pairs.size() >= 1);
            expect(pairs[0].first.isNotEmpty());
            expect(pairs[0].second.isNotEmpty());
        }
    }
};

static MidiDeviceHWTest midiDeviceHWTest;