#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <cmath>

namespace TylerAudio
{
    namespace Constants
    {
        constexpr int defaultWidth = 400;
        constexpr int defaultHeight = 300;
        constexpr float defaultGain = 0.5f;
        constexpr float kMinDbValue = -100.0f;
        constexpr float kMaxDbValue = 12.0f;
        constexpr float kDenormalThreshold = 1e-15f;
    }
    
    namespace Utils
    {
        // DSP utility functions with proper bounds checking
        [[nodiscard]] constexpr float dbToGain(float db) noexcept
        {
            return std::pow(10.0f, juce::jlimit(Constants::kMinDbValue, Constants::kMaxDbValue, db) * 0.05f);
        }
        
        [[nodiscard]] constexpr float gainToDb(float gain) noexcept
        {
            return 20.0f * std::log10(std::max(1e-6f, std::abs(gain)));
        }
        
        // Denormal handling
        [[nodiscard]] constexpr float sanitizeFloat(float value) noexcept
        {
            return (std::abs(value) < Constants::kDenormalThreshold) ? 0.0f : value;
        }
        
        // Thread-safe parameter smoothing
        class SmoothingFilter
        {
        public:
            void setTargetValue(float newValue) noexcept
            {
                targetValue.store(newValue, std::memory_order_relaxed);
            }
            
            [[nodiscard]] float getNextValue() noexcept
            {
                const float target = targetValue.load(std::memory_order_relaxed);
                currentValue += (target - currentValue) * smoothingCoeff;
                return sanitizeFloat(currentValue);
            }
            
            void setSmoothingTime(double smoothingTimeSeconds, double sampleRate) noexcept
            {
                smoothingCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (smoothingTimeSeconds * sampleRate)));
            }
            
            void snapToTarget() noexcept
            {
                currentValue = targetValue.load(std::memory_order_relaxed);
            }
            
        private:
            std::atomic<float> targetValue{0.0f};
            float currentValue{0.0f};
            float smoothingCoeff{0.01f};
        };
        
        // Realtime-safe parameter access helper
        template<typename T>
        class AtomicParameter
        {
        public:
            AtomicParameter(T initialValue = T{}) : value(initialValue) {}
            
            void store(T newValue) noexcept
            {
                value.store(newValue, std::memory_order_relaxed);
            }
            
            [[nodiscard]] T load() const noexcept
            {
                return value.load(std::memory_order_relaxed);
            }
            
            // Convenience operators
            AtomicParameter& operator=(T newValue) noexcept
            {
                store(newValue);
                return *this;
            }
            
            operator T() const noexcept
            {
                return load();
            }
            
        private:
            std::atomic<T> value;
        };
        
        using AtomicFloat = AtomicParameter<float>;
        using AtomicBool = AtomicParameter<bool>;
    }
    
    // Parameter IDs for consistency across plugins
    namespace ParameterIDs
    {
        constexpr char kGain[] = "gain";
        constexpr char kBypass[] = "bypass";
        constexpr char kInputGain[] = "inputGain";
        constexpr char kOutputGain[] = "outputGain";
    }
}