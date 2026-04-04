#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "IOHelper.h"


class TrackIOHelperTest : public juce::UnitTest
{

public:
    TrackIOHelperTest() : juce::UnitTest("TrackIOHelper", "Unit") {}

    void runTest() override
    {

        testFoundPercussionTrue();
        testFoundPercussionFalse();
        testFoundPercussionEmpty();
        testFoundPercussionMixedChannels();

        testGetOriginalBpmEmptyFile();
        testGetOriginalBpmFromTrack();
        testGetOriginalBpmNoTempo();

        testConvertTicksToSeconds120Bpm();
        testConvertTicksZeroStaysZero();


        testApplyChangesNotePairBasic();
        testApplyChangesNotePairTimestampShift();
        testApplyChangesNotePairClampToZero();
        testApplyChangesNotePairOutOfRange();

        testApplyChangesSequenceBasic();
        testApplyChangesSequenceOutOfRange();

        testExtractNotePairsBasic();
        testExtractNotePairsEmpty();

        testSaveLoadBasicRoundtrip();
        testSaveLoadWithChangesMap();
        testSaveLoadEmptyFolder();
        testLoadFromNonExistentFile();
        testSaveLoadMultipleFolders();
        
    }


private:

    void testFoundPercussionTrue()
    {
        beginTest("foundPercussion - returns true for channel 10 notes");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(10, 36, (juce::uint8)100), 0.0);
            seq.addEvent(juce::MidiMessage::noteOff(10, 36), 0.5);
            expect(TrackIOHelper::foundPercussion(&seq) == true);
        }
    }

    void testFoundPercussionFalse()
    {
        beginTest("foundPercussion - returns false for non-channel-10 notes");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 0.5);
            expect(TrackIOHelper::foundPercussion(&seq) == false);
        }
    }

    void testFoundPercussionEmpty()
    {
        beginTest("foundPercussion - returns false for empty sequence");
        {
            juce::MidiMessageSequence seq;
            expect(TrackIOHelper::foundPercussion(&seq) == false);
        }
    }

    void testFoundPercussionMixedChannels()
    {
        //assumption is here that a midi file has been correctly been edited(this part isn't for my app to control)
        //in any situation possible, if a single track which should have all the notes on the same channel
        //has mixed channels, then i can't make neither assumption and correctly take a decision if it's
        //a percussion sequence of events or a normal one
        //check can be added so that when a user wants to add his midi file, a check for correctness of tracks
        //can be made
        beginTest("foundPercussion - mixed channels, one is channel 10");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 0.5);
            seq.addEvent(juce::MidiMessage::noteOn(10, 42, (juce::uint8)80), 1.0);
            seq.addEvent(juce::MidiMessage::noteOff(10, 42), 1.5);
            expect(TrackIOHelper::foundPercussion(&seq) == true);
        }
    }

    void testGetOriginalBpmEmptyFile()
    {
        beginTest("getOriginalBpmFromFile - returns 120 for empty MIDI file");
        {
            juce::MidiFile midiFile;
            expect(TrackIOHelper::getOriginalBpmFromFile(midiFile) == 120.0);
        }
    }

    void testGetOriginalBpmFromTrack()
    {
        beginTest("getOriginalBpmFromFile - reads tempo from first track");
        {
            juce::MidiFile midiFile;
            juce::MidiMessageSequence tempoTrack;

            // 140 BPM = 60/140 seconds per quarter note
            double spqn = 60.0 / 140.0;
            auto tempoMsg = juce::MidiMessage::tempoMetaEvent(
                static_cast<int>(spqn * 1000000.0)  // microseconds per quarter note
            );
            tempoMsg.setTimeStamp(0.0);
            tempoTrack.addEvent(tempoMsg);

            midiFile.addTrack(tempoTrack);
            double bpm = TrackIOHelper::getOriginalBpmFromFile(midiFile);

            //0.5 tolerance
            expect(std::abs(bpm - 140.0) < 0.5);
        }
    }

    void testGetOriginalBpmNoTempo()
    {
        beginTest("getOriginalBpmFromFile - returns 120 if no tempo event");
        {
            juce::MidiFile midiFile;
            juce::MidiMessageSequence track;
            track.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
            midiFile.addTrack(track);
            expect(TrackIOHelper::getOriginalBpmFromFile(midiFile) == 120.0);
        }
    }

    void testConvertTicksToSeconds120Bpm()
    {
        beginTest("convertTicksToSeconds - converts correctly at 120 BPM");
        {
            juce::MidiFile midiFile;
            midiFile.setTicksPerQuarterNote(960);

            juce::MidiMessageSequence track;
            // A note at tick 960 = 1 quarter note = 0.5 seconds at 120 BPM
            auto noteOn = juce::MidiMessage::noteOn(1, 60, (juce::uint8)100);
            noteOn.setTimeStamp(960.0);
            track.addEvent(noteOn);
            midiFile.addTrack(track);

            TrackIOHelper::convertTicksToSeconds(midiFile, 120.0);

            auto* seq = midiFile.getTrack(0);
            double convertedTime = seq->getEventPointer(0)->message.getTimeStamp();
            // At 120 BPM, 960 ticks = 0.5 seconds
            expect(std::abs(convertedTime - 0.5) < 0.001);
        }
    }

    void testConvertTicksZeroStaysZero()
    {
        beginTest("convertTicksToSeconds - tick 0 stays at 0");
        {
            juce::MidiFile midiFile;
            midiFile.setTicksPerQuarterNote(480);

            juce::MidiMessageSequence track;
            auto noteOn = juce::MidiMessage::noteOn(1, 60, (juce::uint8)100);
            noteOn.setTimeStamp(0.0);
            track.addEvent(noteOn);
            midiFile.addTrack(track);

            TrackIOHelper::convertTicksToSeconds(midiFile, 120.0);

            auto* seq = midiFile.getTrack(0);
            expect(seq->getEventPointer(0)->message.getTimeStamp() == 0.0);
        }
    }

    void testApplyChangesNotePairBasic()
    {
        beginTest("applyChangesToASequence (NotePair) - applies note and velocity changes");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 1.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 2.0);
            seq.updateMatchedPairs();
            std::vector<TrackIOHelper::NotePair> pairs;
            TrackIOHelper::extractNotePairEvents(seq, pairs);
            std::unordered_map<int, MidiChangeInfo> changes;
            MidiChangeInfo info;
            info.oldTimeStamp = 1.0;
            info.newTimeStamp = 1.0;
            info.newNumber = 67;
            info.newVelocity = 110;
            changes[0] = info;
            TrackIOHelper::applyChangesToASequence(pairs, changes);
            expect(pairs[0].noteOn->getNoteNumber() == 67);
            expect(pairs[0].noteOn->getVelocity() == 110);
            expect(pairs[0].noteOff->getNoteNumber() == 67);
        }
    }

    void testApplyChangesNotePairTimestampShift()
    {
        beginTest("applyChangesToASequence (NotePair) - shifts timestamp by delta");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 5.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 6.0);
            seq.updateMatchedPairs();
            std::vector<TrackIOHelper::NotePair> pairs;
            TrackIOHelper::extractNotePairEvents(seq, pairs);
            std::unordered_map<int, MidiChangeInfo> changes;
            MidiChangeInfo info;
            info.oldTimeStamp = 5.0;
            info.newTimeStamp = 3.0;
            info.newNumber = 60;
            info.newVelocity = 100;
            changes[0] = info;
            TrackIOHelper::applyChangesToASequence(pairs, changes);
            expect(std::abs(pairs[0].noteOn->getTimeStamp() - 3.0) < 0.001);
            expect(std::abs(pairs[0].noteOff->getTimeStamp() - 4.0) < 0.001);
        }
    }

    void testApplyChangesNotePairClampToZero()
    {
        beginTest("applyChangesToASequence (NotePair) - clamps to 0 when shifted before start");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 1.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 2.0);
            seq.updateMatchedPairs();
            std::vector<TrackIOHelper::NotePair> pairs;
            TrackIOHelper::extractNotePairEvents(seq, pairs);
            std::unordered_map<int, MidiChangeInfo> changes;
            MidiChangeInfo info;
            info.oldTimeStamp = 1.0;
            info.newTimeStamp = 5.0;

            info.oldTimeStamp = 1.0;
            info.newTimeStamp = 3.0;

            info.oldTimeStamp = 5.0;
            info.newTimeStamp = 0.0;
            info.newNumber = 60;
            info.newVelocity = 100;
            changes[0] = info;
            TrackIOHelper::applyChangesToASequence(pairs, changes);
            expect(pairs[0].noteOn->getTimeStamp() == 0.0);
            expect(std::abs(pairs[0].noteOff->getTimeStamp() - 1.0) < 0.001);
        }
    }

    void testApplyChangesNotePairOutOfRange()
    {
        beginTest("applyChangesToASequence (NotePair) - out of range row is ignored");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 1.0);
            seq.updateMatchedPairs();
            std::vector<TrackIOHelper::NotePair> pairs;
            TrackIOHelper::extractNotePairEvents(seq, pairs);
            std::unordered_map<int, MidiChangeInfo> changes;
            MidiChangeInfo info;
            info.newNumber = 72;
            info.newVelocity = 50;
            info.oldTimeStamp = 0.0;
            info.newTimeStamp = 0.0;
            changes[99] = info;
            TrackIOHelper::applyChangesToASequence(pairs, changes);
            expect(pairs[0].noteOn->getNoteNumber() == 60);
        }
    }

    void testApplyChangesSequenceBasic()
    {
        beginTest("applyChangesToASequence - applies note number and velocity changes");
        {
            juce::MidiMessageSequence seq;
            auto on = juce::MidiMessage::noteOn(1, 60, (juce::uint8)100);
            on.setTimeStamp(1.0);
            auto off = juce::MidiMessage::noteOff(1, 60);
            off.setTimeStamp(2.0);
            seq.addEvent(on);
            seq.addEvent(off);
            seq.updateMatchedPairs();

            std::unordered_map<int, MidiChangeInfo> changes;
            MidiChangeInfo info;
            info.oldNumber = 60;
            info.oldTimeStamp = 1.0;
            info.oldVelocity = 100;
            info.newNumber = 64;
            info.newTimeStamp = 1.0;
            info.newVelocity = 80;
            changes[0] = info;

            TrackIOHelper::applyChangesToASequence(seq, changes);


            bool found = false;
            for (int i = 0; i < seq.getNumEvents(); ++i)
            {
                auto& msg = seq.getEventPointer(i)->message;
                if (msg.isNoteOn())
                {
                    expect(msg.getNoteNumber() == 64);
                    expect(msg.getVelocity() == 80);
                    found = true;
                    break;
                }
            }
            expect(found);
        }
    }

    void testApplyChangesSequenceOutOfRange()
    {
        beginTest("applyChangesToASequence - out of range row is ignored");
        {
            juce::MidiMessageSequence seq;
            auto on = juce::MidiMessage::noteOn(1, 60, (juce::uint8)100);
            on.setTimeStamp(0.0);
            auto off = juce::MidiMessage::noteOff(1, 60);
            off.setTimeStamp(1.0);
            seq.addEvent(on);
            seq.addEvent(off);
            seq.updateMatchedPairs();

            std::unordered_map<int, MidiChangeInfo> changes;
            MidiChangeInfo info;
            info.newNumber = 64;
            info.newVelocity = 80;
            info.oldTimeStamp = 0.0;
            info.newTimeStamp = 0.0;
            changes[99] = info;

            TrackIOHelper::applyChangesToASequence(seq, changes);

            expect(seq.getEventPointer(0)->message.getNoteNumber() == 60);
        }
    }

    void testExtractNotePairsBasic()
    {
        beginTest("extractNotePairEvents - extracts matched note on/off pairs");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 60), 1.0);
            seq.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)90), 1.0);
            seq.addEvent(juce::MidiMessage::noteOff(1, 64), 2.0);
            seq.updateMatchedPairs();

            std::vector<TrackIOHelper::NotePair> pairs;
            TrackIOHelper::extractNotePairEvents(seq, pairs);

            expect((int)pairs.size() == 2);
            expect(pairs[0].noteOn->getNoteNumber() == 60);
            expect(pairs[0].noteOff->getNoteNumber() == 60);
            expect(pairs[1].noteOn->getNoteNumber() == 64);
            expect(pairs[1].noteOff->getNoteNumber() == 64);
        }
    }

    void testExtractNotePairsEmpty()
    {
        beginTest("extractNotePairEvents - empty sequence gives empty result");
        {
            juce::MidiMessageSequence seq;
            std::vector<TrackIOHelper::NotePair> pairs;
            TrackIOHelper::extractNotePairEvents(seq, pairs);
            expect(pairs.empty());
        }
    }

    void testSaveLoadBasicRoundtrip()
    {
        beginTest("saveToFile and loadFromFile - basic roundtrip");
        {

            auto tempMidiFile = juce::File::createTempFile(".mid");
            {
                juce::MidiFile midiFile;
                midiFile.setTicksPerQuarterNote(480);

                juce::MidiMessageSequence track;
                track.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
                track.addEvent(juce::MidiMessage::noteOff(1, 60), 480.0);
                track.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)90), 480.0);
                track.addEvent(juce::MidiMessage::noteOff(1, 64), 960.0);

                midiFile.addTrack(track);

                juce::FileOutputStream outStream(tempMidiFile);
                if (outStream.openedOk())
                    midiFile.writeTo(outStream);
            }


            std::unordered_map<juce::String, std::deque<TrackEntry>> groupedTracks;

            TrackEntry entry;
            entry.file = tempMidiFile;
            entry.trackIndex = 0;
            entry.displayName = "Piano";
            entry.folderName = "MyFolder";
            entry.uuid = TrackEntry::generateUUID();

            groupedTracks["MyFolder"].push_back(entry);


            auto tempJsonFile = juce::File::createTempFile(".json");
            TrackIOHelper::saveToFile(tempJsonFile, groupedTracks);


            std::unordered_map<juce::String, std::deque<TrackEntry>> loadedTracks;
            std::vector<juce::String> loadedKeys;
            TrackIOHelper::loadFromFile(tempJsonFile, loadedTracks, loadedKeys);


            expect((int)loadedKeys.size() == 1);
            expect(loadedKeys[0] == "MyFolder");
            expect(loadedTracks.count("MyFolder") == 1);
            expect((int)loadedTracks["MyFolder"].size() == 1);

            auto& loadedEntry = loadedTracks["MyFolder"][0];
            expect(loadedEntry.displayName == "Piano");
            expect(loadedEntry.trackIndex == 0);
            expect(loadedEntry.folderName == "MyFolder");

            tempJsonFile.deleteFile();
            tempMidiFile.deleteFile();
        }
    }

    void testSaveLoadWithChangesMap()
    {
        beginTest("saveToFile and loadFromFile - with non empty changes map");
        {

            auto tempMidiFile = juce::File::createTempFile(".mid");
            {
                juce::MidiFile midiFile;
                midiFile.setTicksPerQuarterNote(480);

                juce::MidiMessageSequence track;
                track.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
                track.addEvent(juce::MidiMessage::noteOff(1, 60), 480.0);
                track.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)90), 480.0);
                track.addEvent(juce::MidiMessage::noteOff(1, 64), 960.0);

                midiFile.addTrack(track);

                juce::FileOutputStream outStream(tempMidiFile);
                if (outStream.openedOk())
                    midiFile.writeTo(outStream);
            }


            std::unordered_map<juce::String, std::deque<TrackEntry>> groupedTracks;

            TrackEntry entry;
            entry.file = tempMidiFile;
            entry.trackIndex = 0;
            entry.displayName = "Piano";
            entry.folderName = "MyFolder";
            entry.uuid = TrackEntry::generateUUID();

            std::unordered_map<juce::String, std::unordered_map<int, MidiChangeInfo>> changesMap;

            MidiChangeInfo newChange;

            newChange.oldVelocity = 80;
            newChange.oldNumber = 63;
            newChange.oldTimeStamp = 20.5;

            newChange.oldBPMchange = 120.0;

            newChange.newBPMchange = 150.0;
            newChange.newTimeStamp = 20.5;
            newChange.newNumber = 63;
            newChange.newVelocity = 90;

            std::unordered_map<int, MidiChangeInfo> innerMap;
            innerMap[1] = newChange;

            changesMap["Style1"] = innerMap;
            entry.styleChangesMap = changesMap;

            groupedTracks["MyFolder"].push_back(entry);


            auto tempJsonFile = juce::File::createTempFile(".json");
            TrackIOHelper::saveToFile(tempJsonFile, groupedTracks);


            std::unordered_map<juce::String, std::deque<TrackEntry>> loadedTracks;
            std::vector<juce::String> loadedKeys;
            TrackIOHelper::loadFromFile(tempJsonFile, loadedTracks, loadedKeys);



            expect((int)loadedKeys.size() == 1);
            expect(loadedKeys[0] == "MyFolder");
            expect(loadedTracks.count("MyFolder") == 1);
            expect((int)loadedTracks["MyFolder"].size() == 1);

            auto& loadedEntry = loadedTracks["MyFolder"][0];
            expect(loadedEntry.displayName == "Piano");
            expect(loadedEntry.trackIndex == 0);
            expect(loadedEntry.folderName == "MyFolder");

            expect(loadedEntry.styleChangesMap.count("Style1") == 1);
            expect(loadedEntry.styleChangesMap["Style1"].count(1) == 1);


            auto& loadedChange = loadedEntry.styleChangesMap["Style1"][1];
            expect(loadedChange.oldVelocity == 80);
            expect(loadedChange.oldNumber == 63);
            expect(std::abs(loadedChange.oldTimeStamp - 20.5) < 0.001);
            expect(std::abs(loadedChange.oldBPMchange - 120.0) < 0.001);
            expect(std::abs(loadedChange.newBPMchange - 150.0) < 0.001);
            expect(std::abs(loadedChange.newTimeStamp - 20.5) < 0.001);
            expect(loadedChange.newNumber == 63);
            expect(loadedChange.newVelocity == 90);

            tempJsonFile.deleteFile();
            tempMidiFile.deleteFile();
        }
    }

    void testSaveLoadEmptyFolder()
    {
        beginTest("saveToFile and loadFromFile - empty folder survives save-load");
        {
            auto tempJsonFile = juce::File::createTempFile(".json");

            std::unordered_map<juce::String, std::deque<TrackEntry>> groupedTracks;
            groupedTracks["EmptyFolder"] = {};  // folder with no tracks

            TrackIOHelper::saveToFile(tempJsonFile, groupedTracks);

            std::unordered_map<juce::String, std::deque<TrackEntry>> loadedTracks;
            std::vector<juce::String> loadedKeys;
            TrackIOHelper::loadFromFile(tempJsonFile, loadedTracks, loadedKeys);

            expect((int)loadedKeys.size() == 1);
            expect(loadedKeys[0] == "EmptyFolder");
            expect(loadedTracks["EmptyFolder"].empty());

            tempJsonFile.deleteFile();
        }
    }

    void testLoadFromNonExistentFile()
    {
        beginTest("loadFromFile - non-existent file does nothing");
        {
            juce::File nonExistent("C:\\this_track_file_does_not_exist.json");
            std::unordered_map<juce::String, std::deque<TrackEntry>> loadedTracks;
            std::vector<juce::String> loadedKeys;

            TrackIOHelper::loadFromFile(nonExistent, loadedTracks, loadedKeys);

            expect(loadedTracks.empty());
            expect(loadedKeys.empty());
        }
    }

    void testSaveLoadMultipleFolders()
    {
        beginTest("saveToFile and loadFromFile - multiple folders with tracks");
        {
            //create two temp MIDI files
            auto tempMidi1 = juce::File::createTempFile(".mid");
            auto tempMidi2 = juce::File::createTempFile(".mid");
            {
                juce::MidiFile mf;
                mf.setTicksPerQuarterNote(480);
                juce::MidiMessageSequence t;
                t.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0.0);
                t.addEvent(juce::MidiMessage::noteOff(1, 60), 480.0);
                mf.addTrack(t);

                juce::FileOutputStream os1(tempMidi1);
                if (os1.openedOk()) mf.writeTo(os1);
            }
            {
                juce::MidiFile mf;
                mf.setTicksPerQuarterNote(480);
                juce::MidiMessageSequence t;
                t.addEvent(juce::MidiMessage::noteOn(10, 36, (juce::uint8)80), 0.0);
                t.addEvent(juce::MidiMessage::noteOff(10, 36), 240.0);
                mf.addTrack(t);

                juce::FileOutputStream os2(tempMidi2);
                if (os2.openedOk()) mf.writeTo(os2);
            }

            std::unordered_map<juce::String, std::deque<TrackEntry>> groupedTracks;

            TrackEntry e1;
            e1.file = tempMidi1;
            e1.trackIndex = 0;
            e1.displayName = "Melody";
            e1.folderName = "Songs";
            groupedTracks["Songs"].push_back(e1);

            TrackEntry e2;
            e2.file = tempMidi2;
            e2.trackIndex = 0;
            e2.displayName = "Drums";
            e2.folderName = "Beats";
            groupedTracks["Beats"].push_back(e2);

            auto tempJsonFile = juce::File::createTempFile(".json");
            TrackIOHelper::saveToFile(tempJsonFile, groupedTracks);

            std::unordered_map<juce::String, std::deque<TrackEntry>> loadedTracks;
            std::vector<juce::String> loadedKeys;
            TrackIOHelper::loadFromFile(tempJsonFile, loadedTracks, loadedKeys);

            expect((int)loadedKeys.size() == 2);
            expect(loadedTracks.count("Songs") == 1);
            expect(loadedTracks.count("Beats") == 1);
            expect(loadedTracks["Songs"][0].displayName == "Melody");
            expect(loadedTracks["Beats"][0].displayName == "Drums");

            //the drums track should be detected as percussion (channel 10)
            expect(loadedTracks["Beats"][0].type == TrackType::Percussion);

            tempJsonFile.deleteFile();
            tempMidi1.deleteFile();
            tempMidi2.deleteFile();
        }
    }
};

static TrackIOHelperTest trackIOHelperTest;




class PlaybackSettingsIOHelperTest : public juce::UnitTest
{
public:
    PlaybackSettingsIOHelperTest() : juce::UnitTest("PlaybackSettingsIOHelper", "Unit") {}

    void runTest() override
    {
        saveAndLoadFullRangeKeyboard();
        loadWithWrongVIDPID();
        loadFromNonExistentFile();
        saveWithSmallKeyboardRange();
    }

private:
    void saveAndLoadFullRangeKeyboard()
    {
        beginTest("Save and load roundtrip - full range keyboard");

        auto tempFile = juce::File::createTempFile(".json");

        PlayBackSettings original;
        original.startNote = 36;
        original.endNote = 96;
        original.leftHandBound = 60;
        original.rightHandBound = 72;
        original.VID = "1234";
        original.PID = "5678";

        PlaybackSettingsIOHelper::saveToFile(tempFile, original, 21, 108);

        PlayBackSettings loaded = PlaybackSettingsIOHelper::loadFromFile(tempFile, "1234", "5678");

        expect(loaded.startNote == 36);
        expect(loaded.endNote == 96);
        expect(loaded.leftHandBound == 60);
        expect(loaded.rightHandBound == 72);
        expect(loaded.VID == "1234");
        expect(loaded.PID == "5678");

        tempFile.deleteFile();
    }

    void loadWithWrongVIDPID()
    {
        beginTest("Load with wrong VID/PID returns defaults");

        auto tempFile = juce::File::createTempFile(".json");

        PlayBackSettings original;
        original.startNote = 36;
        original.endNote = 96;
        original.leftHandBound = 60;
        original.rightHandBound = 72;
        original.VID = "1234";
        original.PID = "5678";

        PlaybackSettingsIOHelper::saveToFile(tempFile, original, 21, 108);

        PlayBackSettings loaded = PlaybackSettingsIOHelper::loadFromFile(tempFile, "9999", "0000");

        expect(loaded.startNote == -1);
        expect(loaded.endNote == -1);

        tempFile.deleteFile();
    }

    void loadFromNonExistentFile()
    {
        beginTest("Load from non-existent file returns defaults");

        juce::File nonExistent("C:\\this_file_does_not_exist_12345.json");
        PlayBackSettings loaded = PlaybackSettingsIOHelper::loadFromFile(nonExistent, "1234", "5678");

        expect(loaded.startNote == -1);
        expect(loaded.endNote == -1);
        expect(loaded.leftHandBound == -1);
        expect(loaded.rightHandBound == -1);
    }

    void saveWithSmallKeyboardRange()
    {
        beginTest("Save with small keyboard range clamps to base octave");

        auto tempFile = juce::File::createTempFile(".json");

        PlayBackSettings original;
        original.startNote = 48;   // C3
        original.endNote = 72;     // C5
        original.leftHandBound = 55;
        original.rightHandBound = 65;
        original.VID = "AAAA";
        original.PID = "BBBB";

        // highest - lowest = 72 - 48 = 24, which is < 49, so clamping kicks in
        PlaybackSettingsIOHelper::saveToFile(tempFile, original, 48, 72);

        PlayBackSettings loaded = PlaybackSettingsIOHelper::loadFromFile(tempFile, "AAAA", "BBBB");

        // Values should be remapped to base 60:  60 + (note % 12)
        expect(loaded.startNote == 60 + (48 % 12));
        expect(loaded.endNote == 60 + (72 % 12));

        tempFile.deleteFile();
    }
};

static PlaybackSettingsIOHelperTest playbackSettingsIOHelperTest;



class SectionIOHelperTest : public juce::UnitTest
{
public:
    SectionIOHelperTest() : juce::UnitTest("SectionIOHelper", "Unit") {}

    void runTest() override
    {
        saveAndLoadRoundtrip();
        loadFromNonExistentFile();
        loadFromInvalidJson();
        multipleStylesSaveAndLoad();
    }

private:
    void saveAndLoadRoundtrip()
    {
        beginTest("Save and load roundtrip");

        auto tempFile = juce::File::createTempFile(".json");

        std::unordered_map<juce::String, std::unordered_map<juce::String, StyleSection>> original;

        StyleSection intro;
        intro.id = "s1";
        intro.name = "Intro";
        intro.startTimeSeconds = 0.0;
        intro.endTimeSeconds = 8.5;
        intro.startBar = 1;
        intro.endBar = 4;

        StyleSection verse;
        verse.id = "s2";
        verse.name = "Verse";
        verse.startTimeSeconds = 8.5;
        verse.endTimeSeconds = 25.0;
        verse.startBar = 5;
        verse.endBar = 12;

        original["style_A"]["Intro"] = intro;
        original["style_A"]["Verse"] = verse;

        SectionIOHelper::saveToFile(tempFile, original);

        std::unordered_map<juce::String, std::unordered_map<juce::String, StyleSection>> loaded;
        SectionIOHelper::loadFromFile(tempFile, loaded);

        expect(loaded.count("style_A") == 1);
        expect(loaded["style_A"].count("Intro") == 1);
        expect(loaded["style_A"].count("Verse") == 1);

        expect(loaded["style_A"]["Intro"].id == "s1");
        expect(loaded["style_A"]["Intro"].name == "Intro");
        expect(std::abs(loaded["style_A"]["Intro"].startTimeSeconds - 0.0) < 0.001);
        expect(std::abs(loaded["style_A"]["Intro"].endTimeSeconds - 8.5) < 0.001);
        expect(loaded["style_A"]["Intro"].startBar == 1);
        expect(loaded["style_A"]["Intro"].endBar == 4);

        expect(loaded["style_A"]["Verse"].id == "s2");
        expect(loaded["style_A"]["Verse"].startBar == 5);

        tempFile.deleteFile();
    }

    void loadFromNonExistentFile()
    {
        beginTest("Load from non-existent file gives empty map");

        juce::File nonExistent("C:\\this_section_file_does_not_exist.json");
        std::unordered_map<juce::String, std::unordered_map<juce::String, StyleSection>> loaded;
        SectionIOHelper::loadFromFile(nonExistent, loaded);
        expect(loaded.empty());
    }

    void loadFromInvalidJson()
    {
        beginTest("Load from empty/invalid JSON gives empty map");

        auto tempFile = juce::File::createTempFile(".json");
        tempFile.replaceWithText("not valid json");

        std::unordered_map<juce::String, std::unordered_map<juce::String, StyleSection>> loaded;
        SectionIOHelper::loadFromFile(tempFile, loaded);
        expect(loaded.empty());

        tempFile.deleteFile();
    }

    void multipleStylesSaveAndLoad()
    {
        beginTest("Multiple styles save and load correctly");

        auto tempFile = juce::File::createTempFile(".json");

        std::unordered_map<juce::String, std::unordered_map<juce::String, StyleSection>> original;

        StyleSection a;
        a.id = "a1"; a.name = "PartA";
        a.startTimeSeconds = 0.0; a.endTimeSeconds = 10.0;
        a.startBar = 1; a.endBar = 8;

        StyleSection b;
        b.id = "b1"; b.name = "PartB";
        b.startTimeSeconds = 0.0; b.endTimeSeconds = 5.0;
        b.startBar = 1; b.endBar = 4;

        original["Rock"]["PartA"] = a;
        original["Jazz"]["PartB"] = b;

        SectionIOHelper::saveToFile(tempFile, original);

        std::unordered_map<juce::String, std::unordered_map<juce::String, StyleSection>> loaded;
        SectionIOHelper::loadFromFile(tempFile, loaded);

        expect(loaded.count("Rock") == 1);
        expect(loaded.count("Jazz") == 1);
        expect(loaded["Rock"]["PartA"].id == "a1");
        expect(loaded["Jazz"]["PartB"].id == "b1");

        tempFile.deleteFile();
    }

};

static SectionIOHelperTest sectionIOHelperTest;
