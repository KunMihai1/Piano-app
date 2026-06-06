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
    initializeMAP();
}

InstrumentHandler::~InstrumentHandler()
{
}

const std::unordered_map<int, std::vector<std::pair<int, int>>>& InstrumentHandler::getInstrumentPresets()
{
    return instrumentPresetsMap;

}

const std::vector<std::pair<int, int>>& InstrumentHandler::getPreset(int programNumber)
{
    return instrumentPresetsMap[programNumber];
}

void InstrumentHandler::initializeMAP()
{
    instrumentPresetsMap.clear();

    instrumentPresetsMap[0] = { {91, -1}, {93, 10}, {74, 100}, {71, 40}, {11, 127} }; //grand piano
    instrumentPresetsMap[4] = { {91, -1}, {93, 70}, {74, 90}, {71, 30}, {11, 127} }; //electric piano
    instrumentPresetsMap[32] = { {91, -1}, {93, 0}, {74, 60}, {71, 20}, {11, 100} };
    instrumentPresetsMap[33] = { {91, -1}, {93, 10}, {74, 70}, {71, 35}, {11, 110} };
    instrumentPresetsMap[34] = { {91, -1}, {93, 15}, {74, 80}, {71, 45}, {11, 115} };
    instrumentPresetsMap[35] = { {91, -1}, {93, 10}, {74, 75}, {71, 50}, {11, 100} };
    instrumentPresetsMap[36] = { {91, -1}, {93, 25}, {74, 95}, {71, 60}, {11, 127} };
    instrumentPresetsMap[37] = { {91, -1}, {93, 30}, {74, 100}, {71, 55}, {11, 127} };
    instrumentPresetsMap[38] = { {91, -1}, {93, 40}, {74, 127}, {71, 80}, {11, 127} };
    instrumentPresetsMap[39] = { {91, -1}, {93, 50}, {74, 127}, {71, 85}, {11, 127} };
    instrumentPresetsMap[24] = { {91, -1}, {93, 10}, {74, 70}, {71, 40}, {11, 100} };
    instrumentPresetsMap[25] = { {91, -1}, {93, 15}, {74, 80}, {71, 50}, {11, 110} };
    instrumentPresetsMap[26] = { {91, -1}, {93, 20}, {74, 90}, {71, 55}, {11, 110} };
    instrumentPresetsMap[27] = { {91, -1}, {93, 25}, {74, 100}, {71, 60}, {11, 115} };
    instrumentPresetsMap[41] = { {91, -1}, {93, 18}, {74, 95}, {71, 55}, {11, 122} }; //violin
    instrumentPresetsMap[40] ={ {91, -1},{93, 18},{74, 94},{71, 55},{11, 122}};  //viola
    instrumentPresetsMap[42] = { {91, -1}, {93, 15}, {74, 85}, {71, 50}, {11, 120} };
    instrumentPresetsMap[43] = { {91, -1}, {93, 10}, {74, 80}, {71, 45}, {11, 115} };
    instrumentPresetsMap[56] = { {91, -1}, {93, 40}, {74, 100}, {71, 30}, {11, 127} };
    instrumentPresetsMap[57] = { {91, -1}, {93, 30}, {74, 95}, {71, 40}, {11, 120} };
    instrumentPresetsMap[59] = { {91, -1}, {93, 20}, {74, 100}, {71, 60}, {11, 127} };
    instrumentPresetsMap[68] = { {91, -1}, {93, 10}, {74, 85}, {71, 35}, {11, 115} };
    instrumentPresetsMap[70] = { {91, -1}, {93, 5}, {74, 80}, {71, 30}, {11, 110} };
    instrumentPresetsMap[71] = { {91, -1}, {93, 15}, {74, 95}, {71, 40}, {11, 120} };
    instrumentPresetsMap[73] = { {91, -1}, {93, 20}, {74, 110}, {71, 45}, {11, 127} };
    instrumentPresetsMap[21] = { {91, -1}, {93, 20}, {74, 75}, {71, 60}, {11, 100} };
    instrumentPresetsMap[22] = { {91, -1}, {93, 15}, {74, 70}, {71, 55}, {11, 105} };
    instrumentPresetsMap[23] = { {91, -1}, {93, 10}, {74, 68}, {71, 52}, {11, 98} };
    instrumentPresetsMap[16] = { {91, -1}, {93, 30}, {74, 80}, {71, 65}, {11, 110} };
    instrumentPresetsMap[17] = { {91, -1}, {93, 25}, {74, 75}, {71, 60}, {11, 108} };
    instrumentPresetsMap[18] = { {91, -1}, {93, 40}, {74, 85}, {71, 55}, {11, 115} };
    instrumentPresetsMap[19] = { {91, -1}, {93, 60}, {74, 90}, {71, 70}, {11, 120} };
    instrumentPresetsMap[20] = { {91, -1}, {93, 35}, {74, 70}, {71, 60}, {11, 105} };   
}
