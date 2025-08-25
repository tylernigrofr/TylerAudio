#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>
#include <functional>

namespace TylerAudio {
namespace UITestUtilities {

//==============================================================================
/** UI Component testing utilities */
class ComponentTester {
public:
    /** Test component visibility and bounds */
    static bool testComponentVisibility(juce::Component& component);
    
    /** Test component resizing behavior */
    static bool testComponentResizing(juce::Component& component,
                                     const std::vector<juce::Rectangle<int>>& testBounds);
    
    /** Test component painting (no crashes during paint) */
    static bool testComponentPainting(juce::Component& component,
                                     int width = 400, 
                                     int height = 300);
    
    /** Test component accessibility */
    static bool testComponentAccessibility(juce::Component& component);
    
    /** Test component keyboard navigation */
    static bool testKeyboardNavigation(juce::Component& component);
    
    /** Test component mouse interaction */
    static bool testMouseInteraction(juce::Component& component);
    
    /** Validate component hierarchy */
    static bool validateComponentHierarchy(juce::Component& rootComponent);
    
    /** Test component with different look and feel */
    static bool testWithDifferentLookAndFeel(juce::Component& component);
};

//==============================================================================
/** Parameter UI testing utilities */
class ParameterUITester {
public:
    /** Test slider parameter binding */
    static bool testSliderParameterBinding(juce::Slider& slider,
                                          juce::AudioParameterFloat& parameter,
                                          juce::AudioProcessorValueTreeState& apvts);
    
    /** Test button parameter binding */
    static bool testButtonParameterBinding(juce::Button& button,
                                          juce::AudioParameterBool& parameter,
                                          juce::AudioProcessorValueTreeState& apvts);
    
    /** Test combobox parameter binding */
    static bool testComboBoxParameterBinding(juce::ComboBox& comboBox,
                                           juce::AudioParameterChoice& parameter,
                                           juce::AudioProcessorValueTreeState& apvts);
    
    /** Test parameter automation from UI */
    static bool testParameterAutomationFromUI(juce::Component& parameterComponent,
                                             juce::RangedAudioParameter& parameter);
    
    /** Test parameter value formatting */
    static bool testParameterValueFormatting(juce::AudioParameterFloat& parameter,
                                           const std::vector<float>& testValues);
    
    /** Test parameter smoothing visualization */
    static bool testParameterSmoothingVisualization(juce::Component& component,
                                                   juce::AudioParameterFloat& parameter);
};

//==============================================================================
/** Editor testing utilities */
class EditorTester {
public:
    /** Test editor instantiation with processor */
    static bool testEditorInstantiation(std::function<std::unique_ptr<juce::AudioProcessorEditor>()> createEditor);
    
    /** Test editor size constraints */
    static bool testEditorSizeConstraints(juce::AudioProcessorEditor& editor,
                                         int minWidth = 200, int minHeight = 150,
                                         int maxWidth = 1200, int maxHeight = 800);
    
    /** Test editor parameter synchronization */
    static bool testEditorParameterSync(juce::AudioProcessorEditor& editor,
                                       juce::AudioProcessor& processor);
    
    /** Test editor preset management */
    static bool testEditorPresetManagement(juce::AudioProcessorEditor& editor,
                                          juce::AudioProcessor& processor);
    
    /** Test editor undo/redo functionality */
    static bool testEditorUndoRedo(juce::AudioProcessorEditor& editor,
                                  juce::AudioProcessor& processor);
    
    /** Test editor with different scaling factors */
    static bool testEditorScaling(juce::AudioProcessorEditor& editor,
                                 const std::vector<float>& scalingFactors = {0.75f, 1.0f, 1.25f, 1.5f, 2.0f});
    
    /** Test editor in different host environments */
    static bool testEditorInDifferentHosts(juce::AudioProcessorEditor& editor);
    
    /** Performance test editor redraw */
    static bool testEditorRedrawPerformance(juce::AudioProcessorEditor& editor,
                                           int numRedraws = 1000);
};

//==============================================================================
/** Visual testing utilities */
class VisualTester {
public:
    struct VisualTestResult {
        bool passed;
        juce::String errorMessage;
        juce::Image capturedImage;
        std::vector<juce::Rectangle<int>> problemAreas;
    };
    
    /** Capture component as image for visual comparison */
    static juce::Image captureComponent(juce::Component& component);
    
    /** Compare two images for visual differences */
    static VisualTestResult compareImages(const juce::Image& reference,
                                         const juce::Image& test,
                                         float tolerance = 0.95f);
    
    /** Test component rendering at different DPI scales */
    static bool testDPIScaling(juce::Component& component,
                              const std::vector<float>& dpiScales = {1.0f, 1.25f, 1.5f, 2.0f});
    
    /** Test component in dark/light modes */
    static bool testColorModes(juce::Component& component);
    
    /** Validate color contrast for accessibility */
    static bool validateColorContrast(juce::Component& component);
    
    /** Test component with different system fonts */
    static bool testWithDifferentFonts(juce::Component& component);
    
    /** Create visual regression test baseline */
    static bool createVisualBaseline(juce::Component& component,
                                   const juce::String& testName,
                                   const juce::String& baselinePath);
    
    /** Run visual regression test against baseline */
    static VisualTestResult runVisualRegressionTest(juce::Component& component,
                                                   const juce::String& testName,
                                                   const juce::String& baselinePath);
};

//==============================================================================
/** Automated UI interaction simulator */
class UIInteractionSimulator {
public:
    /** Simulate mouse clicks on interactive components */
    static bool simulateMouseClicks(juce::Component& rootComponent,
                                   int numRandomClicks = 100);
    
    /** Simulate keyboard input */
    static bool simulateKeyboardInput(juce::Component& component,
                                     const juce::String& testInput = "Test123!@#");
    
    /** Simulate drag operations */
    static bool simulateDragOperations(juce::Component& rootComponent,
                                      int numDragOperations = 50);
    
    /** Simulate parameter automation via UI */
    static bool simulateParameterSweeps(juce::Component& rootComponent,
                                       int sweepDurationMs = 5000);
    
    /** Stress test UI with rapid interactions */
    static bool stressTestUI(juce::Component& rootComponent,
                           int testDurationSeconds = 30);
    
    /** Simulate DAW host interactions */
    static bool simulateDAWHostInteractions(juce::AudioProcessorEditor& editor);
    
    /** Test UI responsiveness under load */
    static bool testUIResponsiveness(juce::Component& rootComponent,
                                   int backgroundLoadThreads = 4);
};

//==============================================================================
/** Layout testing utilities */
class LayoutTester {
public:
    /** Test responsive layout at different sizes */
    static bool testResponsiveLayout(juce::Component& component,
                                   const std::vector<juce::Rectangle<int>>& testSizes);
    
    /** Validate component positioning and overlaps */
    static bool validateComponentPositioning(juce::Component& rootComponent);
    
    /** Test flexbox layout behavior */
    static bool testFlexBoxLayout(juce::Component& component,
                                 const juce::FlexBox& flexBox);
    
    /** Test grid layout behavior */
    static bool testGridLayout(juce::Component& component,
                              const juce::Grid& grid);
    
    /** Test component alignment and spacing */
    static bool testAlignmentAndSpacing(juce::Component& rootComponent,
                                       int expectedSpacing = 8);
    
    /** Validate text wrapping and truncation */
    static bool validateTextLayout(juce::Component& rootComponent);
    
    /** Test layout with different text sizes */
    static bool testLayoutWithDifferentTextSizes(juce::Component& component);
};

//==============================================================================
/** Animation and timing testing */
class AnimationTester {
public:
    /** Test smooth parameter animations */
    static bool testParameterAnimations(juce::Component& component,
                                       juce::AudioParameterFloat& parameter,
                                       int animationDurationMs = 1000);
    
    /** Test UI update frequency */
    static bool testUIUpdateFrequency(juce::Component& component,
                                     int expectedFPS = 30);
    
    /** Test animation performance */
    static bool testAnimationPerformance(juce::Component& component,
                                        int numConcurrentAnimations = 10);
    
    /** Test timer-based updates */
    static bool testTimerUpdates(juce::Component& component,
                                int timerIntervalMs = 50);
    
    /** Validate smooth transitions */
    static bool validateSmoothTransitions(juce::Component& component);
};

//==============================================================================
/** Accessibility testing utilities */
class AccessibilityTester {
public:
    struct AccessibilityReport {
        bool hasAccessibleNames;
        bool hasAccessibleDescriptions;
        bool hasProperFocusOrder;
        bool hasKeyboardNavigation;
        bool hasScreenReaderSupport;
        bool hasProperColorContrast;
        std::vector<juce::String> issues;
    };
    
    /** Comprehensive accessibility audit */
    static AccessibilityReport auditAccessibility(juce::Component& rootComponent);
    
    /** Test screen reader compatibility */
    static bool testScreenReaderCompatibility(juce::Component& component);
    
    /** Test keyboard-only navigation */
    static bool testKeyboardOnlyNavigation(juce::Component& component);
    
    /** Validate focus indicators */
    static bool validateFocusIndicators(juce::Component& component);
    
    /** Test with high contrast mode */
    static bool testHighContrastMode(juce::Component& component);
    
    /** Validate text scaling support */
    static bool validateTextScaling(juce::Component& component,
                                   const std::vector<float>& scalingFactors = {1.0f, 1.25f, 1.5f, 2.0f});
};

//==============================================================================
/** UI test reporting and documentation */
class UITestReporter {
public:
    /** Generate comprehensive UI test report */
    static void generateUITestReport(const juce::String& testName,
                                    const std::vector<juce::String>& testResults,
                                    const std::vector<juce::Image>& screenshots,
                                    const juce::String& outputPath);
    
    /** Create interactive test report with screenshots */
    static void createInteractiveTestReport(const juce::String& testName,
                                           const std::vector<VisualTester::VisualTestResult>& results,
                                           const juce::String& outputPath);
    
    /** Export UI metrics to JSON */
    static void exportUIMetrics(const juce::String& componentName,
                               const juce::Rectangle<int>& bounds,
                               int numChildren,
                               const std::vector<juce::String>& properties,
                               const juce::String& outputPath);
    
    /** Generate accessibility compliance report */
    static void generateAccessibilityReport(const AccessibilityTester::AccessibilityReport& report,
                                           const juce::String& outputPath);
};

//==============================================================================
/** Mock objects for UI testing */
class UIMockObjects {
public:
    /** Create mock audio processor for UI testing */
    static std::unique_ptr<juce::AudioProcessor> createMockAudioProcessor();
    
    /** Create mock parameter for UI binding tests */
    static std::unique_ptr<juce::AudioParameterFloat> createMockParameter(
        const juce::String& paramID = "mockParam",
        const juce::String& paramName = "Mock Parameter",
        float minValue = 0.0f,
        float maxValue = 100.0f,
        float defaultValue = 50.0f);
    
    /** Create test component hierarchy */
    static std::unique_ptr<juce::Component> createTestComponentHierarchy();
    
    /** Create mock look and feel for testing */
    static std::unique_ptr<juce::LookAndFeel> createMockLookAndFeel();
    
    /** Create simulated host environment */
    static void setupMockHostEnvironment();
};

} // namespace UITestUtilities  
} // namespace TylerAudio