#ifndef GUI_HELPERS_H
#define GUI_HELPERS_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include "ParameterDefaults.h"

//==============================================================================
/**
 * @class GuiHelpers
 * @brief Factory functions for creating consistent GUI components
 */
class GuiHelpers
{
public:
    //==============================================================================
    // SLIDER CREATION
    
    /**
     * @brief Create horizontal slider without text box
     */
    static std::unique_ptr<juce::Slider> createCompactSlider(
        double min = MIDI_MIN_VALUE, 
        double max = MIDI_MAX_VALUE, 
        double defaultVal = 64.0, 
        double interval = MIDI_STEP_VALUE);
    
    /**
     * @brief Apply consistent styling to slider based on mode
     */
    static void styleSlider(juce::Slider& slider, bool debugMode = false);
    
    //==============================================================================
    // LABEL CREATION
    
    /**
     * @brief Create label for slider controls
     */
    static std::unique_ptr<juce::Label> createSliderLabel(const juce::String& text);
    
    /**
     * @brief Create small info label
     */
    static std::unique_ptr<juce::Label> createInfoLabel(const juce::String& text);
    
    /**
     * @brief Apply consistent styling to label based on mode
     */
    static void styleLabel(juce::Label& label, bool isSliderLabel = false, bool debugMode = false);
    
    //==============================================================================
    // LAYOUT HELPERS
    
    /**
     * @brief Position slider with label in area
     * @param area Area to remove space from
     * @param label Label to position (can be nullptr)
     * @param slider Slider to position (can be nullptr)
     */
    static void positionSliderWithLabel(juce::Rectangle<int>& area, 
                                       juce::Label* label, 
                                       juce::Slider* slider);
    
    /**
     * @brief Position info label in area
     * @param area Area to remove space from
     * @param label Label to position
     * @param spacing Extra spacing after label
     */
    static void positionInfoLabel(juce::Rectangle<int>& area, 
                                 juce::Label* label, 
                                 int spacing = 4);

private:
    GuiHelpers() = delete; // Static class
};

//==============================================================================
/**
 * @class DebugHelper
 * @brief Debug output helper with conditional compilation
 */
class DebugHelper
{
public:
    static void print(const juce::String& message);
    static void setDebugMode(bool enabled) { debugEnabled = enabled; }
    
private:
    static bool debugEnabled;
};

#endif // GUI_HELPERS_H