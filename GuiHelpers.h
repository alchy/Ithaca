/**
 * @file GuiHelpers.h
 * @brief GUI helper funkce pro vytváření komponent a styling
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GuiConstants.h"

class GuiHelpers {
public:
    // ZMĚNA: Horizontální slidery místo vertikálních
    static std::unique_ptr<juce::Slider> createCompactSlider(
        double min, double max, double defaultVal, double interval = 1.0);
    
    // ZACHOVAT: Původní styling logiku
    static void styleSlider(juce::Slider& slider, bool debugMode);
    
    // ZACHOVAT: Původní label styling
    static std::unique_ptr<juce::Label> createSliderLabel(const juce::String& text, bool debugMode = false);
    static std::unique_ptr<juce::Label> createSmallLabel(const juce::String& text, bool debugMode = false);
    static std::unique_ptr<juce::Label> createInfoLabel(const juce::String& text, bool debugMode = false);
    
    // Layout helpers pro horizontální uspořádání
    static void positionHorizontalSliderWithLabel(juce::Rectangle<int>& area, 
                                                  juce::Label* label, 
                                                  juce::Slider* slider);
    
    static juce::Rectangle<int> layoutTwoColumnSliders(
        juce::Rectangle<int> totalArea, 
        juce::Rectangle<int>& leftColumn, 
        juce::Rectangle<int>& rightColumn);
    
    // ZACHOVAT: Původní overlay vykreslování
    static void applyControlAreaOverlay(juce::Graphics& g, juce::Rectangle<int> area);
    static void applyDebugBackground(juce::Graphics& g, juce::Rectangle<int> area);
    
    // Debug mode detection
    static bool isDebugModeEnabled();
    
    // Text update helper
    static void updateLabelText(juce::Label* label, const juce::String& newText);

private:
    // Disable instantiation
    GuiHelpers() = delete;
};