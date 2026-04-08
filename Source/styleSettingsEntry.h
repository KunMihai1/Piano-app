/*
  ==============================================================================

    styleSettingsEntry.h
    Created: 8 Apr 2026 1:08:42am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

struct SoundSettings
{
	int reverb = 64;
	int volume = 64;

	int brightness = 64;
	int chorus = 64;
	int expression = 64;
	int resonance = 64;
	bool sustainToggle = 0;

	int attack = 64;
	int decay = 64;
	int release = 64;
	int vibrato = 0;

	int delay = 64;
	int pan = 64;

	int distortion = 0;
	int filterTrack = 0;
	int tremolo = 0;
	int randomMod = 0;

    void setValue(const juce::String& key, int value)
    {
        if (key == "brightness") brightness = value;
        else if (key == "expression") expression = value;
        else if (key == "chorus") chorus = value;
        else if (key == "resonance") resonance = value;
        else if (key == "sustainToggle") sustainToggle = value;

        else if (key == "vibrato") vibrato = value;
        else if (key == "attack") attack = value;
        else if (key == "decay") decay = value;
        else if (key == "release") release = value;

        else if (key == "volume") volume = value;
        else if (key == "reverb") reverb = value;
        else if (key == "delay") delay = value;
        else if (key == "pan") pan = value;

        else if (key == "distortion") distortion = value;
        else if (key == "filterTrack") filterTrack = value;
        else if (key == "tremolo") tremolo = value;
        else if (key == "randomMod") randomMod = value;
    }

};

struct StyleSettings {
	SoundSettings firstHand;
	SoundSettings secondHand;
};