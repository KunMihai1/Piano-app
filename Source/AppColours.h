/*
  ==============================================================================

    AppColours.h
    Created: 6 Apr 2026
    Author:  Kisuke

    Shared colour palette used across all themed windows
    (SoundEffectWindow, MIDIWindow, etc.)

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace AppColours
{
    // --- Backgrounds ---
    static const juce::Colour background     { 0xff0f0f1a };   // very dark blue-black
    static const juce::Colour panelBg        { 0xff1a1a2e };   // dark indigo panel
    static const juce::Colour cardBg         { 0xff16213e };   // slightly lighter card

    // --- Text ---
    static const juce::Colour titleText      { 0xffe0e0ff };   // soft white-lavender
    static const juce::Colour subtitleText   { 0xff8888aa };   // muted label

    // --- Accents ---
    static const juce::Colour accent1        { 0xffff6f3c };   // warm orange
    static const juce::Colour accent2        { 0xff845ec2 };   // purple
    static const juce::Colour accent3        { 0xff00c9a7 };   // teal
    static const juce::Colour accent4        { 0xff4b7bec };   // blue
    static const juce::Colour accent5        { 0xfff9ca24 };   // golden yellow
    static const juce::Colour accent6        { 0xfffc5c65 };   // coral red
    static const juce::Colour accent7        { 0xff26de81 };   // emerald green
    static const juce::Colour accent8        { 0xffa55eea };   // violet

    // --- UI elements ---
    static const juce::Colour knobTrack      { 0xff2a2a3e };   // dark arc / slider track background
    static const juce::Colour separator      { 0xff2e2e44 };   // subtle divider
    static const juce::Colour rowLabel        { 0xff5e5e7e };   // row section label

    // --- Sustain LED ---
    static const juce::Colour sustainOn      { 0xff00e676 };   // green LED on
    static const juce::Colour sustainOff     { 0xff3a3a4a };   // dim LED off
}
