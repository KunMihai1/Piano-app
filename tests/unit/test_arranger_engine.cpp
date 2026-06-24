#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "Arranger/ArrangerEngine.h"
#include "Arranger/ArrangerModel.h"

class ArrangerEngineTest : public juce::UnitTest
{
public:
    ArrangerEngineTest() : juce::UnitTest ("ArrangerEngine", "Arranger") {}

    static ArrangerStyle makeStyle()
    {
        ArrangerTrack t;
        t.channel = 2;
        t.partType = ArrangerPartType::Acc;
        t.pattern = {
            { 0.0, juce::MidiMessage::noteOn  (2, 60, (juce::uint8) 100) },
            { 0.5, juce::MidiMessage::noteOff (2, 60) },
        };
        ArrangerSection s; s.lengthBars = 1; s.tracks.push_back (t);
        ArrangerStyle style; style.timeSigNum = 4; style.timeSigDenom = 4; style.originalTempo = 120.0;
        style.sections.push_back (s);
        return style;
    }

    // Two one-bar variations on channel 2. Variation 1 holds a note across the bar
    // (no note-off) so a section switch must flush it; Variation 2 plays a short note.
    static ArrangerStyle makeTwoVarStyle()
    {
        ArrangerTrack v1; v1.channel = 2; v1.partType = ArrangerPartType::Acc;
        v1.pattern = { { 0.0, juce::MidiMessage::noteOn (2, 60, (juce::uint8) 100) } }; // held
        ArrangerSection s1; s1.id = "var_1"; s1.name = "Variation 1";
        s1.type = ArrangerSectionType::Variation; s1.lengthBars = 1; s1.tracks.push_back (v1);

        ArrangerTrack v2; v2.channel = 2; v2.partType = ArrangerPartType::Acc;
        v2.pattern = { { 0.0, juce::MidiMessage::noteOn  (2, 67, (juce::uint8) 100) },
                       { 0.5, juce::MidiMessage::noteOff (2, 67) } };
        ArrangerSection s2; s2.id = "var_2"; s2.name = "Variation 2";
        s2.type = ArrangerSectionType::Variation; s2.lengthBars = 1; s2.tracks.push_back (v2);

        ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4; st.originalTempo = 120.0;
        st.sections.push_back (s1); st.sections.push_back (s2);
        return st;
    }

    static ArrangerStyle makeEndingStyle()
    {
        ArrangerTrack v; v.channel = 2; v.partType = ArrangerPartType::Acc;
        v.pattern = { { 0.0, juce::MidiMessage::noteOn  (2, 60, (juce::uint8) 100) },
                      { 0.5, juce::MidiMessage::noteOff (2, 60) } };
        ArrangerSection var; var.id = "var_1"; var.name = "Variation 1";
        var.type = ArrangerSectionType::Variation; var.lengthBars = 1; var.tracks.push_back (v);

        ArrangerSection end; end.id = "ending_1"; end.name = "Ending 1";
        end.type = ArrangerSectionType::Ending; end.lengthBars = 1;
        end.afterComplete = ArrangerAfterComplete::Stop; end.tracks.push_back (v);

        ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4; st.originalTempo = 120.0;
        st.sections.push_back (var); st.sections.push_back (end);
        return st;
    }

    // One section: an Acc track (ch2, note 60) + a Drum track (ch10, note 36), both short.
    static ArrangerStyle makeAccAndDrumStyle()
    {
        ArrangerTrack acc; acc.channel = 2; acc.partType = ArrangerPartType::Acc;
        acc.pattern = { { 0.0, juce::MidiMessage::noteOn  (2, 60, (juce::uint8) 100) },
                        { 0.5, juce::MidiMessage::noteOff (2, 60) } };
        ArrangerTrack drum; drum.channel = 10; drum.partType = ArrangerPartType::Drum;
        drum.pattern = { { 0.0, juce::MidiMessage::noteOn  (10, 36, (juce::uint8) 100) },
                         { 0.5, juce::MidiMessage::noteOff (10, 36) } };
        ArrangerSection s; s.id = "var_1"; s.name = "Variation 1";
        s.type = ArrangerSectionType::Variation; s.lengthBars = 1;
        s.tracks.push_back (acc); s.tracks.push_back (drum);
        ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4; st.originalTempo = 120.0;
        st.sections.push_back (s);
        return st;
    }

    // One section: an Acc track holding note 60 across the bar (no note-off) -> closed at the seam.
    static ArrangerStyle makeAccHeldNoteStyle()
    {
        ArrangerTrack acc; acc.channel = 2; acc.partType = ArrangerPartType::Acc;
        acc.pattern = { { 0.0, juce::MidiMessage::noteOn (2, 60, (juce::uint8) 100) } };
        ArrangerSection s; s.id = "var_1"; s.name = "Variation 1";
        s.type = ArrangerSectionType::Variation; s.lengthBars = 1; s.tracks.push_back (acc);
        ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4; st.originalTempo = 120.0;
        st.sections.push_back (s);
        return st;
    }

    void runTest() override
    {
        beginTest ("renderRange dispatches scheduled events through onMidiMessage");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            std::vector<juce::MidiMessage> captured;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m) { captured.push_back (m); };

            engine.setStyle (makeStyle());
            engine.renderRange (0.0, 1.0);   // beats [0,1): note on then off

            expectEquals ((int) captured.size(), 2);
            expect (captured[0].isNoteOn());
            expectEquals (captured[0].getNoteNumber(), 60);
            expect (captured[1].isNoteOff());
        }

        beginTest ("loop length in beats reflects the section bars");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            engine.setStyle (makeStyle());
            expectWithinAbsoluteError (engine.getLoopLengthBeats(), 4.0, 1e-9); // 1 bar of 4/4
        }

        beginTest ("stop sends all-notes-off and is reflected in isPlaying");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            int noteOffCount = 0;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m)
            {
                if (m.isAllNotesOff()) ++noteOffCount;
            };
            engine.setStyle (makeStyle());
            engine.stop();
            expect (! engine.isPlaying());
            expect (noteOffCount >= 1);
        }

        beginTest ("queueSection switches active section only after the bar boundary");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            engine.setStyle (makeTwoVarStyle());
            engine.queueSection (ArrangerSectionType::Variation, "Variation 2");

            expectEquals (engine.getActiveSectionIndex(), 0);
            engine.renderRange (0.0, 2.0);                    // no boundary yet
            expectEquals (engine.getActiveSectionIndex(), 0);
            engine.renderRange (2.0, 5.0);                    // crosses beat 4
            expectEquals (engine.getActiveSectionIndex(), 1);
        }

        beginTest ("a section swap flushes the outgoing section's held note (no hung notes)");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            std::vector<juce::MidiMessage> captured;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m) { captured.push_back (m); };

            engine.setStyle (makeTwoVarStyle());
            engine.queueSection (ArrangerSectionType::Variation, "Variation 2");
            engine.renderRange (0.0, 1.0);   // Variation 1 note 60 on (held)
            engine.renderRange (1.0, 5.0);   // boundary -> swap to Variation 2

            int offIdx = -1, onIdx = -1;
            for (int i = 0; i < (int) captured.size(); ++i)
            {
                if (captured[i].isNoteOff() && captured[i].getNoteNumber() == 60) offIdx = i;
                if (captured[i].isNoteOn()  && captured[i].getNoteNumber() == 67 && onIdx < 0) onIdx = i;
            }
            expect (offIdx >= 0);            // held note 60 was closed at the swap
            expect (onIdx  >= 0);            // Variation 2 note 67 started
            expect (offIdx < onIdx);         // closed before the new section sounded
        }

        beginTest ("an Ending completing silences output (all-notes-off)");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            int allNotesOff = 0;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m)
            {
                if (m.isAllNotesOff()) ++allNotesOff;
            };
            engine.setStyle (makeEndingStyle());
            engine.queueSection (ArrangerSectionType::Ending, "Ending 1");
            engine.renderRange (0.0, 5.0);   // enter ending at beat 4
            engine.renderRange (5.0, 9.0);   // ending completes at beat 8 -> stop
            expect (allNotesOff >= 1);
        }

        beginTest ("selectStartSection makes start begin on the chosen section");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            engine.setStyle (makeTwoVarStyle());
            engine.selectStartSection (ArrangerSectionType::Variation, "Variation 2");
            expectEquals (engine.peekPendingStartIndex(), 1);
        }

        beginTest ("active chord transposes Acc notes but leaves Drum notes unchanged");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            std::vector<juce::MidiMessage> captured;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m) { captured.push_back (m); };

            engine.setStyle (makeAccAndDrumStyle());                 // original = C major (default)
            engine.setActiveChord ({ 2, ChordQuality::Maj, 2 });    // play D major (+2)
            engine.renderRange (0.0, 1.0);

            bool accShifted = false, drumKept = false;
            for (auto& m : captured)
            {
                if (m.isNoteOn() && m.getChannel() == 2  && m.getNoteNumber() == 62) accShifted = true;
                if (m.isNoteOn() && m.getChannel() == 10 && m.getNoteNumber() == 36) drumKept   = true;
            }
            expect (accShifted, "Acc note 60 should shift to 62 under D major");
            expect (drumKept,   "Drum note 36 should be unchanged");
        }

        beginTest ("transposed note-off matches the transposed note-on pitch (no hung notes)");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            std::vector<juce::MidiMessage> captured;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m) { captured.push_back (m); };

            engine.setStyle (makeAccHeldNoteStyle());               // original C major
            engine.setActiveChord ({ 2, ChordQuality::Maj, 2 });    // D major (+2): 60 -> 62
            engine.renderRange (0.0, 4.5);                          // cross the seam -> synthetic off

            int on62 = 0, off62 = 0, on60 = 0, off60 = 0;
            for (auto& m : captured)
            {
                if (m.getChannel() == 2 && m.isNoteOn()  && m.getNoteNumber() == 62) ++on62;
                if (m.getChannel() == 2 && m.isNoteOff() && m.getNoteNumber() == 62) ++off62;
                if (m.getChannel() == 2 && m.isNoteOn()  && m.getNoteNumber() == 60) ++on60;
                if (m.getChannel() == 2 && m.isNoteOff() && m.getNoteNumber() == 60) ++off60;
            }
            expect (on62  >= 1, "note-on should sound the transposed pitch 62");
            expect (off62 >= 1, "the seam note-off must use the same transposed pitch 62");
            expectEquals (on60,  0);   // nothing at the original pitch
            expectEquals (off60, 0);
        }
    }
};

static ArrangerEngineTest arrangerEngineTest;
