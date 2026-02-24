/*
  ==============================================================================

    HelperFunction.cpp
    Created: 12 Oct 2025 8:32:50pm
    Author:  Kisuke

  ==============================================================================
*/

#include "HelperFunctions.h"

juce::String MapHelper::intToStringNote(int note)
{
    static const juce::StringArray noteNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (note < 0 || note > 127)
        return "Invalid";

    int octave = (note / 12) - 1;
    juce::String name = noteNames[note % 12] + juce::String(octave);

    return name;

}

int MapHelper::stringToIntNote(juce::String noteString)
{

    noteString = noteString.trim().toUpperCase();
    static const juce::StringArray noteNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    int firstDigitIndex = -1;
    for (int i = 0; i < noteString.length(); ++i)
    {
        if (juce::CharacterFunctions::isDigit(noteString[i]) || noteString[i] == '-')
        {
            firstDigitIndex = i;
            break;
        }
    }

    if (firstDigitIndex == -1)
        return -1; 

    juce::String notePart = noteString.substring(0, firstDigitIndex);
    juce::String octavePart = noteString.substring(firstDigitIndex);

    int noteIndex = noteNames.indexOf(notePart);
    if (noteIndex == -1)
        return -1; 

    int octave = octavePart.getIntValue();
    int midiNote = (octave + 1) * 12 + noteIndex;

    if (midiNote < 0 || midiNote > 127)
        return -1;

    return midiNote;
}

std::vector<int> ChordHelper::getNotesForChord(const Chord& c)
{
    static std::map<juce::String, int> rootMap =
    {
        {"C", 60},
        {"C#", 61},
        {"D", 62},
        {"D#", 63},
        {"E", 64},
        {"F", 65},
        {"F#", 66},
        {"G", 67},
        {"G#", 68},
        {"A", 69},
        {"A#", 70},
        {"B", 71}
    };

    juce::String name = c.name;
    juce::String root;
    juce::String type;

    if (name.contains(" "))
    {
        root = name.upToFirstOccurrenceOf(" ", false, false);
        type = name.fromFirstOccurrenceOf(" ", false, false).trim();
    }
    else
    {
        root = name.dropLastCharacters(1);
        type = "7";
    }

    if (!rootMap.count(root))
        return {};

    int baseNote = rootMap[root];

    std::vector<int> intervals;

    if (type.equalsIgnoreCase("Major"))
        intervals = { 0, 4, 7 };

    else if (type.equalsIgnoreCase("Minor"))
        intervals = { 0, 3, 7 };

    else if (type.equalsIgnoreCase("7"))
        intervals = { 0, 4, 7, 10 };

    else if (type.equalsIgnoreCase("Augmented"))
        intervals = { 0, 4, 8 };

    else if (type.equalsIgnoreCase("Diminished"))
        intervals = { 0, 3, 6 };

    std::vector<int> notes;

    for (int interval : intervals)
        notes.push_back(baseNote + interval);

    return notes;
}

void ChordHelper::loadChordNeeded(Chord& c)
{
    if (c.imagesLoaded)
        return;

    juce::String name = c.name.toUpperCase();

    // ----- C -----
    if (name == "C MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_MAJ_ROOT_png, BinaryData::C_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_MAJ_INV1_png, BinaryData::C_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_MAJ_INV2_png, BinaryData::C_MAJ_INV2_pngSize);
    }
    else if (name == "C MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_MINOR_ROOT_png, BinaryData::C_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_MINOR_INV1_png, BinaryData::C_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_MINOR_INV2_png, BinaryData::C_MINOR_INV2_pngSize);
    }

    // ----- D -----
    else if (name == "D MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_MAJ_ROOT_png, BinaryData::D_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_MAJ_INV1_png, BinaryData::D_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_MAJ_INV2_png, BinaryData::D_MAJ_INV2_pngSize);
    }
    else if (name == "D MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_MINOR_ROOT_png, BinaryData::D_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_MINOR_INV1_png, BinaryData::D_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_MINOR_INV2_png, BinaryData::D_MINOR_INV2_pngSize);
    }

    // ----- E -----
    else if (name == "E MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::E_MAJ_ROOT_png, BinaryData::E_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::E_MAJ_INV1_png, BinaryData::E_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::E_MAJ_INV2_png, BinaryData::E_MAJ_INV2_pngSize);
    }
    else if (name == "E MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::E_MINOR_ROOT_png, BinaryData::E_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::E_MINOR_INV1_png, BinaryData::E_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::E_MINOR_INV2_png, BinaryData::E_MINOR_INV2_pngSize);
    }

    // ----- F -----
    else if (name == "F MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_MAJ_ROOT_png, BinaryData::F_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_MAJ_INV1_png, BinaryData::F_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_MAJ_INV2_png, BinaryData::F_MAJ_INV2_pngSize);
    }
    else if (name == "F MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_MINOR_ROOT_png, BinaryData::F_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_MINOR_INV1_png, BinaryData::F_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_MINOR_INV2_png, BinaryData::F_MINOR_INV2_pngSize);
    }

    // ----- G -----
    else if (name == "G MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_MAJ_ROOT_png, BinaryData::G_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_MAJ_INV1_png, BinaryData::G_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_MAJ_INV2_png, BinaryData::G_MAJ_INV2_pngSize);
    }
    else if (name == "G MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_MINOR_ROOT_png, BinaryData::G_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_MINOR_INV1_png, BinaryData::G_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_MINOR_INV2_png, BinaryData::G_MINOR_INV2_pngSize);
    }

    // ----- A -----
    else if (name == "A MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_MAJ_ROOT_png, BinaryData::A_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_MAJ_INV1_png, BinaryData::A_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_MAJ_INV2_png, BinaryData::A_MAJ_INV2_pngSize);
    }
    else if (name == "A MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_MINOR_ROOT_png, BinaryData::A_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_MINOR_INV1_png, BinaryData::A_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_MINOR_INV2_png, BinaryData::A_MINOR_INV2_pngSize);
    }

    // ----- B -----
    else if (name == "B MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::B_MAJ_ROOT_png, BinaryData::B_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::B_MAJ_INV1_png, BinaryData::B_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::B_MAJ_INV2_png, BinaryData::B_MAJ_INV2_pngSize);
    }
    else if (name == "B MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::B_MINOR_ROOT_png, BinaryData::B_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::B_MINOR_INV1_png, BinaryData::B_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::B_MINOR_INV2_png, BinaryData::B_MINOR_INV2_pngSize);
    }

    // ----- C# -----
    else if (name == "C# MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_MAJ_ROOT_png, BinaryData::C_SHARP_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_MAJ_INV1_png, BinaryData::C_SHARP_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_MAJ_INV2_png, BinaryData::C_SHARP_MAJ_INV2_pngSize);
    }
    else if (name == "C# MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_MINOR_ROOT_png, BinaryData::C_SHARP_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_MINOR_INV1_png, BinaryData::C_SHARP_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_MINOR_INV2_png, BinaryData::C_SHARP_MINOR_INV2_pngSize);
    }

    // ----- D# -----
    else if (name == "D# MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_MAJ_ROOT_png, BinaryData::D_SHARP_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_MAJ_INV1_png, BinaryData::D_SHARP_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_MAJ_INV2_png, BinaryData::D_SHARP_MAJ_INV2_pngSize);
    }
    else if (name == "D# MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_MINOR_ROOT_png, BinaryData::D_SHARP_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_MINOR_INV1_png, BinaryData::D_SHARP_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_MINOR_INV2_png, BinaryData::D_SHARP_MINOR_INV2_pngSize);
    }

    // ----- F# -----
    else if (name == "F# MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_MAJ_ROOT_png, BinaryData::F_SHARP_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_MAJ_INV1_png, BinaryData::F_SHARP_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_MAJ_INV2_png, BinaryData::F_SHARP_MAJ_INV2_pngSize);
    }
    else if (name == "F# MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_MINOR_ROOT_png, BinaryData::F_SHARP_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_MINOR_INV1_png, BinaryData::F_SHARP_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_MINOR_INV2_png, BinaryData::F_SHARP_MINOR_INV2_pngSize);
    }

    // ----- G# -----
    else if (name == "G# MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_MAJ_ROOT_png, BinaryData::G_SHARP_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_MAJ_INV1_png, BinaryData::G_SHARP_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_MAJ_INV2_png, BinaryData::G_SHARP_MAJ_INV2_pngSize);
    }
    else if (name == "G# MINOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_MIN_ROOT_png, BinaryData::G_SHARP_MIN_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_MINOR_INV1_png, BinaryData::G_SHARP_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_MINOR_INV2_png, BinaryData::G_SHARP_MINOR_INV2_pngSize);
    }

    // ----- A# -----
    else if (name == "A# MAJOR") {
        c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_MAJ_ROOT_png, BinaryData::A_SHARP_MAJ_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_MAJ_INV1_png, BinaryData::A_SHARP_MAJ_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_MAJ_INV2_png, BinaryData::A_SHARP_MAJ_INV2_pngSize);
    }
    else if (name == "A# MINOR") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_MINOR_ROOT_png, BinaryData::A_SHARP_MINOR_ROOT_pngSize);
        c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_MINOR_INV1_png, BinaryData::A_SHARP_MINOR_INV1_pngSize);
        c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_MINOR_INV2_png, BinaryData::A_SHARP_MINOR_INV2_pngSize);
    }


    // ----- C ----- DIM / AUG
    else if (name == "C DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_DIM_ROOT_png, BinaryData::C_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_DIM_INV1_png, BinaryData::C_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_DIM_INV2_png, BinaryData::C_DIM_INV2_pngSize);
    }
    else if (name == "C AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_AUG_ROOT_png, BinaryData::C_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_AUG_INV1_png, BinaryData::C_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_AUG_INV2_png, BinaryData::C_AUG_INV2_pngSize);
    }

    // ----- D ----- DIM / AUG
    else if (name == "D DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_DIM_ROOT_png, BinaryData::D_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_DIM_INV1_png, BinaryData::D_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_DIM_INV2_png, BinaryData::D_DIM_INV2_pngSize);
    }
    else if (name == "D AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_AUG_ROOT_png, BinaryData::D_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_AUG_INV1_png, BinaryData::D_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_AUG_INV2_png, BinaryData::D_AUG_INV2_pngSize);
    }

    // ----- E ----- DIM / AUG
    else if (name == "E DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::E_DIM_ROOT_png, BinaryData::E_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::E_DIM_INV1_png, BinaryData::E_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::E_DIM_INV2_png, BinaryData::E_DIM_INV2_pngSize);
    }
    else if (name == "E AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::E_AUG_ROOT_png, BinaryData::E_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::E_AUG_INV1_png, BinaryData::E_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::E_AUG_INV2_png, BinaryData::E_AUG_INV2_pngSize);
    }

    // ----- F ----- DIM / AUG
    else if (name == "F DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_DIM_ROOT_png, BinaryData::F_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_DIM_INV1_png, BinaryData::F_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_DIM_INV2_png, BinaryData::F_DIM_INV2_pngSize);
    }
    else if (name == "F AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_AUG_ROOT_png, BinaryData::F_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_AUG_INV1_png, BinaryData::F_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_AUG_INV2_png, BinaryData::F_AUG_INV2_pngSize);
    }

    // ----- G ----- DIM / AUG
    else if (name == "G DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_DIM_ROOT_png, BinaryData::G_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_DIM_INV1_png, BinaryData::G_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_DIM_INV2_png, BinaryData::G_DIM_INV2_pngSize);
    }
    else if (name == "G AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_AUG_ROOT_png, BinaryData::G_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_AUG_INV1_png, BinaryData::G_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_AUG_INV2_png, BinaryData::G_AUG_INV2_pngSize);
    }

    // ----- A ----- DIM / AUG
    else if (name == "A DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_DIM_ROOT_png, BinaryData::A_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_DIM_INV1_png, BinaryData::A_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_DIM_INV2_png, BinaryData::A_DIM_INV2_pngSize);
    }
    else if (name == "A AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_AUG_ROOT_png, BinaryData::A_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_AUG_INV1_png, BinaryData::A_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_AUG_INV2_png, BinaryData::A_AUG_INV2_pngSize);
    }

    // ----- B ----- DIM / AUG
    else if (name == "B DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::B_DIM_ROOT_png, BinaryData::B_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::B_DIM_INV1_png, BinaryData::B_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::B_DIM_INV2_png, BinaryData::B_DIM_INV2_pngSize);
    }
    else if (name == "B AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::B_AUG_ROOT_png, BinaryData::B_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::B_AUG_INV1_png, BinaryData::B_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::B_AUG_INV2_png, BinaryData::B_AUG_INV2_pngSize);
    }

    // ----- C# ----- DIM / AUG
    else if (name == "C# DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_DIM_ROOT_png, BinaryData::C_SHARP_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_DIM_INV1_png, BinaryData::C_SHARP_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_DIM_INV2_png, BinaryData::C_SHARP_DIM_INV2_pngSize);
    }
    else if (name == "C# AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_AUG_ROOT_png, BinaryData::C_SHARP_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_AUG_INV1_png, BinaryData::C_SHARP_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::C_SHARP_AUG_INV2_png, BinaryData::C_SHARP_AUG_INV2_pngSize);
    }

    // ----- D# ----- DIM / AUG
    else if (name == "D# DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_DIM_ROOT_png, BinaryData::D_SHARP_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_DIM_INV1_png, BinaryData::D_SHARP_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_DIM_INV2_png, BinaryData::D_SHARP_DIM_INV2_pngSize);
    }
    else if (name == "D# AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_AUG_ROOT_png, BinaryData::D_SHARP_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_AUG_INV1_png, BinaryData::D_SHARP_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::D_SHARP_AUG_INV2_png, BinaryData::D_SHARP_AUG_INV2_pngSize);
    }

    // ----- F# ----- DIM / AUG
    else if (name == "F# DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_DIM_ROOT_png, BinaryData::F_SHARP_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_DIM_INV1_png, BinaryData::F_SHARP_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_DIM_INV2_png, BinaryData::F_SHARP_DIM_INV2_pngSize);
    }
    else if (name == "F# AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_AUG_ROOT_png, BinaryData::F_SHARP_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_AUG_INV1_png, BinaryData::F_SHARP_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::F_SHARP_AUG_INV2_png, BinaryData::F_SHARP_AUG_INV2_pngSize);
    }

    // ----- G# ----- DIM / AUG
    else if (name == "G# DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_DIM_ROOT_png, BinaryData::G_SHARP_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_DIM_INV1_png, BinaryData::G_SHARP_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_DIM_INV2_png, BinaryData::G_SHARP_DIM_INV2_pngSize);
    }
    else if (name == "G# AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_AUG_ROOT_png, BinaryData::G_SHARP_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_AUG_INV1_png, BinaryData::G_SHARP_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::G_SHARP_AUG_INV2_png, BinaryData::G_SHARP_AUG_INV2_pngSize);
    }

    // ----- A# ----- DIM / AUG
    else if (name == "A# DIMINISHED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_DIM_ROOT_png, BinaryData::A_SHARP_DIM_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_DIM_INV1_png, BinaryData::A_SHARP_DIM_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_DIM_INV2_png, BinaryData::A_SHARP_DIM_INV2_pngSize);
    }
    else if (name == "A# AUGMENTED") {
    c.imgRoot = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_AUG_ROOT_png, BinaryData::A_SHARP_AUG_ROOT_pngSize);
    c.imgInv1 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_AUG_INV1_png, BinaryData::A_SHARP_AUG_INV1_pngSize);
    c.imgInv2 = juce::ImageCache::getFromMemory(BinaryData::A_SHARP_AUG_INV2_png, BinaryData::A_SHARP_AUG_INV2_pngSize);
    }


    c.imagesLoaded = true;
}