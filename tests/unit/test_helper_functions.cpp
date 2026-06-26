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

        beginTest("getNotesForChord - unknown root returns empty for every quality");
        {
            for (const char* q : { "Major", "Minor", "Diminished", "Augmented" })
            {
                Chord c; c.name = juce::String("H ") + q;   // H is not a note
                expect(ChordHelper::getNotesForChord(c).empty());
            }
        }

        // NOTE: we deliberately do NOT assert imgRoot.isValid() across all 48 names — that decodes
        // PNGs (JUCE's job) from compile-time-guaranteed BinaryData symbols, so it tests the framework,
        // not our code. We only test OUR control flow: the matched-branch flag, the early-return guard,
        // and the unknown-name fallback.

        beginTest("loadChordNeeded - a matched name sets imagesLoaded and is idempotent");
        {
            Chord c; c.name = "C Major";
            expect(! c.imagesLoaded);
            ChordHelper::loadChordNeeded(c);
            expect(c.imagesLoaded);
            ChordHelper::loadChordNeeded(c);     // already loaded -> early-return guard, no change
            expect(c.imagesLoaded);
        }

        beginTest("loadChordNeeded - unknown chord still marks loaded but assigns no image");
        {
            Chord c; c.name = "Z Wonky";         // no branch matches
            ChordHelper::loadChordNeeded(c);
            expect(c.imagesLoaded);              // flag is still set (our fallback path)
            expect(! c.imgRoot.isValid());       // ...and no image was assigned
        }
    }

};


static MapHelperTest mapHelperTest;

static ChordHelperTest chordHelperTest;
