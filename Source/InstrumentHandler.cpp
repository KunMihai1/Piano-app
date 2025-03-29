/*
  ==============================================================================

    InstrumentHandler.cpp
    Created: 26 Mar 2025 9:45:50pm
    Author:  Kisuke

  ==============================================================================
*/

#include "InstrumentHandler.h"

InstrumentHandler::InstrumentHandler()
{
}

InstrumentHandler::~InstrumentHandler()
{
}

void InstrumentHandler::setInstrument(InstrumentType newInstrument)
{
    this->currentInstrument = newInstrument;
    loadInstrumentSettings();
}

void InstrumentHandler::loadInstrumentSettings()
{
}
