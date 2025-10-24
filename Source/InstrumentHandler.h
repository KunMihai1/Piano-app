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
 * @brief Handles instrument presets for MIDI program numbers.
 *
 * @details
 * Provides a mapping of General MIDI program numbers to a set of control change
 * values (as pairs of <controllerNumber, value>) used for instrument presets.
 * Presets can be queried either as the full map or individually by program number.
 */
class InstrumentHandler {
public:

    /** @brief Constructs the InstrumentHandler and initializes the presets map. */
    InstrumentHandler();

    /** @brief Destructor. Currently does no special cleanup. */

    ~InstrumentHandler();

    /**
     * @brief Returns a reference to the full instrument presets map.
     * @return const reference to std::unordered_map mapping program numbers to preset vectors
     */
    const std::unordered_map<int, std::vector<std::pair<int, int>>>& getInstrumentPresets();

    /**
     * @brief Returns the preset for a given MIDI program number.
     * @param programNumber MIDI program number (0-127)
     * @return const reference to a vector of <controller, value> pairs
     *
     * @note Accessing a program number that does not exist in the map will insert a new empty preset.
     */
    const std::vector<std::pair<int, int>>& getPreset(int programNumber);

private:

    /** @brief Initializes the internal instrumentPresetsMap with predefined values. */
    void initializeMAP();

    /** 
     * @brief Internal mapping of MIDI program numbers to controller/value pairs.
     *
     * The key is the MIDI program number (0-127), and the value is a vector of
     * pairs, where each pair represents a MIDI controller number and its value.
     */
    std::unordered_map<int, std::vector<std::pair<int, int>>> instrumentPresetsMap;

};
