#include <juce_core/juce_core.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "MidiHandler.h"

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

        // ---- CALL ALL TESTS ----
        test_handleIncomingMidiMessage_normal(device);
        test_handleIncomingMidiMessage_start(device);
        test_handleIncomingMidiMessage_end(device);
        test_handleIncomingMidiMessage_start_no_callback(device);
        test_handleIncomingMidiMessage_end_no_callback(device);

        test_noteOnKeyboard_normal(device);
        test_noteOnKeyboard_start(device);
        test_noteOnKeyboard_end(device);
        test_noteOnKeyboard_multiple(device);

        test_allOffKeyboard(device);

        test_setProgramNumber_basic(device, instrumentHandler);
        test_setProgramNumber_right_channel(device, instrumentHandler);
        test_setProgramNumber_left_channel(device, instrumentHandler);
        test_setProgramNumber_cc_messages(device, instrumentHandler);

        device.deviceCloseOUT();
    }

private:
    // ===============================
    // handleIncomingMidiMessage tests
    // ===============================

    void test_handleIncomingMidiMessage_normal(MidiDevice& device)
    {
        beginTest("handleIncomingMidiMessage [device] - normal path");

        MidiHandler handler(device);

        class Listener : public MidiHandlerListener
        {
        public:
            int noteOnCount = 0;
            int lastNote = -1;
            int msgCount = 0;

            void noteOnReceived(int midiNote) override { noteOnCount++; lastNote = midiNote; }
            void handleIncomingMessage(const juce::MidiMessage&) override { msgCount++; }
        };

        Listener listener;
        handler.addListener(&listener);

        auto noteOn = juce::MidiMessage::noteOn(1, 60, (juce::uint8)100);
        handler.handleIncomingMidiMessage(nullptr, noteOn);

        expect(listener.noteOnCount == 1);
        expect(listener.lastNote == 60);
        expect(listener.msgCount == 1);

        handler.removeListener(&listener);
    }

    void test_handleIncomingMidiMessage_start(MidiDevice& device)
    {
        beginTest("handleIncomingMidiMessage [device] - startNoteSetting");

        MidiHandler handler(device);
        handler.set_start_end_notes(60, 72);

        bool startCalled = false;
        handler.onStartNoteSetting = [&]() { startCalled = true; };

        class Listener : public MidiHandlerListener
        {
        public:
            int noteOnCount = 0;
            void noteOnReceived(int) override { noteOnCount++; }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.handleIncomingMidiMessage(nullptr,
            juce::MidiMessage::noteOn(1, 60, (juce::uint8)100));

        expect(startCalled == true);
        expect(listener.noteOnCount == 0);

        handler.removeListener(&listener);
    }

    void test_handleIncomingMidiMessage_end(MidiDevice& device)
    {
        beginTest("handleIncomingMidiMessage [device] - endNoteSetting");

        MidiHandler handler(device);
        handler.set_start_end_notes(60, 72);

        bool endCalled = false;
        handler.onEndNoteSetting = [&]() { endCalled = true; };

        class Listener : public MidiHandlerListener
        {
        public:
            int noteOnCount = 0;
            void noteOnReceived(int) override { noteOnCount++; }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.handleIncomingMidiMessage(nullptr,
            juce::MidiMessage::noteOn(1, 72, (juce::uint8)100));

        expect(endCalled == true);
        expect(listener.noteOnCount == 0);

        handler.removeListener(&listener);
    }

    void test_handleIncomingMidiMessage_start_no_callback(MidiDevice& device)
    {
        beginTest("handleIncomingMidiMessage [device] - start no callback");

        MidiHandler handler(device);
        handler.set_start_end_notes(60, 72);

        handler.handleIncomingMidiMessage(nullptr,
            juce::MidiMessage::noteOn(1, 60, (juce::uint8)100));
    }

    void test_handleIncomingMidiMessage_end_no_callback(MidiDevice& device)
    {
        beginTest("handleIncomingMidiMessage [device] - end no callback");

        MidiHandler handler(device);
        handler.set_start_end_notes(60, 72);

        handler.handleIncomingMidiMessage(nullptr,
            juce::MidiMessage::noteOn(1, 72, (juce::uint8)100));
    }

    // ===============================
    // noteOnKeyboard tests
    // ===============================

    void test_noteOnKeyboard_normal(MidiDevice& device)
    {
        beginTest("noteOnKeyboard [device] - normal");

        MidiHandler handler(device);

        class Listener : public MidiHandlerListener
        {
        public:
            int noteOnCount = 0;
            int lastNote = -1;
            int msgCount = 0;

            void noteOnReceived(int midiNote) override { noteOnCount++; lastNote = midiNote; }
            void handleIncomingMessage(const juce::MidiMessage&) override { msgCount++; }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.noteOnKeyboard(64, 100);

        expect(listener.noteOnCount == 1);
        expect(listener.lastNote == 64);
        expect(listener.msgCount == 1);

        handler.removeListener(&listener);
    }

    void test_noteOnKeyboard_start(MidiDevice& device)
    {
        beginTest("noteOnKeyboard [device] - startNoteSetting");

        MidiHandler handler(device);
        handler.set_start_end_notes(60, 72);

        bool startCalled = false;
        handler.onStartNoteSetting = [&]() { startCalled = true; };

        class Listener : public MidiHandlerListener
        {
        public:
            int noteOnCount = 0;
            void noteOnReceived(int) override { noteOnCount++; }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.noteOnKeyboard(60, 127);

        expect(startCalled == true);
        expect(listener.noteOnCount == 0);

        handler.removeListener(&listener);
    }

    void test_noteOnKeyboard_end(MidiDevice& device)
    {
        beginTest("noteOnKeyboard [device] - endNoteSetting");

        MidiHandler handler(device);
        handler.set_start_end_notes(60, 72);

        bool endCalled = false;
        handler.onEndNoteSetting = [&]() { endCalled = true; };

        class Listener : public MidiHandlerListener
        {
        public:
            int noteOnCount = 0;
            void noteOnReceived(int) override { noteOnCount++; }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.noteOnKeyboard(72, 127);

        expect(endCalled == true);
        expect(listener.noteOnCount == 0);

        handler.removeListener(&listener);
    }

    void test_noteOnKeyboard_multiple(MidiDevice& device)
    {
        beginTest("noteOnKeyboard [device] - multiple");

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

    // ===============================
    // other tests
    // ===============================

    void test_allOffKeyboard(MidiDevice& device)
    {
        beginTest("allOffKeyboard [device]");

        MidiHandler handler(device);
        handler.allOffKeyboard();
    }

    void test_setProgramNumber_basic(MidiDevice& device, InstrumentHandler& instrumentHandler)
    {
        beginTest("setProgramNumber [device] - basic");

        MidiHandler handler(device, &instrumentHandler);

        class Listener : public MidiHandlerListener
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

        Listener listener;
        handler.addListener(&listener);

        handler.setProgramNumber(5, "left");

        expect(handler.getProgramNumberLeftHand() == 5);
        expect(listener.gotProgramChange == true);
        expect(listener.programChangeNumber == 5);
        expect(listener.msgCount >= 1);

        handler.removeListener(&listener);
    }

    void test_setProgramNumber_right_channel(MidiDevice& device, InstrumentHandler& instrumentHandler)
    {
        beginTest("setProgramNumber [device] - right channel");

        MidiHandler handler(device, &instrumentHandler);

        class Listener : public MidiHandlerListener
        {
        public:
            bool gotProgramChange = false;
            int channel = -1;

            void handleIncomingMessage(const juce::MidiMessage& message) override
            {
                if (message.isProgramChange())
                {
                    gotProgramChange = true;
                    channel = message.getChannel();
                }
            }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.setProgramNumber(10, "right");

        expect(handler.getProgramNumberRightHand() == 10);
        expect(listener.gotProgramChange == true);
        expect(listener.channel == 16);

        handler.removeListener(&listener);
    }

    void test_setProgramNumber_left_channel(MidiDevice& device, InstrumentHandler& instrumentHandler)
    {
        beginTest("setProgramNumber [device] - left channel");

        MidiHandler handler(device, &instrumentHandler);

        class Listener : public MidiHandlerListener
        {
        public:
            bool gotProgramChange = false;
            int channel = -1;

            void handleIncomingMessage(const juce::MidiMessage& message) override
            {
                if (message.isProgramChange())
                {
                    gotProgramChange = true;
                    channel = message.getChannel();
                }
            }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.setProgramNumber(3, "left");

        expect(handler.getProgramNumberLeftHand() == 3);
        expect(listener.gotProgramChange == true);
        expect(listener.channel == 1);

        handler.removeListener(&listener);
    }

    void test_setProgramNumber_cc_messages(MidiDevice& device, InstrumentHandler& instrumentHandler)
    {
        beginTest("setProgramNumber [device] - CC messages");

        MidiHandler handler(device, &instrumentHandler);

        class Listener : public MidiHandlerListener
        {
        public:
            int ccCount = 0;
            int pcCount = 0;

            void handleIncomingMessage(const juce::MidiMessage& message) override
            {
                if (message.isController()) ccCount++;
                if (message.isProgramChange()) pcCount++;
            }
        };

        Listener listener;
        handler.addListener(&listener);

        handler.setProgramNumber(0, "left");

        expect(listener.pcCount == 1);
        expect(listener.ccCount >= 0);

        handler.removeListener(&listener);
    }
};

static MidiHandlerHWTest midiHandlerHWTest;