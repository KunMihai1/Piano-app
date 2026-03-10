#include <juce_core/juce_core.h>
#include "HelperFunctions.h"
class MapHelperTest : public juce::UnitTest
{
public:
    MapHelperTest() : juce::UnitTest("MapHelper", "Unit") {}
    void runTest() override
    {
        beginTest("intToStringNote - valid notes");
        expect(MapHelper::intToStringNote(60) == "C4");   
        expect(MapHelper::intToStringNote(69) == "A4");   
        expect(MapHelper::intToStringNote(0) == "C-1");  


        beginTest("intToStringNote - invalid notes");
        expect(MapHelper::intToStringNote(-1) == "Invalid");
        expect(MapHelper::intToStringNote(128) == "Invalid");


        beginTest("stringToIntNote - valid strings");
        expect(MapHelper::stringToIntNote("C4") == 60);
        expect(MapHelper::stringToIntNote("A4") == 69);


        beginTest("stringToIntNote - invalid strings");
        expect(MapHelper::stringToIntNote("XYZ") == -1);
        expect(MapHelper::stringToIntNote("") == -1);


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
        beginTest("Minor chords return correct intervals (root, min3, perf5)");
        {
            Chord c; c.name = "D Minor";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 62, 65, 69 }));  
        }

        beginTest("Diminished chords (root, min3, dim5)");
        {
            Chord c; c.name = "C Diminished";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 60, 63, 66 }));
        }

        beginTest("Augmented chords (root, maj3, aug5)");
        {
            Chord c; c.name = "C Augmented";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 60, 64, 68 }));
        }

        beginTest("Sharp root notes");
        {
            Chord c; c.name = "F# Minor";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 66, 69, 73 }));  
        }

        beginTest("Invalid root returns empty");
        {
            Chord c; c.name = "X Major";
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes.empty());
        }

        beginTest("7th chord");
        {
            Chord c; c.name = "C7";  
            auto notes = ChordHelper::getNotesForChord(c);
            expect(notes == std::vector<int>({ 60, 64, 67, 70 }));  
        }
    }

};


static MapHelperTest mapHelperTest;

static ChordHelperTest chordHelperTest;
