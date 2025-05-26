/*
  ==============================================================================

    InstrumentHandler.h
    Created: 26 Mar 2025 9:45:50pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class InstrumentHandler {
public:
    InstrumentHandler();
    ~InstrumentHandler();
    const std::unordered_map<int, std::vector<std::pair<int, int>>>& getInstrumentPresets();
    const std::vector<std::pair<int, int>>& getPreset(int programNumber);

private:
    void initializeMAP();

    std::unordered_map<int, std::vector<std::pair<int, int>>> instrumentPresetsMap;

};