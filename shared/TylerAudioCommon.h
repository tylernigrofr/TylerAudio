#pragma once

#include <JuceHeader.h>

namespace TylerAudio
{
    namespace Constants
    {
        constexpr int defaultWidth = 400;
        constexpr int defaultHeight = 300;
        constexpr float defaultGain = 0.5f;
    }
    
    namespace Utils
    {
        inline float dbToGain(float db)
        {
            return std::pow(10.0f, db * 0.05f);
        }
        
        inline float gainToDb(float gain)
        {
            return 20.0f * std::log10(std::max(0.000001f, gain));
        }
    }
}