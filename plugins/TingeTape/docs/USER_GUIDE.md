# TingeTape User Guide

## Overview

TingeTape is a research-backed tape emulation plugin that brings authentic analog tape character to your digital productions. Based on comprehensive analysis of vintage tape machines, TingeTape delivers professional-quality tape warmth, saturation, and wow effects.

## Quick Start

### Installation
1. **Windows**: Copy `TingeTape.vst3` to `C:\Program Files\Common Files\VST3\`
2. **macOS**: Copy `TingeTape.vst3` to `~/Library/Audio/Plug-Ins/VST3/` and `TingeTape.component` to `~/Library/Audio/Plug-Ins/Components/`

### First Use
1. Load TingeTape on any audio track
2. Start with the "Subtle Warmth" preset (default)
3. Adjust the **Wow** control to taste (25% is a great starting point)
4. Fine-tune **Dirt** for desired saturation (25% for gentle warmth)

## Parameters

### Wow (0-100%)
**What it does**: Simulates the pitch instability characteristic of analog tape machines
- **Low values (10-30%)**: Subtle tape character, perfect for mixing
- **High values (50-100%)**: Obvious wow effect for creative applications
- **Musical tip**: Use automation for dynamic tape effects

### Low Cut (20-200 Hz, Q: 0.1-2.0)
**What it does**: High-pass filter to remove unwanted low frequencies
- **40-60 Hz**: Gentle rumble removal without affecting bass
- **80-120 Hz**: More aggressive filtering for cleaner low end
- **Q setting**: Higher Q values create resonant peaks near the cutoff

### High Cut (5-20 kHz, Q: 0.1-2.0)  
**What it does**: Low-pass filter for vintage tape high-frequency rolloff
- **15-18 kHz**: Subtle warmth while maintaining clarity
- **8-12 kHz**: Strong vintage character, darker sound
- **Q setting**: Higher Q values emphasize frequencies just below cutoff

### Dirt (0-100%)
**What it does**: Tape saturation that adds harmonic richness
- **10-30%**: Clean tape machine warmth
- **40-60%**: Noticeable saturation and compression
- **70-100%**: Heavy tape distortion for creative effects
- **Technical note**: Uses research-accurate tanh algorithm with proper gain compensation

### Tone (-100% to +100%)
**What it does**: Tilt EQ that simultaneously adjusts bass and treble
- **Negative values**: Warmer, darker sound (boosts lows, cuts highs)
- **Positive values**: Brighter sound (cuts lows, boosts highs)
- **0%**: Neutral, no tonal coloration

### Bypass
**What it does**: Completely bypasses all processing for A/B comparison
- Use to compare processed vs. unprocessed signal
- Essential for making informed mixing decisions

## Professional Preset Collection

### Mixing Presets

#### "Subtle Warmth" (Default)
- **Best for**: General mixing, vocals, acoustic instruments
- **Character**: Gentle tape enhancement without obvious processing
- **Settings**: 25% Wow, 40Hz Low Cut, 15kHz High Cut, 25% Dirt, 0% Tone

#### "Clean Tape" 
- **Best for**: Critical sources requiring transparency
- **Character**: Minimal processing with tape machine presence
- **Settings**: 10% Wow, 30Hz Low Cut, 18kHz High Cut, 15% Dirt, +5% Tone

#### "Vintage Character"
- **Best for**: Electric guitars, drums, vintage-style mixes  
- **Character**: Noticeable wow, warm saturation, slightly dark
- **Settings**: 50% Wow, 60Hz Low Cut, 12kHz High Cut, 45% Dirt, -15% Tone

### Mastering Presets

#### "Mastering Glue"
- **Best for**: Stereo bus processing for mix cohesion
- **Character**: Subtle binding effect with tape machine glue
- **Settings**: 20% Wow, 35Hz Low Cut, 17kHz High Cut, 20% Dirt, 0% Tone

#### "Analog Master"
- **Best for**: Full mixes, album mastering chains
- **Character**: Authentic vintage tape master character
- **Settings**: 35% Wow, 45Hz Low Cut, 14kHz High Cut, 35% Dirt, -8% Tone

### Creative Presets

#### "Heavy Saturation"
- **Best for**: Drums, distorted guitars, lo-fi aesthetics
- **Character**: Aggressive processing with significant tape artifacts
- **Settings**: 70% Wow, 80Hz Low Cut, 10kHz High Cut, 75% Dirt, -25% Tone

#### "Lo-Fi Tape"
- **Best for**: Lo-fi hip-hop, vintage recreations
- **Character**: Heavy wow, limited bandwidth, warm saturation
- **Settings**: 85% Wow, 100Hz Low Cut, 8kHz High Cut, 60% Dirt, -35% Tone

## Usage Tips

### For Mixing Engineers
1. **Start Subtle**: Begin with 15-25% settings and increase as needed
2. **A/B Compare**: Use bypass frequently to ensure you're enhancing, not degrading
3. **Context Matters**: What sounds good solo may not work in the mix
4. **Automation**: Consider automating wow for chorus/verse dynamics

### For Mastering Engineers  
1. **Less is More**: Use gentle settings (10-30%) for stereo bus processing
2. **Monitor Levels**: Tape saturation can increase apparent loudness
3. **Frequency Balance**: Use tone control to maintain tonal balance
4. **Reference Tracks**: Compare against commercial releases

### For Creative Applications
1. **Extreme Settings**: Don't be afraid to push parameters to 70-100%
2. **Parallel Processing**: Blend heavily processed signal with dry signal
3. **Send Effects**: Use TingeTape on a send for creative blending
4. **Automation**: Create tape stop effects with wow automation

## Technical Specifications

### Performance
- **CPU Usage**: <1% on modern systems
- **Memory Usage**: <50KB per instance  
- **Latency**: <5ms (from delay line only)
- **Sample Rate**: 44.1kHz - 192kHz supported

### Audio Quality
- **THD+N**: <0.1% moderate settings, <1% extreme settings
- **Signal-to-Noise**: >100dB
- **Dynamic Range**: >120dB preservation
- **Frequency Response**: ±0.1dB from specifications

### Compatibility
- **Formats**: VST3, Audio Unit (macOS)
- **Platforms**: Windows 10/11, macOS 10.15+
- **DAWs**: Pro Tools, Logic Pro, Ableton Live, Cubase, Reaper, FL Studio
- **Bit Depth**: 32-bit float internal processing

## Troubleshooting

### Plugin Not Loading
- **Windows**: Ensure plugin is in `C:\Program Files\Common Files\VST3\`
- **macOS**: Check both VST3 and AU directories, restart DAW
- **Permissions**: Ensure read/execute permissions on plugin files

### High CPU Usage
- **Check Sample Rate**: Higher sample rates increase CPU usage
- **Buffer Size**: Larger buffer sizes can improve efficiency  
- **Multiple Instances**: Each instance adds to CPU load
- **Bypass When Not Needed**: Use bypass during playback if not needed

### Audio Artifacts
- **Clicks/Pops**: Usually caused by buffer underruns - increase buffer size
- **Distortion**: Reduce dirt parameter or check input levels
- **Weird Modulation**: Check wow parameter isn't too high for the source

### Parameter Automation Issues
- **Smooth Automation**: Use curved automation for wow and dirt parameters
- **Jump Prevention**: Parameters are smoothed to prevent clicks
- **DAW-Specific**: Some DAWs handle automation differently

## Support

### Documentation
- **Implementation Guide**: Technical details for advanced users
- **Preset Guide**: Detailed breakdown of all presets
- **Beta Testing**: Feedback and bug reporting process

### Contact
- **Email**: support@tyleraudio.com
- **Website**: www.tyleraudio.com
- **User Manual**: Latest version always available online

### Known Issues
- None currently - this is a stable release

## Version History

### v1.0.0 (Current)
- Initial release
- Research-backed tape emulation algorithms
- Professional preset collection
- Cross-platform compatibility
- Comprehensive testing and validation

---

*TingeTape represents the culmination of extensive research into vintage tape machine behavior, bringing authentic analog character to your digital productions with the reliability and precision of modern plugin development.*