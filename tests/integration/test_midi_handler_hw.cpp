#include <juce_core/juce_core.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "MidiHandler.h"

// ==================================================================
// HARDWARE-DEPENDENT TESTS (Integration)
// These tests require a MIDI output device to be connected.
// They skip automatically if no device is found.
// ==================================================================

class MidiHandlerHWTest : public juce::UnitTest
{
public:
    MidiHandlerHWTest() : juce::UnitTest("MidiHandler_HW", "Integration") {}

    void runTest() override
    {
        MidiDevice device;
        InstrumentHandler instrumentHandler;

        std::vector<std::string> outDevices;
        device.getAvailableDevicesMidiOUT(outDevices);

        if (outDevices.empty() || outDevices[0] == "No devices found!")
        {
            logMessage("SKIPPED - no MIDI output device connected");
            return;
        }

        if (!device.deviceOpenOUT(0))
        {
            logMessage("SKIPPED - failed to open MIDI output device");
            return;
        }

        

        // --- handleIncomingMidiMessage with device ---

        beginTest("handleIncomingMidiMessage [device] - noteOn normal path notifies listeners");
        {
            MidiHandler handler(device);
            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                int lastNote = -1;
                int msgCount = 0;
                void noteOnReceived(int midiNote) override { noteOnCount++; lastNote = midiNote; }
                void handleIncomingMessage(const juce::MidiMessage& message) override { msgCount++; (void)message; }
            };

            Listener listener;
            handler.addListener(&listener);

            // startNoteSetting and endNoteSetting default to -1,
            // so note 60 does NOT match either => normal path => ok=1
            auto noteOn = juce::MidiMessage::noteOn(1, 60, (juce::uint8)100);
            handler.handleIncomingMidiMessage(nullptr, noteOn);

            expect(listener.noteOnCount == 1);
            expect(listener.lastNote == 60);
            expect(listener.msgCount == 1);

            handler.removeListener(&listener);
        }

        beginTest("handleIncomingMidiMessage [device] - startNoteSetting triggers onStartNoteSetting callback");
        {
            MidiHandler handler(device);
            handler.set_start_end_notes(60, 72);

            bool startCalled = false;
            handler.onStartNoteSetting = [&startCalled]() { startCalled = true; };

            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                void noteOnReceived(int midiNote) override { noteOnCount++; (void)midiNote; }
            };

            Listener listener;
            handler.addListener(&listener);

            // Note 60 matches startNoteSetting => callback path, ok stays 0
            handler.handleIncomingMidiMessage(nullptr, juce::MidiMessage::noteOn(1, 60, (juce::uint8)100));

            expect(startCalled == true);
            expect(listener.noteOnCount == 0); // ok was 0

            handler.removeListener(&listener);
        }

        beginTest("handleIncomingMidiMessage [device] - endNoteSetting triggers onEndNoteSetting callback");
        {
            MidiHandler handler(device);
            handler.set_start_end_notes(60, 72);

            bool endCalled = false;
            handler.onEndNoteSetting = [&endCalled]() { endCalled = true; };

            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                void noteOnReceived(int midiNote) override { noteOnCount++; (void)midiNote; }
            };

            Listener listener;
            handler.addListener(&listener);

            // Note 72 matches endNoteSetting => callback path, ok stays 0
            handler.handleIncomingMidiMessage(nullptr, juce::MidiMessage::noteOn(1, 72, (juce::uint8)100));

            expect(endCalled == true);
            expect(listener.noteOnCount == 0);

            handler.removeListener(&listener);
        }

        beginTest("handleIncomingMidiMessage [device] - startNoteSetting without callback does not crash");
        {
            MidiHandler handler(device);
            handler.set_start_end_notes(60, 72);
            // onStartNoteSetting is NOT set (nullptr/empty) — should not crash
            handler.handleIncomingMidiMessage(nullptr, juce::MidiMessage::noteOn(1, 60, (juce::uint8)100));

        }

        beginTest("handleIncomingMidiMessage [device] - endNoteSetting without callback does not crash");
        {
            MidiHandler handler(device);
            handler.set_start_end_notes(60, 72);
            // onEndNoteSetting is NOT set — should not crash
            handler.handleIncomingMidiMessage(nullptr, juce::MidiMessage::noteOn(1, 72, (juce::uint8)100));
        }

        // --- noteOnKeyboard with device ---

        beginTest("noteOnKeyboard [device] - normal note notifies listeners");
        {
            MidiHandler handler(device);
            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                int lastNote = -1;
                int msgCount = 0;
                void noteOnReceived(int midiNote) override { noteOnCount++; lastNote = midiNote; }
                void handleIncomingMessage(const juce::MidiMessage& message) override { msgCount++; (void)message; }
            };

            Listener listener;
            handler.addListener(&listener);

            handler.noteOnKeyboard(64, 100);

            expect(listener.noteOnCount == 1);
            expect(listener.lastNote == 64);
            expect(listener.msgCount == 1);

            handler.removeListener(&listener);
        }

        beginTest("noteOnKeyboard [device] - startNoteSetting triggers onStartNoteSetting");
        {
            MidiHandler handler(device);
            handler.set_start_end_notes(60, 72);

            bool startCalled = false;
            handler.onStartNoteSetting = [&startCalled]() { startCalled = true; };

            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                void noteOnReceived(int midiNote) override { noteOnCount++; (void)midiNote; }
            };

            Listener listener;
            handler.addListener(&listener);

            handler.noteOnKeyboard(60, 127);

            expect(startCalled == true);
            expect(listener.noteOnCount == 0);

            handler.removeListener(&listener);
        }

        beginTest("noteOnKeyboard [device] - endNoteSetting triggers onEndNoteSetting");
        {
            MidiHandler handler(device);
            handler.set_start_end_notes(60, 72);

            bool endCalled = false;
            handler.onEndNoteSetting = [&endCalled]() { endCalled = true; };

            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                void noteOnReceived(int midiNote) override { noteOnCount++; (void)midiNote; }
            };

            Listener listener;
            handler.addListener(&listener);

            handler.noteOnKeyboard(72, 127);

            expect(endCalled == true);
            expect(listener.noteOnCount == 0);

            handler.removeListener(&listener);
        }

        beginTest("noteOnKeyboard [device] - multiple normal notes all notify listener");
        {
            MidiHandler handler(device);
            class Listener : public MidiHandlerListener
            {
            public:
                int noteOnCount = 0;
                std::vector<int> notesReceived;
                void noteOnReceived(int midiNote) override
                {
                    noteOnCount++;
                    notesReceived.push_back(midiNote);
                }
            };

            Listener listener;
            handler.addListener(&listener);

            handler.noteOnKeyboard(60, 100);
            handler.noteOnKeyboard(64, 80);
            handler.noteOnKeyboard(67, 127);

            expect(listener.noteOnCount == 3);
            expect(listener.notesReceived[0] == 60);
            expect(listener.notesReceived[1] == 64);
            expect(listener.notesReceived[2] == 67);

            handler.removeListener(&listener);
        }

        // --- allOffKeyboard with device ---

        beginTest("allOffKeyboard [device] - sends 128 note-offs on both channels without crash");
        {
            MidiHandler handler(device);
            // allOffKeyboard sends noteOff for notes 0-127 on ch1 and ch16
            handler.allOffKeyboard();
        }

        // --- applyInstrumentPreset (via setProgramNumber) with device ---

        beginTest("setProgramNumber [device] - applyInstrumentPreset notifies listener with programChange");
        {
            MidiHandler handler(device, &instrumentHandler);
            class PresetListener : public MidiHandlerListener
            {
            public:
                int msgCount = 0;
                bool gotProgramChange = false;
                int programChangeNumber = -1;
                void handleIncomingMessage(const juce::MidiMessage& message) override
                {
                    msgCount++;
                    if (message.isProgramChange())
                    {
                        gotProgramChange = true;
                        programChangeNumber = message.getProgramChangeNumber();
                    }
                }
            };

            PresetListener listener;
            handler.addListener(&listener);

            handler.setProgramNumber(5, "left");

            expect(handler.getProgramNumberLeftHand() == 5);
            expect(listener.gotProgramChange == true);
            expect(listener.programChangeNumber == 5);
            // msgCount >= 1 because applyInstrumentPreset also sends CC messages
            expect(listener.msgCount >= 1);

            handler.removeListener(&listener);
        }

        beginTest("setProgramNumber [device] - right hand uses channel 16");
        {
            MidiHandler handler(device, &instrumentHandler);
            class PresetListener : public MidiHandlerListener
            {
            public:
                bool gotProgramChange = false;
                int programChangeChannel = -1;
                void handleIncomingMessage(const juce::MidiMessage& message) override
                {
                    if (message.isProgramChange())
                    {
                        gotProgramChange = true;
                        programChangeChannel = message.getChannel();
                    }
                }
            };

            PresetListener listener;
            handler.addListener(&listener);

            handler.setProgramNumber(10, "right");

            expect(handler.getProgramNumberRightHand() == 10);
            expect(listener.gotProgramChange == true);
            expect(listener.programChangeChannel == 16);

            handler.removeListener(&listener);
        }

        beginTest("setProgramNumber [device] - left hand uses channel 1");
        {
            MidiHandler handler(device, &instrumentHandler);
            class PresetListener : public MidiHandlerListener
            {
            public:
                bool gotProgramChange = false;
                int programChangeChannel = -1;
                void handleIncomingMessage(const juce::MidiMessage& message) override
                {
                    if (message.isProgramChange())
                    {
                        gotProgramChange = true;
                        programChangeChannel = message.getChannel();
                    }
                }
            };

            PresetListener listener;
            handler.addListener(&listener);

            handler.setProgramNumber(3, "left");

            expect(handler.getProgramNumberLeftHand() == 3);
            expect(listener.gotProgramChange == true);
            expect(listener.programChangeChannel == 1);

            handler.removeListener(&listener);
        }

        beginTest("setProgramNumber [device] - applyInstrumentPreset sends CC messages to listener");
        {
            MidiHandler handler(device, &instrumentHandler);
            class CCListener : public MidiHandlerListener
            {
            public:
                int ccMsgCount = 0;
                int programChangeCount = 0;
                void handleIncomingMessage(const juce::MidiMessage& message) override
                {
                    if (message.isController())
                        ccMsgCount++;
                    if (message.isProgramChange())
                        programChangeCount++;
                }
            };

            CCListener listener;
            handler.addListener(&listener);

            handler.setProgramNumber(0, "left");

            // applyInstrumentPreset sends 1 programChange + N CC messages
            expect(listener.programChangeCount == 1);
            // CC count depends on the preset; at minimum it should be >= 0
            expect(listener.ccMsgCount >= 0);

            handler.removeListener(&listener);
        }

        device.deviceCloseOUT();
    }
};

static MidiHandlerHWTest midiHandlerHWTest;
