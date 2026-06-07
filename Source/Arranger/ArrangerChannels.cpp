#include "ArrangerChannels.h"

namespace ArrangerChannels
{
    int melodicChannelForIndex (int melodicIndex)
    {
        static const int channels[] = { 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13 };
        constexpr int count = (int) (sizeof (channels) / sizeof (channels[0]));
        if (melodicIndex < 0 || melodicIndex >= count)
            return -1;
        return channels[melodicIndex];
    }
}
