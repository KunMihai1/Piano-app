## Built With

- [JUCE](https://juce.com) — an open-source C++ framework for developing cross-platform audio applications.  
- Licensed under the [JUCE License (GPLv3 / Commercial)](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md)

This project uses JUCE under the terms of the [GNU General Public License v3 (GPLv3)](https://www.gnu.org/licenses/gpl-3.0.html).


## Building

1. Install [JUCE](https://juce.com/get-juce).
2. Open the project in Projucer (`MyPlugin.jucer`).
3. Configure your exporter (Xcode / Visual Studio / CLion).
4. Build and run.


# Piano App — Synth & Arranger

This is a synth/arranger app I built using JUCE. The idea was to make something that’s fun to play with but also useful for experimenting with tracks and arrangements. It’s still a work in progress, but it already lets you play, record, and mix MIDI tracks in real time.

## Features

- Play notes in real time — the piano keys respond instantly.
- Record your performance — capture what you play without noticeable lag.
- Load MIDI files — pick which tracks you want to play along with.
- Visual keyboard — see which keys are active as you play.
- Built with future arranger features in mind (more coming soon).

## Tech

- **Framework:** JUCE
- **Language:** C++
- **Audio:** Low-latency, real-time processing
- **UI:** JUCE components for cross-platform desktop apps

## What I Learned

Working on this project taught me a lot about low-latency audio programming and the challenges of real-time performance. Some highlights:

- Handling audio and GUI threads without lag
- Recording and playing MIDI in sync
- Structuring a JUCE project for expandability
- Optimizing C++ code for real-time responsiveness

It was a great experience for getting hands-on with how music software actually works under the hood.

## Future Plans

- Expand arranger features: patterns, automation, loops, and divide tracks into sections (intro, main, ending)
- Enhance visual feedback: velocity, timing, key animations
- Integrate and improve MIDI controller support
- Apply BPM changes dynamically, including during playback

## Skills I Gained

- Real-time audio programming
- MIDI and event handling
- C++ and JUCE framework
- UI/UX for music applications
- Optimizing for performance
