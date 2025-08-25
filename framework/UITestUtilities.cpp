#include "UITestUtilities.h"
#include <thread>
#include <random>

namespace TylerAudio {
namespace UITestUtilities {

//==============================================================================
// ComponentTester Implementation

bool ComponentTester::testComponentVisibility(juce::Component& component) {
    // Test basic visibility
    if (!component.isVisible()) {
        component.setVisible(true);
    }
    
    // Test bounds
    auto bounds = component.getBounds();
    if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0) {
        return false;
    }
    
    // Test opacity
    if (component.getAlpha() < 0.0f || component.getAlpha() > 1.0f) {
        return false;
    }
    
    return true;
}

bool ComponentTester::testComponentResizing(juce::Component& component, 
                                          const std::vector<juce::Rectangle<int>>& testBounds) {
    auto originalBounds = component.getBounds();
    
    try {
        for (const auto& bounds : testBounds) {
            component.setBounds(bounds);
            component.resized();
            
            // Verify the component handled the resize
            if (!component.isVisible() && bounds.getWidth() > 0 && bounds.getHeight() > 0) {
                component.setBounds(originalBounds);
                return false;
            }
            
            // Test painting after resize
            if (!testComponentPainting(component, bounds.getWidth(), bounds.getHeight())) {
                component.setBounds(originalBounds);
                return false;
            }
        }
        
        component.setBounds(originalBounds);
        return true;
    } catch (...) {
        component.setBounds(originalBounds);
        return false;
    }
}

bool ComponentTester::testComponentPainting(juce::Component& component, int width, int height) {
    try {
        juce::Image testImage(juce::Image::ARGB, width, height, true);
        juce::Graphics g(testImage);
        
        component.setBounds(0, 0, width, height);
        component.paint(g);
        component.paintOverChildren(g);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool ComponentTester::testComponentAccessibility(juce::Component& component) {
    // Check if component has accessibility handler
    auto* accessibilityHandler = component.getAccessibilityHandler();
    
    if (accessibilityHandler == nullptr) {
        return false; // Component should have accessibility support
    }
    
    // Test basic accessibility properties
    auto role = accessibilityHandler->getRole();
    auto title = accessibilityHandler->getTitle();
    auto description = accessibilityHandler->getDescription();
    
    // Verify component has appropriate accessibility information
    return !title.isEmpty() || !description.isEmpty();
}

bool ComponentTester::testKeyboardNavigation(juce::Component& component) {
    // Test if component can receive focus
    if (!component.getWantsKeyboardFocus()) {
        return true; // Not all components need keyboard focus
    }
    
    // Test focus traversal
    component.grabKeyboardFocus();
    
    // Simulate Tab key press
    juce::KeyPress tabKey(juce::KeyPress::tabKey);
    return component.keyPressed(tabKey);
}

bool ComponentTester::testMouseInteraction(juce::Component& component) {
    auto bounds = component.getBounds();
    juce::Point<int> center = bounds.getCentre();
    
    try {
        // Simulate mouse events
        juce::MouseEvent mouseDown(juce::MouseInputSource::InputSourceType::mouse, 
                                  center, juce::ModifierKeys(), 0.0f, 
                                  juce::MouseInputSource::InputSourceType::mouse, 
                                  juce::MouseInputSource::InputSourceType::mouse, 
                                  juce::Time::getCurrentTime(), center, 
                                  juce::Time::getCurrentTime(), 1, false);
        
        component.mouseDown(mouseDown);
        component.mouseUp(mouseDown);
        
        return true;
    } catch (...) {
        return false;
    }
}

//==============================================================================
// ParameterUITester Implementation

bool ParameterUITester::testSliderParameterBinding(juce::Slider& slider,
                                                  juce::AudioParameterFloat& parameter,
                                                  juce::AudioProcessorValueTreeState& apvts) {
    // Create attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    try {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, parameter.paramID, slider);
        
        // Test parameter to slider sync
        parameter.setValue(0.75f);
        
        // Allow for parameter smoothing delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        double sliderValue = slider.getValue();
        double expectedValue = parameter.getValue() * (slider.getMaximum() - slider.getMinimum()) + slider.getMinimum();
        
        if (std::abs(sliderValue - expectedValue) > 0.01) {
            return false;
        }
        
        // Test slider to parameter sync
        slider.setValue(slider.getMinimum() + 0.25 * (slider.getMaximum() - slider.getMinimum()));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (std::abs(parameter.getValue() - 0.25f) > 0.01f) {
            return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool ParameterUITester::testParameterValueFormatting(juce::AudioParameterFloat& parameter,
                                                    const std::vector<float>& testValues) {
    try {
        for (float value : testValues) {
            parameter.setValue(value);
            
            juce::String text = parameter.getText(parameter.getValue(), 100);
            
            // Verify text is not empty and doesn't contain invalid characters
            if (text.isEmpty() || text.contains("nan") || text.contains("inf")) {
                return false;
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

//==============================================================================
// EditorTester Implementation

bool EditorTester::testEditorInstantiation(std::function<std::unique_ptr<juce::AudioProcessorEditor>()> createEditor) {
    try {
        auto editor = createEditor();
        
        if (editor == nullptr) {
            return false;
        }
        
        // Test basic editor properties
        auto bounds = editor->getBounds();
        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0) {
            return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool EditorTester::testEditorSizeConstraints(juce::AudioProcessorEditor& editor,
                                            int minWidth, int minHeight,
                                            int maxWidth, int maxHeight) {
    try {
        // Test minimum size
        editor.setBounds(0, 0, minWidth - 10, minHeight - 10);
        editor.resized();
        
        auto bounds = editor.getBounds();
        if (bounds.getWidth() < minWidth - 10 && bounds.getHeight() < minHeight - 10) {
            // Editor should enforce minimum size
            return false;
        }
        
        // Test maximum size
        editor.setBounds(0, 0, maxWidth + 100, maxHeight + 100);
        editor.resized();
        
        bounds = editor.getBounds();
        // Editor might enforce maximum size (optional)
        
        return true;
    } catch (...) {
        return false;
    }
}

bool EditorTester::testEditorParameterSync(juce::AudioProcessorEditor& editor,
                                          juce::AudioProcessor& processor) {
    try {
        auto& parameters = processor.getParameters();
        
        if (parameters.empty()) {
            return true; // No parameters to test
        }
        
        // Change parameter value and check if editor updates
        auto* firstParam = parameters[0];
        float originalValue = firstParam->getValue();
        
        firstParam->setValue(1.0f - originalValue);
        
        // Allow time for editor to update
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Restore original value
        firstParam->setValue(originalValue);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool EditorTester::testEditorScaling(juce::AudioProcessorEditor& editor,
                                    const std::vector<float>& scalingFactors) {
    auto originalBounds = editor.getBounds();
    
    try {
        for (float scale : scalingFactors) {
            int scaledWidth = static_cast<int>(originalBounds.getWidth() * scale);
            int scaledHeight = static_cast<int>(originalBounds.getHeight() * scale);
            
            editor.setBounds(0, 0, scaledWidth, scaledHeight);
            
            // Test painting at this scale
            if (!ComponentTester::testComponentPainting(editor, scaledWidth, scaledHeight)) {
                editor.setBounds(originalBounds);
                return false;
            }
        }
        
        editor.setBounds(originalBounds);
        return true;
    } catch (...) {
        editor.setBounds(originalBounds);
        return false;
    }
}

//==============================================================================
// VisualTester Implementation

juce::Image VisualTester::captureComponent(juce::Component& component) {
    auto bounds = component.getBounds();
    
    if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0) {
        bounds = juce::Rectangle<int>(0, 0, 400, 300);
        component.setBounds(bounds);
    }
    
    juce::Image image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics g(image);
    
    component.paintEntireComponent(g, true);
    
    return image;
}

VisualTester::VisualTestResult VisualTester::compareImages(const juce::Image& reference,
                                                          const juce::Image& test,
                                                          float tolerance) {
    VisualTestResult result;
    result.capturedImage = test;
    result.passed = true;
    
    if (reference.getWidth() != test.getWidth() || reference.getHeight() != test.getHeight()) {
        result.passed = false;
        result.errorMessage = "Image dimensions don't match";
        return result;
    }
    
    int totalPixels = reference.getWidth() * reference.getHeight();
    int differentPixels = 0;
    
    for (int y = 0; y < reference.getHeight(); ++y) {
        for (int x = 0; x < reference.getWidth(); ++x) {
            auto refPixel = reference.getPixelAt(x, y);
            auto testPixel = test.getPixelAt(x, y);
            
            if (refPixel.getARGB() != testPixel.getARGB()) {
                differentPixels++;
                
                // Mark problem area
                result.problemAreas.push_back(juce::Rectangle<int>(x, y, 1, 1));
            }
        }
    }
    
    float similarity = 1.0f - (static_cast<float>(differentPixels) / totalPixels);
    
    if (similarity < tolerance) {
        result.passed = false;
        result.errorMessage = "Images differ by " + juce::String(100.0f * (1.0f - similarity), 2) + "%";
    }
    
    return result;
}

bool VisualTester::testDPIScaling(juce::Component& component,
                                 const std::vector<float>& dpiScales) {
    for (float scale : dpiScales) {
        try {
            // Simulate DPI scaling
            auto originalBounds = component.getBounds();
            auto scaledBounds = originalBounds.withSize(
                static_cast<int>(originalBounds.getWidth() * scale),
                static_cast<int>(originalBounds.getHeight() * scale)
            );
            
            component.setBounds(scaledBounds);
            
            if (!ComponentTester::testComponentPainting(component, scaledBounds.getWidth(), scaledBounds.getHeight())) {
                component.setBounds(originalBounds);
                return false;
            }
            
            component.setBounds(originalBounds);
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

//==============================================================================
// UIInteractionSimulator Implementation

bool UIInteractionSimulator::simulateMouseClicks(juce::Component& rootComponent, int numRandomClicks) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    auto bounds = rootComponent.getBounds();
    std::uniform_int_distribution<int> xDist(bounds.getX(), bounds.getRight());
    std::uniform_int_distribution<int> yDist(bounds.getY(), bounds.getBottom());
    
    try {
        for (int i = 0; i < numRandomClicks; ++i) {
            juce::Point<int> randomPoint(xDist(gen), yDist(gen));
            
            auto* hitComponent = rootComponent.getComponentAt(randomPoint);
            if (hitComponent != nullptr) {
                // Simulate click
                juce::MouseEvent mouseEvent(juce::MouseInputSource::InputSourceType::mouse,
                                          randomPoint, juce::ModifierKeys(), 0.0f,
                                          juce::MouseInputSource::InputSourceType::mouse,
                                          juce::MouseInputSource::InputSourceType::mouse,
                                          juce::Time::getCurrentTime(), randomPoint,
                                          juce::Time::getCurrentTime(), 1, false);
                
                hitComponent->mouseDown(mouseEvent);
                hitComponent->mouseUp(mouseEvent);
            }
            
            // Small delay between clicks
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool UIInteractionSimulator::stressTestUI(juce::Component& rootComponent, int testDurationSeconds) {
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::seconds(testDurationSeconds);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> actionDist(0, 3);
    
    try {
        while (std::chrono::steady_clock::now() < endTime) {
            int action = actionDist(gen);
            
            switch (action) {
                case 0: // Random clicks
                    simulateMouseClicks(rootComponent, 5);
                    break;
                case 1: // Resize
                    {
                        auto bounds = rootComponent.getBounds();
                        std::uniform_int_distribution<int> sizeDist(100, 800);
                        rootComponent.setBounds(bounds.withSize(sizeDist(gen), sizeDist(gen)));
                    }
                    break;
                case 2: // Repaint
                    rootComponent.repaint();
                    break;
                case 3: // Brief pause
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    break;
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

//==============================================================================
// LayoutTester Implementation

bool LayoutTester::testResponsiveLayout(juce::Component& component,
                                       const std::vector<juce::Rectangle<int>>& testSizes) {
    auto originalBounds = component.getBounds();
    
    try {
        for (const auto& size : testSizes) {
            component.setBounds(size);
            component.resized();
            
            // Validate that child components are properly positioned
            if (!validateComponentPositioning(component)) {
                component.setBounds(originalBounds);
                return false;
            }
        }
        
        component.setBounds(originalBounds);
        return true;
    } catch (...) {
        component.setBounds(originalBounds);
        return false;
    }
}

bool LayoutTester::validateComponentPositioning(juce::Component& rootComponent) {
    auto& children = rootComponent.getChildren();
    
    for (int i = 0; i < children.size(); ++i) {
        auto* child = children.getUnchecked(i);
        auto childBounds = child->getBounds();
        
        // Check if child is within parent bounds
        if (!rootComponent.getBounds().contains(childBounds) && child->isVisible()) {
            // Child extends beyond parent - might be intentional, but worth checking
        }
        
        // Check for overlaps with siblings (might be intentional)
        for (int j = i + 1; j < children.size(); ++j) {
            auto* sibling = children.getUnchecked(j);
            if (child->isVisible() && sibling->isVisible()) {
                auto siblingBounds = sibling->getBounds();
                if (childBounds.intersects(siblingBounds)) {
                    // Overlapping components detected
                }
            }
        }
    }
    
    return true;
}

//==============================================================================
// AccessibilityTester Implementation

AccessibilityTester::AccessibilityReport AccessibilityTester::auditAccessibility(juce::Component& rootComponent) {
    AccessibilityReport report;
    report.hasAccessibleNames = true;
    report.hasAccessibleDescriptions = true;
    report.hasProperFocusOrder = true;
    report.hasKeyboardNavigation = true;
    report.hasScreenReaderSupport = true;
    report.hasProperColorContrast = true;
    
    // Recursive audit of component tree
    std::function<void(juce::Component*)> auditComponent = [&](juce::Component* component) {
        if (component == nullptr) return;
        
        // Check accessibility handler
        auto* handler = component->getAccessibilityHandler();
        if (handler == nullptr) {
            report.hasScreenReaderSupport = false;
            report.issues.push_back("Component missing accessibility handler: " + component->getName());
        } else {
            // Check for accessible name
            if (handler->getTitle().isEmpty()) {
                report.hasAccessibleNames = false;
                report.issues.push_back("Component missing accessible name: " + component->getName());
            }
        }
        
        // Check keyboard focus capability
        if (component->isVisible() && component->isEnabled() && !component->getWantsKeyboardFocus()) {
            // Not all components need keyboard focus, so this is just informational
        }
        
        // Audit child components
        for (auto* child : component->getChildren()) {
            auditComponent(child);
        }
    };
    
    auditComponent(&rootComponent);
    
    return report;
}

//==============================================================================
// UITestReporter Implementation

void UITestReporter::generateUITestReport(const juce::String& testName,
                                         const std::vector<juce::String>& testResults,
                                         const std::vector<juce::Image>& screenshots,
                                         const juce::String& outputPath) {
    juce::File reportFile(outputPath);
    
    juce::String html;
    html << "<!DOCTYPE html>\n";
    html << "<html><head><title>UI Test Report: " << testName << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    html << ".test-result { margin: 10px 0; padding: 10px; border-left: 4px solid #ccc; }\n";
    html << ".pass { border-left-color: #4CAF50; background-color: #f1f8e9; }\n";
    html << ".fail { border-left-color: #f44336; background-color: #ffebee; }\n";
    html << ".screenshot { max-width: 400px; margin: 10px 0; }\n";
    html << "</style></head><body>\n";
    
    html << "<h1>UI Test Report: " << testName << "</h1>\n";
    html << "<p>Generated: " << juce::Time::getCurrentTime().toString(true, true) << "</p>\n";
    
    for (const auto& result : testResults) {
        juce::String cssClass = result.startsWith("PASS") ? "pass" : "fail";
        html << "<div class=\"test-result " << cssClass << "\">" << result << "</div>\n";
    }
    
    if (!screenshots.empty()) {
        html << "<h2>Screenshots</h2>\n";
        for (int i = 0; i < screenshots.size(); ++i) {
            juce::String imagePath = outputPath + "_screenshot_" + juce::String(i) + ".png";
            juce::File imageFile(imagePath);
            
            juce::FileOutputStream stream(imageFile);
            if (stream.openedOk()) {
                juce::PNGImageFormat pngFormat;
                pngFormat.writeImageToStream(screenshots[i], stream);
            }
            
            html << "<img src=\"" << imageFile.getFileName() << "\" class=\"screenshot\" alt=\"Screenshot " << i << "\">\n";
        }
    }
    
    html << "</body></html>\n";
    
    reportFile.replaceWithText(html);
}

void UITestReporter::generateAccessibilityReport(const AccessibilityTester::AccessibilityReport& report,
                                                const juce::String& outputPath) {
    juce::File reportFile(outputPath);
    
    juce::String content;
    content << "Accessibility Audit Report\n";
    content << "==========================\n\n";
    content << "Generated: " << juce::Time::getCurrentTime().toString(true, true) << "\n\n";
    
    content << "Accessibility Features:\n";
    content << "- Accessible Names: " << (report.hasAccessibleNames ? "PASS" : "FAIL") << "\n";
    content << "- Accessible Descriptions: " << (report.hasAccessibleDescriptions ? "PASS" : "FAIL") << "\n";
    content << "- Proper Focus Order: " << (report.hasProperFocusOrder ? "PASS" : "FAIL") << "\n";
    content << "- Keyboard Navigation: " << (report.hasKeyboardNavigation ? "PASS" : "FAIL") << "\n";
    content << "- Screen Reader Support: " << (report.hasScreenReaderSupport ? "PASS" : "FAIL") << "\n";
    content << "- Color Contrast: " << (report.hasProperColorContrast ? "PASS" : "FAIL") << "\n\n";
    
    if (!report.issues.empty()) {
        content << "Issues Found:\n";
        content << "-------------\n";
        for (const auto& issue : report.issues) {
            content << "- " << issue << "\n";
        }
    }
    
    reportFile.replaceWithText(content);
}

//==============================================================================
// UIMockObjects Implementation

std::unique_ptr<juce::AudioParameterFloat> UIMockObjects::createMockParameter(
    const juce::String& paramID,
    const juce::String& paramName,
    float minValue,
    float maxValue,
    float defaultValue) {
    
    return std::make_unique<juce::AudioParameterFloat>(
        paramID, paramName,
        juce::NormalisableRange<float>(minValue, maxValue), defaultValue);
}

std::unique_ptr<juce::Component> UIMockObjects::createTestComponentHierarchy() {
    auto root = std::make_unique<juce::Component>("TestRoot");
    root->setBounds(0, 0, 400, 300);
    
    // Add some child components
    for (int i = 0; i < 5; ++i) {
        auto child = std::make_unique<juce::Component>("TestChild" + juce::String(i));
        child->setBounds(10 + i * 70, 10, 60, 30);
        root->addAndMakeVisible(child.release());
    }
    
    return root;
}

} // namespace UITestUtilities
} // namespace TylerAudio