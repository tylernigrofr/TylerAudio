# TingeTape Professional Presets Guide

## Overview

This document defines professional presets for TingeTape that showcase its research-backed tape emulation capabilities across various mixing and mastering scenarios.

## Preset Categories

### 1. Mixing Presets

#### "Subtle Warmth" (Default)
**Use Case**: General mixing - adds gentle tape character without obvious processing
```
Wow Depth: 25%
Low-Cut Freq: 40Hz, Q: 0.7
High-Cut Freq: 15kHz, Q: 0.7
Drive: 25%
Tone: 0%
```
**Best For**: Vocals, acoustic instruments, mix bus
**Character**: Gentle harmonic enhancement with subtle pitch instability

#### "Clean Tape"
**Use Case**: Transparent tape color for critical sources
```  
Wow Depth: 10%
Low-Cut Freq: 30Hz, Q: 0.7
High-Cut Freq: 18kHz, Q: 0.7
Drive: 15%
Tone: +5%
```
**Best For**: Lead vocals, piano, strings
**Character**: Minimal processing with tape machine presence

#### "Vintage Character"
**Use Case**: Obvious tape character for creative enhancement
```
Wow Depth: 50%
Low-Cut Freq: 60Hz, Q: 0.9
High-Cut Freq: 12kHz, Q: 0.8
Drive: 45%
Tone: -15%
```
**Best For**: Electric guitars, drums, vintage-style mixes
**Character**: Noticeable wow, warm saturation, slightly dark

#### "Modern Warmth"
**Use Case**: Contemporary productions needing subtle enhancement
```
Wow Depth: 15%
Low-Cut Freq: 25Hz, Q: 0.7
High-Cut Freq: 16kHz, Q: 0.7
Drive: 30%
Tone: +10%
```
**Best For**: Electronic music, pop vocals, synths
**Character**: Clean with modern brightness and gentle saturation

### 2. Mastering Presets

#### "Mastering Glue"
**Use Case**: Stereo bus processing for cohesive mix glue
```
Wow Depth: 20%
Low-Cut Freq: 35Hz, Q: 0.7
High-Cut Freq: 17kHz, Q: 0.7
Drive: 20%
Tone: 0%
```
**Best For**: Mix bus, stereo processing
**Character**: Subtle binding effect with tape machine cohesion

#### "Analog Master"
**Use Case**: Full analog tape machine emulation for mastering
```
Wow Depth: 35%
Low-Cut Freq: 45Hz, Q: 0.8
High-Cut Freq: 14kHz, Q: 0.8  
Drive: 35%
Tone: -8%
```
**Best For**: Full mixes, album mastering chains
**Character**: Authentic vintage tape master character

#### "Digital Warmth"
**Use Case**: Adding analog character to digital masters
```
Wow Depth: 12%
Low-Cut Freq: 30Hz, Q: 0.7
High-Cut Freq: 19kHz, Q: 0.7
Drive: 18%
Tone: +3%
```
**Best For**: Digital sources, streaming masters
**Character**: Subtle analog enhancement without sacrificing clarity

### 3. Creative Presets

#### "Heavy Saturation"
**Use Case**: Aggressive tape saturation for creative effects
```
Wow Depth: 70%
Low-Cut Freq: 80Hz, Q: 1.2
High-Cut Freq: 10kHz, Q: 1.1
Drive: 75%
Tone: -25%
```
**Best For**: Drums, distorted guitars, lo-fi aesthetics
**Character**: Heavy processing with significant tape artifacts

#### "Lo-Fi Tape"
**Use Case**: Vintage lo-fi character for creative applications
```
Wow Depth: 85%
Low-Cut Freq: 100Hz, Q: 1.0
High-Cut Freq: 8kHz, Q: 1.0
Drive: 60%
Tone: -35%
```
**Best For**: Lo-fi hip-hop, vintage recreations, creative processing
**Character**: Heavy wow, limited bandwidth, warm saturation

#### "Tape Stop Effect"
**Use Case**: Extreme wow for creative pitch modulation
```
Wow Depth: 95%
Low-Cut Freq: 50Hz, Q: 0.8
High-Cut Freq: 12kHz, Q: 0.9
Drive: 40%
Tone: -10%
```
**Best For**: Transition effects, creative modulation, sound design
**Character**: Extreme pitch modulation simulating tape speed variation

### 4. Genre-Specific Presets

#### "Jazz Vocals"
**Use Case**: Classic jazz vocal recording character
```
Wow Depth: 30%
Low-Cut Freq: 50Hz, Q: 0.8
High-Cut Freq: 13kHz, Q: 0.8
Drive: 35%
Tone: -12%
```
**Best For**: Jazz vocals, acoustic jazz instruments
**Character**: Vintage recording studio warmth

#### "Rock Drums"
**Use Case**: Punchy rock drum sound with tape compression
```
Wow Depth: 40%
Low-Cut Freq: 70Hz, Q: 1.0
High-Cut Freq: 11kHz, Q: 0.9
Drive: 55%
Tone: -5%
```
**Best For**: Rock/metal drums, aggressive percussion
**Character**: Punchy saturation with controlled high-end

#### "Ambient Textures"
**Use Case**: Atmospheric processing for ambient music
```
Wow Depth: 60%
Low-Cut Freq: 40Hz, Q: 0.6
High-Cut Freq: 14kHz, Q: 0.6
Drive: 25%
Tone: -20%
```
**Best For**: Pads, ambient textures, atmospheric sounds
**Character**: Dreamy wow with dark, warm character

## Preset Implementation Guidelines

### Parameter Validation

All presets must pass these validation criteria:

1. **Performance**: CPU usage remains <1% on target hardware
2. **Stability**: No audio artifacts under automation
3. **Quality**: THD+N within research-specified limits
4. **Musical Usefulness**: Enhances rather than detracts from source material

### Preset Testing Protocol

For each preset:

1. **Signal Compatibility**: Test with various input types
2. **Level Management**: Verify appropriate output levels
3. **Automation Behavior**: Confirm smooth parameter transitions  
4. **Mix Context**: Validate in full mix scenarios
5. **Genre Appropriateness**: Confirm suitability for intended musical styles

### Documentation Standards

Each preset includes:
- **Use Case Description**: When and how to use
- **Parameter Breakdown**: Exact values and reasoning
- **Best Applications**: Specific instruments/sources
- **Character Description**: Sonic qualities and effects
- **Mix Integration**: How it sits in a full production

## Advanced Preset Techniques

### Parameter Interaction Optimization

Understanding how parameters interact for optimal preset design:

1. **Wow + Drive**: Higher wow often pairs with moderate drive for vintage character
2. **Tone + Filters**: Tone control can complement filter settings for balanced frequency response
3. **Drive + High-Cut**: Higher drive benefits from gentler high-cut for smoothness
4. **Low-Cut + Wow**: Low-cut prevents wow-induced subsonic buildup

### Dynamic Preset Usage

Presets designed for different mix positions:

- **Insert Processing**: Direct on individual tracks
- **Send Processing**: For parallel processing blends
- **Bus Processing**: On grouped instruments or mix bus
- **Master Processing**: Final stereo chain enhancement

### Automation-Friendly Parameters

Preset parameters suitable for real-time automation:
- **Wow Depth**: Musical pitch variation effects
- **Drive**: Dynamic saturation intensity
- **Tone**: Real-time brightness adjustment
- **Bypass**: A/B comparison and effect drops

## Quality Assurance

### Preset Validation Checklist

- [ ] All parameters within research-specified ranges
- [ ] CPU usage <1% in typical scenarios
- [ ] No audio artifacts or instabilities
- [ ] Musical enhancement rather than degradation
- [ ] Appropriate for stated use cases
- [ ] Professional mixing/mastering standards met
- [ ] Cross-platform consistency verified
- [ ] Documentation complete and accurate

### Professional Standards

All presets meet professional audio industry standards:
- Compatible with major DAW automation systems
- Appropriate output levels for professional workflows
- Stable under extended use and parameter automation
- Consistent behavior across different sample rates
- Professional naming and organization

## User Guidelines

### Preset Selection

Choose presets based on:
1. **Source Material**: Match preset character to input type
2. **Mix Context**: Consider how it fits with other processing
3. **Musical Genre**: Select genre-appropriate character
4. **Processing Goals**: Enhancement vs. creative effect

### Customization

Starting from presets:
1. **Fine-tuning**: Adjust parameters for specific sources
2. **Automation**: Add movement for musical expression  
3. **Parallel Processing**: Blend with dry signal for subtlety
4. **Chain Position**: Optimize placement in processing chain

### Best Practices

- Start with subtle presets and increase intensity as needed
- Use A/B comparison to validate improvements
- Consider mono compatibility for critical sources
- Save custom variations for consistent workflow
- Document successful combinations for future use

This preset system provides users with immediate access to TingeTape's capabilities while serving as educational examples of optimal parameter combinations for various applications.