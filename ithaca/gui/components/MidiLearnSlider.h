/**
 * @file MidiLearnSlider.h
 * @brief Custom Slider that supports right-click for MIDI Learn
 * 
 * This slider extends juce::Slider to intercept right-click events
 * before they are consumed by the slider's default mouse handling.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

/**
 * @class MidiLearnSlider
 * @brief Slider with right-click menu support for MIDI Learn
 * 
 * Problem: JUCE Slider consumes all mouse events for dragging.
 * Solution: Override mouseDown() to catch right-clicks first.
 * 
 * Usage example:
 * @code
 * auto slider = std::make_unique<MidiLearnSlider>();
 * slider->setRange(0.0, 127.0, 1.0);
 * slider->setValue(100.0);
 * 
 * slider->setRightClickCallback([this, slider](juce::Point<int> pos) {
 *     showMidiLearnMenu(slider.get(), pos);
 * });
 * @endcode
 */
class MidiLearnSlider : public juce::Slider {
public:
    /**
     * @brief Callback type for right-click events
     * @param position Mouse position relative to slider
     */
    using RightClickCallback = std::function<void(juce::Point<int>)>;
    
    /**
     * @brief Default constructor
     */
    MidiLearnSlider() 
        : juce::Slider() 
    {
    }
    
    /**
     * @brief Constructor with style and text box position
     * @param style Slider style (LinearHorizontal, LinearVertical, etc.)
     * @param textBoxPosition Text box position (NoTextBox, TextBoxLeft, etc.)
     */
    MidiLearnSlider(juce::Slider::SliderStyle style, 
                   juce::Slider::TextEntryBoxPosition textBoxPosition)
        : juce::Slider(style, textBoxPosition) 
    {
    }
    
    /**
     * @brief Set callback for right-click events
     * @param callback Function to call on right-click
     * @note If callback is set, right-clicks will NOT be passed to base Slider
     */
    void setRightClickCallback(RightClickCallback callback) 
    {
        rightClickCallback_ = callback;
    }
    
    /**
     * @brief Check if right-click callback is set
     * @return true if callback is registered
     */
    bool hasRightClickCallback() const 
    {
        return static_cast<bool>(rightClickCallback_);
    }
    
protected:
    /**
     * @brief Override mouseDown to catch right-clicks
     * @param event Mouse event
     * 
     * If right-click AND callback is set:
     *   - Call callback
     *   - Do NOT pass event to base Slider (prevents drag start)
     * 
     * Otherwise:
     *   - Pass to base Slider for normal left-click handling
     */
    void mouseDown(const juce::MouseEvent& event) override 
    {
        // Check for right-click BEFORE passing to base class
        if (event.mods.isRightButtonDown() && rightClickCallback_) {
            // Call callback with mouse position
            rightClickCallback_(event.getPosition());
            return;  // Don't pass to base class - prevents slider drag
        }
        
        // Normal left-click handling - pass to base Slider
        juce::Slider::mouseDown(event);
    }

private:
    RightClickCallback rightClickCallback_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnSlider)
};