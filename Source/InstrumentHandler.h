/*
  ==============================================================================

    InstrumentHandler.h
    Created: 26 Mar 2025 9:45:50pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

enum class InstrumentType { Piano, Bass, Guitar, Violin };

class InstrumentHandler {
public:
    InstrumentHandler();
    ~InstrumentHandler();

    void setInstrument(InstrumentType newInstrument);

private:
    void loadInstrumentSettings();

    InstrumentType currentInstrument = InstrumentType::Piano;

};