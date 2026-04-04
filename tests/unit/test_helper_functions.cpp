#include <juce_core/juce_core.h>
#include "HelperFunctions.h"

class MapHelperTest : public juce::UnitTest
{
public:
    MapHelperTest() : juce::UnitTest("MapHelper", "Unit") {}

    void runTest() override
    {
        testIntToStringNoteValid();
        testIntToStringNoteInvalid();
        testStringToIntNoteValid();
        testStringToIntNoteInvalid();
        testIntStringIntConsistency();
    }

private:
    void testIntToStringNoteValid()
    {
        beginTest("intToStringNote - valid notes");
        expect(MapHelper::intToStringNote(60) == "C4");
        expect(MapHelper::intToStringNote(69) == "A4");
        expect(MapHelper::intToStringNote(0) == "C-1");
    }

    void testIntToStringNoteInvalid()
    {
        beginTest("intToStringNote - invalid notes");
        expect(MapHelper::intToStringNote(-1) == "Invalid");
        expect(MapHelper::intToStringNote(128) == "Invalid");
    }

    void testStringToIntNoteValid()
    {
        beginTest("stringToIntNote - valid strings");
        expect(MapHelper::stringToIntNote("C4") == 60);
        expect(MapHelper::stringToIntNote("A4") == 69);
    }

    void testStringToIntNoteInvalid()
    {
        beginTest("stringToIntNote - invalid strings");
        expect(MapHelper::stringToIntNote("XYZ") == -1);
        expect(MapHelper::stringToIntNote("") == -1);
    }

    void testIntStringIntConsistency()
    {
        beginTest("int-string-int");
        for (int i = 0; i <= 127; ++i)
        {
            auto str = MapHelper::intToStringNote(i);
            expect(MapHelper::stringToIntNote(str) == i);
        }
    }
};

class ChordHelperTest : public juce::UnitTest
{
public:
    ChordHelperTest() : juce::UnitTest("ChordHelper", "Unit") {}

    void runTest() override
    {
        testMajorChords();
        testMinorChords();
        testDiminishedChords();
        testAugmentedChords();
        testSharpRootNotes();
        testInvalidRootChord();
        testSeventhChord();
    }

private:
    void testMajorChords()
    {
        beginTest("Major chords return correct intervals (root, maj3, perf5)");
        {
            Chord c; c.name = "C Major";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 60, 64, 67 }));
        }
        {
            Chord c; c.name = "A Major";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 69, 73, 76 }));
        }
    }

    void testMinorChords()
    {
        beginTest("Minor chords return correct intervals (root, min3, perf5)");
        Chord c; c.name = "D Minor";
        auto notes = ChordHelper::getNotesForChord(c);
        expect(notes == std::vector<int>({ 62, 65, 69 }));
    }

    void testDiminishedChords()
    {
        beginTest("Diminished chords (root, min3, dim5)");
        Chord c; c.name = "C Diminished";
        auto notes = ChordHelper::getNotesForChord(c);
        expect(notes == std::vector<int>({ 60, 63, 66 }));
    }

    void testAugmentedChords()
    {
        beginTest("Augmented chords (root, maj3, aug5)");
        Chord c; c.name = "C Augmented";
        auto notes = ChordHelper::getNotesForChord(c);
        expect(notes == std::vector<int>({ 60, 64, 68 }));
    }

    void testSharpRootNotes()
    {
        beginTest("Sharp root notes");
        Chord c; c.name = "F# Minor";
        auto notes = ChordHelper::getNotesForChord(c);
        expect(notes == std::vector<int>({ 66, 69, 73 }));
    }

    void testInvalidRootChord()
    {
        beginTest("Invalid root returns empty");
        Chord c; c.name = "X Major";
        auto notes = ChordHelper::getNotesForChord(c);
        expect(notes.empty());
    }

    void testSeventhChord()
    {
        beginTest("7th chord");
        Chord c; c.name = "C7";
        auto notes = ChordHelper::getNotesForChord(c);
        expect(notes == std::vector<int>({ 60, 64, 67, 70 }));
    }
};

// Register tests
static MapHelperTest mapHelperTest;
static ChordHelperTest chordHelperTest;