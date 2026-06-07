/*
  ==============================================================================

    SelectableLabel.cpp
    Created: 19 Jul 2025 2:08:23am
    Author:  Kisuke

  ==============================================================================
*/

#include "SelectableLabel.h"

void SelectableLabel::mouseDown(const juce::MouseEvent& event)
{
    if (onClick)
        onClick();

    juce::Label::mouseDown(event);

}
