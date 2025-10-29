/*
  ==============================================================================

    InstrumentHandler.h
    Created: 26 Mar 2025 9:45:50pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * @class InstrumentHandler
 * @brief Manages instrument presets for MIDI programs.
 *
 * This class stores predefined presets for various MIDI instrument programs.
 * Each preset consists of a set of control changes represented as pairs of
 * controller number and value.
 */
class InstrumentHandler {
public:
    /** Constructor */
    InstrumentHandler();

    /** Destructor */
    ~InstrumentHandler();

    /**
     * @brief Returns the full map of instrument presets.
     * @return Map of program number to a vector of control change pairs
     */
    const std::unordered_map<int, std::vector<std::pair<int, int>>>& getInstrumentPresets();

    /**
     * @brief Returns the preset for a specific program number.
     * @param programNumber MIDI program number
     * @return Vector of control change pairs for the given program
     */
    const std::vector<std::pair<int, int>>& getPreset(int programNumber);

private:
    /** @brief Initializes the internal preset map with predefined values */
    void initializeMAP();

    std::unordered_map<int, std::vector<std::pair<int, int>>> instrumentPresetsMap; ///< Map of program number to preset control changes
};
