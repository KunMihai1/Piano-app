#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "keyListener.h"

class KeyboardListenerTest : public juce::UnitTest
{
public:
    KeyboardListenerTest() : juce::UnitTest("KeyboardListener-Unit", "Unit"), device{}, handler{ device }, listener{ handler } {}

    void runTest() override
    {
        testDefaultState();
        testIsKeyboardInput();
        testStartFinishNote();
        testMapKeyMidiBasic();
        testMapKeyMidiBoundaries();
        testMapKeyMidiCustomStartNote();
        testResetState();
        testMapKeyMidiFinishNoteEdgeCases();
        testMapKeyMidiExtraKeys();
    }

private:
    MidiDevice device;
    MidiHandler handler;
    KeyboardListener listener;

    void testDefaultState()
    {
        beginTest("Default state - keyboard input disabled");
        {
            expect(listener.getIsKeyboardInput() == false);
        }

        beginTest("Default state - startNote is 60");
        {
            expect(listener.getStartNoteKeyboardInput() == 60);
        }

        beginTest("Default state - finishNote is 77");
        {
            KeyboardListener listener(handler);
            expect(listener.getFinishNoteKeyboardInput() == 77);
        }
    }

    void testIsKeyboardInput()
    {
        listener.resetState();

        beginTest("setIsKeyboardInput - enable and disable");
        {
            listener.setIsKeyboardInput(true);
            expect(listener.getIsKeyboardInput() == true);

            listener.setIsKeyboardInput(false);
            expect(listener.getIsKeyboardInput() == false);
        }
    }

    void testStartFinishNote()
    {
        listener.resetState();

        beginTest("setStartNoteKeyboardInput / getStartNoteKeyboardInput");
        {
            listener.setStartNoteKeyboardInput(48);
            expect(listener.getStartNoteKeyboardInput() == 48);

            listener.setStartNoteKeyboardInput(72);
            expect(listener.getStartNoteKeyboardInput() == 72);
        }

        beginTest("setFinishNoteKeyboardInput / getFinishNoteKeyboardInput");
        {


            listener.setFinishNoteKeyboardInput(84);
            expect(listener.getFinishNoteKeyboardInput() == 84);

            listener.setFinishNoteKeyboardInput(100);
            expect(listener.getFinishNoteKeyboardInput() == 100);
        }
    }

    void testMapKeyMidiBasic()
    {
        listener.resetState();

        beginTest("mapKeyMidi - 'A' maps to startNote (default 60)");
        {
            juce::KeyPress keyA('A');
            expect(listener.mapKeyMidi(keyA) == 60);
        }

        beginTest("mapKeyMidi - 'W' maps to startNote + 1");
        {

            juce::KeyPress keyW('W');
            expect(listener.mapKeyMidi(keyW) == 61);
        }

        beginTest("mapKeyMidi - 'S' maps to startNote + 2");
        {
            juce::KeyPress keyS('S');
            expect(listener.mapKeyMidi(keyS) == 62);
        }

        beginTest("mapKeyMidi - 'E' maps to startNote + 3");
        {

            juce::KeyPress keyE('E');
            expect(listener.mapKeyMidi(keyE) == 63);
        }

        beginTest("mapKeyMidi - 'D' maps to startNote + 4");
        {

            juce::KeyPress keyD('D');
            expect(listener.mapKeyMidi(keyD) == 64);
        }

        beginTest("mapKeyMidi - 'F' maps to startNote + 5");
        {
            juce::KeyPress keyF('F');
            expect(listener.mapKeyMidi(keyF) == 65);
        }

        beginTest("mapKeyMidi - 'K' maps to startNote + 12 (one octave)");
        {
            juce::KeyPress keyK('K');
            expect(listener.mapKeyMidi(keyK) == 72);
        }

        beginTest("mapKeyMidi - 'H' maps to startNote + 9");
        {
            juce::KeyPress keyH('H');
            expect(listener.mapKeyMidi(keyH) == 69);
        }

        beginTest("mapKeyMidi - unmapped key returns -1");
        {
            juce::KeyPress keyZ('Z');
            expect(listener.mapKeyMidi(keyZ) == -1);
            juce::KeyPress keyX('X');
            expect(listener.mapKeyMidi(keyX) == -1);
        }
    }

    void testMapKeyMidiBoundaries()
    {
        beginTest("mapKeyMidi - 'A' returns -1 when startNote < 24");
        {
            listener.setStartNoteKeyboardInput(20);
            juce::KeyPress keyA('A');
            expect(listener.mapKeyMidi(keyA) == -1);
        }

        beginTest("mapKeyMidi - 'W' returns -1 when startNote < 24");
        {
            listener.setStartNoteKeyboardInput(10);
            juce::KeyPress keyW('W');
            expect(listener.mapKeyMidi(keyW) == -1);
        }

        beginTest("mapKeyMidi - 'O' returns -1 when finishNote > 101");
        {
            listener.setFinishNoteKeyboardInput(105);
            juce::KeyPress keyO('O');
            expect(listener.mapKeyMidi(keyO) == -1);
        }

        beginTest("mapKeyMidi - 'L' returns -1 when finishNote > 101");
        {
            listener.setFinishNoteKeyboardInput(110);
            juce::KeyPress keyL('L');
            expect(listener.mapKeyMidi(keyL) == -1);
        }

        beginTest("mapKeyMidi - 'P' returns -1 when finishNote > 101");
        {
            listener.setFinishNoteKeyboardInput(102);
            juce::KeyPress keyP('P');
            expect(listener.mapKeyMidi(keyP) == -1);
        }

        beginTest("mapKeyMidi - ';' returns -1 when finishNote > 101");
        {
            listener.setFinishNoteKeyboardInput(120);
            juce::KeyPress keySemicolon(';');
            expect(listener.mapKeyMidi(keySemicolon) == -1);
        }

        beginTest("mapKeyMidi - apostrophe returns -1 when finishNote > 101");
        {
            listener.setFinishNoteKeyboardInput(115);
            juce::KeyPress keyApostrophe('\'');
            expect(listener.mapKeyMidi(keyApostrophe) == -1);
        }
    }

    void testMapKeyMidiCustomStartNote()
    {
        listener.resetState();

        beginTest("mapKeyMidi - custom startNote shifts all mappings");
        {
            listener.setStartNoteKeyboardInput(48);
            juce::KeyPress keyA('A');
            expect(listener.mapKeyMidi(keyA) == 48);
            juce::KeyPress keyW('W');
            expect(listener.mapKeyMidi(keyW) == 49);
            juce::KeyPress keyK('K');
            expect(listener.mapKeyMidi(keyK) == 60);
            juce::KeyPress keyH('H');
            expect(listener.mapKeyMidi(keyH) == 57);
        }
    }

    void testResetState()
    {
        beginTest("resetState - clears internal maps without crash");
        {
            juce::KeyPress keyA('A');
            listener.mapKeyMidi(keyA);
            juce::KeyPress keyW('W');
            listener.mapKeyMidi(keyW);
            listener.resetState();
            expect(listener.getIsKeyboardInput() == false);
            expect(listener.getStartNoteKeyboardInput() == 60);
            expect(listener.getFinishNoteKeyboardInput() == 77);
        }
    }

    void testMapKeyMidiFinishNoteEdgeCases()
    {
        listener.resetState();

        beginTest("mapKeyMidi - 'O' maps to startNote+13 when finishNote <= 101");
        {
            listener.setFinishNoteKeyboardInput(100);
            juce::KeyPress keyO('O');
            expect(listener.mapKeyMidi(keyO) == 73); // 60+13
        }

        beginTest("mapKeyMidi - 'L' maps to startNote+14 when finishNote <= 101");
        {
            listener.setFinishNoteKeyboardInput(90);
            juce::KeyPress keyL('L');
            expect(listener.mapKeyMidi(keyL) == 74); // 60+14
        }

        beginTest("mapKeyMidi - apostrophe maps to startNote+17 when finishNote <= 101");
        {
            listener.setFinishNoteKeyboardInput(95);
            juce::KeyPress keyApostrophe('\'');
            expect(listener.mapKeyMidi(keyApostrophe) == 77); // 60+17
        }

        beginTest("mapKeyMidi - 'P' maps to startNote+15 when finishNote <= 101");
        {
            listener.setFinishNoteKeyboardInput(90);
            juce::KeyPress keyP('P');
            expect(listener.mapKeyMidi(keyP) == 75);
        }

        beginTest("mapKeyMidi - ';' maps to startNote+16 when finishNote <= 101");
        {
            listener.setFinishNoteKeyboardInput(90);
            juce::KeyPress keySemicolon(';');
            expect(listener.mapKeyMidi(keySemicolon) == 76);
        }
    }

    void testMapKeyMidiExtraKeys()
    {
        listener.resetState();

        beginTest("mapKeyMidi - 'T' maps to startNote + 6");
        {
            juce::KeyPress keyT('T');
            expect(listener.mapKeyMidi(keyT) == 66);
        }

        beginTest("mapKeyMidi - 'G' maps to startNote + 7");
        {
            juce::KeyPress keyG('G');
            expect(listener.mapKeyMidi(keyG) == 67);
        }

        beginTest("mapKeyMidi - 'Y' maps to startNote + 8");
        {
            juce::KeyPress keyY('Y');
            expect(listener.mapKeyMidi(keyY) == 68);
        }

        beginTest("mapKeyMidi - 'T' returns -1 when startNote < 24");
        {
            listener.setStartNoteKeyboardInput(20);
            juce::KeyPress keyT('T');
            expect(listener.mapKeyMidi(keyT) == -1);
        }

        beginTest("mapKeyMidi - 'G' returns -1 when startNote < 24");
        {
            listener.setStartNoteKeyboardInput(10);
            juce::KeyPress keyG('G');
            expect(listener.mapKeyMidi(keyG) == -1);
        }

        beginTest("mapKeyMidi - 'Y' returns -1 when startNote < 24");
        {
            listener.setStartNoteKeyboardInput(15);
            juce::KeyPress keyY('Y');
            expect(listener.mapKeyMidi(keyY) == -1);
        }

        beginTest("mapKeyMidi - 'U' maps to startNote + 10");
        {
            listener.resetState();

            juce::KeyPress keyU('U');
            expect(listener.mapKeyMidi(keyU) == 70);
        }

        beginTest("mapKeyMidi - 'J' maps to startNote + 11");
        {
            juce::KeyPress keyJ('J');
            expect(listener.mapKeyMidi(keyJ) == 71);
        }

        beginTest("mapKeyMidi - custom startNote shifts T, G, Y, U, J mappings");
        {
            listener.setStartNoteKeyboardInput(48);

            juce::KeyPress keyT('T');
            expect(listener.mapKeyMidi(keyT) == 54);
            juce::KeyPress keyG('G');
            expect(listener.mapKeyMidi(keyG) == 55);
            juce::KeyPress keyY('Y');
            expect(listener.mapKeyMidi(keyY) == 56);
            juce::KeyPress keyU('U');
            expect(listener.mapKeyMidi(keyU) == 58);
            juce::KeyPress keyJ('J');
            expect(listener.mapKeyMidi(keyJ) == 59);
        }
    }
};

static KeyboardListenerTest keyboardListenerTest;