#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "keyListener.h"

class KeyboardListenerTest : public juce::UnitTest
{
public:
    KeyboardListenerTest() : juce::UnitTest("KeyboardListener", "Unit") {}

    void runTest() override
    {
        // ---- Default State ----

        beginTest("Default state - keyboard input disabled");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);
            expect(listener.getIsKeyboardInput() == false);
        }

        beginTest("Default state - startNote is 60");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);
            expect(listener.getStartNoteKeyboardInput() == 60);
        }

        beginTest("Default state - finishNote is 77");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);
            expect(listener.getFinishNoteKeyboardInput() == 77);
        }

        // ---- setIsKeyboardInput / getIsKeyboardInput ----

        beginTest("setIsKeyboardInput - enable and disable");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setIsKeyboardInput(true);
            expect(listener.getIsKeyboardInput() == true);

            listener.setIsKeyboardInput(false);
            expect(listener.getIsKeyboardInput() == false);
        }

        // ---- Start/Finish Note ----

        beginTest("setStartNoteKeyboardInput / getStartNoteKeyboardInput");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(48);
            expect(listener.getStartNoteKeyboardInput() == 48);

            listener.setStartNoteKeyboardInput(72);
            expect(listener.getStartNoteKeyboardInput() == 72);
        }

        beginTest("setFinishNoteKeyboardInput / getFinishNoteKeyboardInput");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(84);
            expect(listener.getFinishNoteKeyboardInput() == 84);

            listener.setFinishNoteKeyboardInput(100);
            expect(listener.getFinishNoteKeyboardInput() == 100);
        }

        // ---- mapKeyMidi ----

        beginTest("mapKeyMidi - 'A' maps to startNote (default 60)");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyA('A');
            expect(listener.mapKeyMidi(keyA) == 60);
        }

        beginTest("mapKeyMidi - 'W' maps to startNote + 1");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyW('W');
            expect(listener.mapKeyMidi(keyW) == 61);
        }

        beginTest("mapKeyMidi - 'S' maps to startNote + 2");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyS('S');
            expect(listener.mapKeyMidi(keyS) == 62);
        }

        beginTest("mapKeyMidi - 'E' maps to startNote + 3");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyE('E');
            expect(listener.mapKeyMidi(keyE) == 63);
        }

        beginTest("mapKeyMidi - 'D' maps to startNote + 4");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyD('D');
            expect(listener.mapKeyMidi(keyD) == 64);
        }

        beginTest("mapKeyMidi - 'F' maps to startNote + 5");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyF('F');
            expect(listener.mapKeyMidi(keyF) == 65);
        }

        beginTest("mapKeyMidi - 'K' maps to startNote + 12 (one octave)");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyK('K');
            expect(listener.mapKeyMidi(keyK) == 72);
        }

        beginTest("mapKeyMidi - 'H' maps to startNote + 9");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyH('H');
            expect(listener.mapKeyMidi(keyH) == 69);
        }

        beginTest("mapKeyMidi - unmapped key returns -1");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyZ('Z');
            expect(listener.mapKeyMidi(keyZ) == -1);

            juce::KeyPress keyX('X');
            expect(listener.mapKeyMidi(keyX) == -1);
        }

        // ---- mapKeyMidi boundary conditions ----

        beginTest("mapKeyMidi - 'A' returns -1 when startNote < 24");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(20);  // below 24
            juce::KeyPress keyA('A');
            expect(listener.mapKeyMidi(keyA) == -1);
        }

        beginTest("mapKeyMidi - 'W' returns -1 when startNote < 24");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(10);
            juce::KeyPress keyW('W');
            expect(listener.mapKeyMidi(keyW) == -1);
        }

        beginTest("mapKeyMidi - 'O' returns -1 when finishNote > 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(105);  // above 101
            juce::KeyPress keyO('O');
            expect(listener.mapKeyMidi(keyO) == -1);
        }

        beginTest("mapKeyMidi - 'L' returns -1 when finishNote > 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(110);
            juce::KeyPress keyL('L');
            expect(listener.mapKeyMidi(keyL) == -1);
        }

        beginTest("mapKeyMidi - 'P' returns -1 when finishNote > 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(102);
            juce::KeyPress keyP('P');
            expect(listener.mapKeyMidi(keyP) == -1);
        }

        beginTest("mapKeyMidi - ';' returns -1 when finishNote > 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(120);
            juce::KeyPress keySemicolon(';');
            expect(listener.mapKeyMidi(keySemicolon) == -1);
        }

        beginTest("mapKeyMidi - apostrophe returns -1 when finishNote > 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(115);
            juce::KeyPress keyApostrophe('\'');
            expect(listener.mapKeyMidi(keyApostrophe) == -1);
        }

        // ---- Custom startNote changes mapping ----

        beginTest("mapKeyMidi - custom startNote shifts all mappings");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

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

        // ---- resetState ----

        beginTest("resetState - clears internal maps without crash");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            // map some keys first
            juce::KeyPress keyA('A');
            listener.mapKeyMidi(keyA);
            juce::KeyPress keyW('W');
            listener.mapKeyMidi(keyW);

            listener.resetState();

            // after reset, basic getters still work
            expect(listener.getIsKeyboardInput() == false);
            expect(listener.getStartNoteKeyboardInput() == 60);
            expect(listener.getFinishNoteKeyboardInput() == 77);
        }

        // ---- O, L, P, ;, ' allowed when finishNote <= 101 ----

        beginTest("mapKeyMidi - 'O' maps to startNote+13 when finishNote <= 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(100);
            juce::KeyPress keyO('O');
            expect(listener.mapKeyMidi(keyO) == 60 + 13);
        }

        beginTest("mapKeyMidi - 'L' maps to startNote+14 when finishNote <= 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(90);
            juce::KeyPress keyL('L');
            expect(listener.mapKeyMidi(keyL) == 60 + 14);
        }

        beginTest("mapKeyMidi - apostrophe maps to startNote+17 when finishNote <= 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(95);
            juce::KeyPress keyApostrophe('\'');
            expect(listener.mapKeyMidi(keyApostrophe) == 60 + 17);
        }

        // ---- mapKeyMidi - missing key mappings: T, G, Y ----

        beginTest("mapKeyMidi - 'T' maps to startNote + 6");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyT('T');
            expect(listener.mapKeyMidi(keyT) == 66);
        }

        beginTest("mapKeyMidi - 'G' maps to startNote + 7");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyG('G');
            expect(listener.mapKeyMidi(keyG) == 67);
        }

        beginTest("mapKeyMidi - 'Y' maps to startNote + 8");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyY('Y');
            expect(listener.mapKeyMidi(keyY) == 68);
        }

        beginTest("mapKeyMidi - 'T' returns -1 when startNote < 24");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(20);
            juce::KeyPress keyT('T');
            expect(listener.mapKeyMidi(keyT) == -1);
        }

        beginTest("mapKeyMidi - 'G' returns -1 when startNote < 24");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(10);
            juce::KeyPress keyG('G');
            expect(listener.mapKeyMidi(keyG) == -1);
        }

        beginTest("mapKeyMidi - 'Y' returns -1 when startNote < 24");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(15);
            juce::KeyPress keyY('Y');
            expect(listener.mapKeyMidi(keyY) == -1);
        }

        // ---- mapKeyMidi - U, J (no boundary guard) ----

        beginTest("mapKeyMidi - 'U' maps to startNote + 10");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyU('U');
            expect(listener.mapKeyMidi(keyU) == 70);
        }

        beginTest("mapKeyMidi - 'J' maps to startNote + 11");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            juce::KeyPress keyJ('J');
            expect(listener.mapKeyMidi(keyJ) == 71);
        }

        // ---- mapKeyMidi - P, ; (finishNote <= 101 guard, valid path) ----

        beginTest("mapKeyMidi - 'P' maps to startNote+15 when finishNote <= 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(90);
            juce::KeyPress keyP('P');
            expect(listener.mapKeyMidi(keyP) == 60 + 15);
        }

        beginTest("mapKeyMidi - ';' maps to startNote+16 when finishNote <= 101");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setFinishNoteKeyboardInput(90);
            juce::KeyPress keySemicolon(';');
            expect(listener.mapKeyMidi(keySemicolon) == 60 + 16);
        }

        // ---- mapKeyMidi - custom startNote with T, G, Y, U, J ----

        beginTest("mapKeyMidi - custom startNote shifts T, G, Y, U, J mappings");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setStartNoteKeyboardInput(48);

            juce::KeyPress keyT('T');
            expect(listener.mapKeyMidi(keyT) == 54);  // 48 + 6

            juce::KeyPress keyG('G');
            expect(listener.mapKeyMidi(keyG) == 55);  // 48 + 7

            juce::KeyPress keyY('Y');
            expect(listener.mapKeyMidi(keyY) == 56);  // 48 + 8

            juce::KeyPress keyU('U');
            expect(listener.mapKeyMidi(keyU) == 58);  // 48 + 10

            juce::KeyPress keyJ('J');
            expect(listener.mapKeyMidi(keyJ) == 59);  // 48 + 11
        }

        // ---- keyPressed ----

        beginTest("keyPressed - returns false when keyboard input is disabled");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            // keyboard input is disabled by default
            juce::KeyPress keyA('A');
            expect(listener.keyPressed(keyA, nullptr) == false);
        }

        beginTest("keyPressed - returns false when keyboard input is enabled");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setIsKeyboardInput(true);
            juce::KeyPress keyA('A');
            // keyPressed always returns false regardless of key validity
            expect(listener.keyPressed(keyA, nullptr) == false);
        }

        beginTest("keyPressed - unmapped key does not crash when enabled");
        {
            MidiDevice device;
            MidiHandler handler(device);
            KeyboardListener listener(handler);

            listener.setIsKeyboardInput(true);
            juce::KeyPress keyZ('Z');
            expect(listener.keyPressed(keyZ, nullptr) == false);
        }
    }
};

static KeyboardListenerTest keyboardListenerTest;
