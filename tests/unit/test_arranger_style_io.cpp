#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ArrangerStyleIOHelper.h"
#include "Arranger/ArrangerPatternBuilder.h"
#include "Arranger/ArrangerEngine.h"

static ArrangerStyleFile makeSample()
{
    ArrangerStyleFile s;
    s.name = "Sunset Groove";
    s.originalTempo = 120.0;
    s.timeSigNum = 4; s.timeSigDenom = 4;

    SourceTrackFile drums;
    drums.id = "t1"; drums.name = "Drums";
    drums.partType = ArrangerPartType::Drum;
    drums.channel = 10; drums.instrument = -1; drums.volume = 100.0;
    drums.events.push_back ({ 0.0, juce::MidiMessage::noteOn  (10, 36, (juce::uint8) 100) });
    drums.events.push_back ({ 0.5, juce::MidiMessage::noteOff (10, 36) });
    s.sourceTracks.push_back (drums);

    SectionWindow var;
    var.id = "var_1"; var.name = "Variation 1";
    var.type = ArrangerSectionType::Variation;
    var.startBar = 5; var.lengthBars = 8;
    var.afterComplete = ArrangerAfterComplete::Loop;
    s.sections.push_back (var);
    return s;
}

class ArrangerStyleIOTest : public juce::UnitTest
{
public:
    ArrangerStyleIOTest() : juce::UnitTest ("ArrangerStyleIO", "Arranger") {}

    void runTest() override
    {
        beginTest ("save then load reproduces the style");
        {
            auto file = juce::File::createTempFile (".style");
            auto original = makeSample();
            ArrangerStyleIOHelper::saveToFile (file, original);

            ArrangerStyleFile loaded; juce::String err;
            const bool ok = ArrangerStyleIOHelper::loadFromFile (file, loaded, err);
            expect (ok, err);

            expectEquals (loaded.schemaVersion, 3);
            expectEquals (loaded.name, juce::String ("Sunset Groove"));
            expectEquals (loaded.timeSigNum, 4);
            expectEquals ((int) loaded.sourceTracks.size(), 1);
            expectEquals ((int) loaded.sections.size(), 1);

            auto& tr = loaded.sourceTracks[0];
            expectEquals (tr.channel, 10);
            expect (tr.partType == ArrangerPartType::Drum);
            expectEquals ((int) tr.events.size(), 2);
            expect (tr.events[0].message.isNoteOn());
            expectEquals (tr.events[0].message.getNoteNumber(), 36);
            expectWithinAbsoluteError (tr.events[1].beats, 0.5, 1e-9);

            auto& sec = loaded.sections[0];
            expect (sec.type == ArrangerSectionType::Variation);
            expectEquals (sec.startBar, 5);
            expectEquals (sec.lengthBars, 8);
            expect (sec.afterComplete == ArrangerAfterComplete::Loop);

            file.deleteFile();
        }

        beginTest ("loading a missing file fails gracefully");
        {
            // Use a non-existent ABSOLUTE path: juce::File asserts on relative paths.
            auto missing = juce::File::getSpecialLocation (juce::File::tempDirectory)
                               .getChildFile ("arranger-style-does-not-exist-9d3f.style");
            missing.deleteFile();

            ArrangerStyleFile loaded; juce::String err;
            const bool ok = ArrangerStyleIOHelper::loadFromFile (missing, loaded, err);
            expect (! ok);
            expect (err.isNotEmpty());
        }

        beginTest ("file -> buildStyleFromFile -> engine renders the first section");
        {
            // Build a 1-bar variation with a single note, save, reload, build, render one bar.
            ArrangerStyleFile f;
            f.name = "Smoke"; f.originalTempo = 120.0; f.timeSigNum = 4; f.timeSigDenom = 4;
            SourceTrackFile t; t.id = "t"; t.partType = ArrangerPartType::Acc; t.channel = 2;
            t.events.push_back ({ 0.0, juce::MidiMessage::noteOn  (2, 60, (juce::uint8) 100) });
            t.events.push_back ({ 1.0, juce::MidiMessage::noteOff (2, 60) });
            f.sourceTracks.push_back (t);
            SectionWindow w; w.id = "v"; w.type = ArrangerSectionType::Variation;
            w.startBar = 1; w.lengthBars = 1; w.afterComplete = ArrangerAfterComplete::Loop;
            f.sections.push_back (w);

            auto file = juce::File::createTempFile (".style");
            ArrangerStyleIOHelper::saveToFile (file, f);
            ArrangerStyleFile loaded; juce::String err;
            expect (ArrangerStyleIOHelper::loadFromFile (file, loaded, err), err);

            ArrangerStyle style = ArrangerPatternBuilder::buildStyleFromFile (loaded);

            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            int noteOns = 0;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m) { if (m.isNoteOn()) ++noteOns; };
            engine.setStyle (style);
            engine.renderRange (0.0, 4.0); // one 4/4 bar

            expect (noteOns >= 1);
            file.deleteFile();
        }
    }
};
static ArrangerStyleIOTest arrangerStyleIOTest;
