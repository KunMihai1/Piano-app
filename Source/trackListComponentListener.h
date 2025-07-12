/*
  ==============================================================================

    trackListComponentListener.h
    Created: 13 Jul 2025 12:28:41am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

class TrackListListener
{
public:
    virtual void updateUIbeforeAnyLoadingCase() = 0;
    virtual ~TrackListListener() = default;
};