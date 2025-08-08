# JUCE Build Command for TingeTape Plugin

## Objective
Execute every task in the checklist sequentially using strict test-driven development (TDD), mark tasks as complete, and commit changes with proper validation.

## Instructions for Claude

### Execution Principles
- **Strict TDD**: Always write failing tests before implementing functionality
- **Atomic Commits**: Commit after each completed task with descriptive messages
- **Continuous Validation**: Run full test suite after each implementation
- **No Skipping Tests**: Every implementation must have corresponding tests
- **Performance Monitoring**: Check performance after each major component

### Build Execution Process

#### Step 1: Initialize Build Environment
```bash
# Ensure clean build state
cmake --build build --clean-first
# Run initial test suite to verify baseline
cd build && ctest --output-on-failure
```

#### Step 2: Execute Checklist Tasks Sequentially

For each task in `docs/checklist.md`:

1. **Pre-Task Validation**
   - [ ] Verify all dependencies completed
   - [ ] Review task requirements and acceptance criteria
   - [ ] Estimate time and prepare development environment

2. **Test-First Implementation**
   - [ ] Write failing tests first (following TDD principles)
   - [ ] Verify tests fail as expected
   - [ ] Run test suite to ensure no regressions

3. **Minimal Implementation**
   - [ ] Implement minimum code to make tests pass
   - [ ] Focus on correctness over optimization initially
   - [ ] Use Context7 MCP for JUCE API verification

4. **Refactor & Optimize**
   - [ ] Improve code quality while keeping tests green
   - [ ] Optimize for performance if needed
   - [ ] Add error handling and edge case management

5. **Task Validation**
   - [ ] Run full test suite (unit + integration + performance)
   - [ ] Verify acceptance criteria met
   - [ ] Check performance targets achieved
   - [ ] Run smoke tests on plugin

6. **Documentation & Commit**
   - [ ] Update relevant documentation
   - [ ] Mark task as complete in checklist
   - [ ] Commit with descriptive message
   - [ ] Tag commit with task ID for tracking

#### Step 3: Continuous Integration Checks

After each task completion:
```bash
# Build plugin
cmake --build build --config Release --target TingeTape

# Run comprehensive test suite
./scripts/run-smoke-tests.sh

# Deploy for testing
./scripts/deploy-plugins.sh

# Run pluginval if available
pluginval --strictness-level 10 --validate [plugin-path]
```

#### Step 4: Phase Completion Validation

At the end of each phase:
- [ ] All phase tasks completed and marked
- [ ] Full regression test suite passes
- [ ] Performance benchmarks within targets
- [ ] Manual testing in DAW completed
- [ ] Phase completion commit with summary

### Task Execution Template

For each task, follow this template:

```markdown
## Task X.Y: [Task Name] - [Status: In Progress/Complete]

### Started: [Date/Time]
### Estimated: [X hours]
### Actual: [Y hours]

### Tests Written:
- [ ] Test 1: [Description] - [Pass/Fail]
- [ ] Test 2: [Description] - [Pass/Fail]
- [ ] Test 3: [Description] - [Pass/Fail]

### Implementation Notes:
- Context7 queries performed: [List API queries]
- Key design decisions: [Brief notes]
- Performance measurements: [CPU/Memory stats]

### Validation Results:
- [ ] Acceptance criteria 1: [Met/Not Met]
- [ ] Acceptance criteria 2: [Met/Not Met]
- [ ] Performance target: [Within/Exceeded/Below target]

### Commit Information:
- Commit hash: [SHA]
- Commit message: [Message]
- Files changed: [List key files]

### Next Task Dependencies:
- [List any tasks now unblocked]
```

### Error Handling Protocol

If any task fails:
1. **Document Failure**: Record specific failure mode and error messages
2. **Rollback if Needed**: Use git to rollback to last stable state
3. **Research Solution**: Use Context7 MCP to research JUCE best practices
4. **Revise Approach**: Modify implementation strategy if needed
5. **Update Checklist**: Adjust task if requirements were unrealistic

### Performance Monitoring

Track these metrics throughout build:
- **CPU Usage**: Per-component and total processing time
- **Memory Usage**: Peak and steady-state memory consumption
- **Test Execution Time**: Ensure test suite remains fast (<30 seconds)
- **Build Time**: Monitor for build performance regressions

### Quality Gates

Before marking any task complete:
- [ ] All task tests pass
- [ ] No regression in existing tests
- [ ] Performance within acceptable range
- [ ] Code follows project style guidelines
- [ ] Documentation updated if needed

### Final Build Validation

When all tasks complete:
1. **Comprehensive Test Suite**
   ```bash
   # Run all tests
   cd build && ctest --output-on-failure --parallel 4
   
   # Run smoke tests
   ./scripts/run-smoke-tests.sh
   
   # Run benchmarks
   ./benchmarks --reporter compact
   ```

2. **Plugin Validation**
   ```bash
   # Run pluginval
   pluginval --strictness-level 10 --validate [plugin-paths]
   
   # Deploy plugins
   ./scripts/deploy-plugins.sh
   ```

3. **Manual Testing Checklist**
   - [ ] Plugin loads in target DAWs without errors
   - [ ] All parameters respond correctly
   - [ ] Audio processing sounds as expected
   - [ ] No audio dropouts or glitches
   - [ ] Automation works properly
   - [ ] Preset save/load functions correctly

4. **Performance Validation**
   - [ ] CPU usage <1% on target hardware
   - [ ] No memory leaks during extended operation
   - [ ] Stable operation at all supported sample rates
   - [ ] Proper threading behavior

### Success Criteria

Build is complete when:
- [ ] All checklist tasks marked complete
- [ ] All automated tests pass
- [ ] Plugin passes pluginval validation
- [ ] Performance targets met
- [ ] Manual testing completed successfully
- [ ] Documentation updated
- [ ] Clean commit history with descriptive messages

### Completion Report

Generate final report including:
- Total development time vs. estimates
- Performance benchmarks achieved
- Test coverage statistics
- Known issues or limitations
- Recommendations for future development

## Execution Command

To begin build execution:
```bash
# Start build process
echo "🚀 Starting TingeTape Plugin Build Process"
echo "Following TDD principles with continuous validation"

# Initialize environment
./scripts/dev-setup.sh  # If needed

# Begin task execution from Phase 1, Task 1.1
# Each task will be executed with full TDD cycle
```

Ready to begin systematic plugin development with professional quality standards!