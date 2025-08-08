# JUCE Research Command for TingeTape Plugin

## Objective
Research and compile comprehensive information for tape emulation plugin development using JUCE framework.

## Instructions for Claude

### Step 1: Classify Plugin Concept
Analyze the TingeTape plugin concept and classify it within the audio plugin ecosystem:
- Plugin category (e.g., tape emulation, analog modeling, creative effect)
- Target use cases (mixing, mastering, creative processing)
- Technical complexity level
- Market positioning relative to existing plugins

### Step 2: Context7 MCP Research (MANDATORY)
**CRITICAL**: Query Context7 MCP server for up-to-date JUCE documentation on:
- `juce::dsp` module for DSP processing
- `juce::AudioProcessor` best practices
- `juce::AudioProcessorValueTreeState` for parameter management  
- `juce::dsp::DelayLine` for modulation effects
- `juce::dsp::IIR::Filter` for resonant filtering
- `juce::dsp::Oscillator` for LFO implementation
- Performance optimization techniques for realtime audio

### Step 3: Compile Research Sources
Search and compile from multiple sources:

#### A. Official Documentation
- JUCE official documentation and tutorials
- JUCE GitHub repository examples
- JUCE DSP module documentation

#### B. Academic Sources
- Recent papers on tape emulation algorithms
- Digital signal processing research (last 5 years)
- Psychoacoustic research on tape characteristics
- Performance optimization studies

#### C. Industry Best Practices
- Open source tape emulation plugins for reference
- Professional plugin development patterns
- JUCE community discussions and solutions

### Step 4: Apply Relevance Filtering
Filter and prioritize sources based on:
- Relevance to tape emulation DSP
- JUCE framework compatibility
- Realtime performance requirements
- Code quality and maintainability
- Recent publication/update date

### Step 5: Create Research Output
Generate the following files in `docs/research/`:

#### `tape_emulation_algorithms.md`
- Wow and flutter modulation techniques
- Tape saturation modeling approaches
- Frequency response characteristics of tape
- Noise modeling (if applicable to our scope)

#### `juce_dsp_patterns.md`
- Best practices for JUCE DSP implementation
- Realtime safety patterns
- Parameter smoothing techniques
- Performance optimization strategies

#### `reference_implementations.md`
- Analysis of existing open-source tape emulation plugins
- Code patterns and architectural decisions
- Performance benchmarks and comparisons

#### `technical_specifications.md`
- Detailed technical requirements for TingeTape
- DSP algorithm specifications
- Performance targets and constraints
- API design considerations

## Success Criteria
Research is complete when:
- [ ] Context7 MCP queries completed for all JUCE modules
- [ ] Minimum 5 academic sources found and analyzed
- [ ] 3+ reference implementations analyzed
- [ ] Technical specifications clearly defined
- [ ] All research files created with comprehensive information
- [ ] Sources properly cited with links and publication dates

## Next Step
After completing research, proceed to `2_juce_specification.md` to distill findings into actionable specification.