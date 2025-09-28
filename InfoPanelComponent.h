/**
 * @file InfoPanelComponent.h
 * @brief Specializovaná komponenta pro info labely a timer updates
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "IthacaPluginProcessor.h" // Forward declaration assumption
#include "GuiHelpers.h"

class InfoPanelComponent : public juce::Component,
                          private juce::Timer {
public:
    explicit InfoPanelComponent(IthacaPluginProcessor& processor);
    ~InfoPanelComponent() override;
    
    // Component overrides
    void paint(juce::Graphics&) override;  // Transparentní - bez pozadí
    void resized() override;               // Layout info labelů
    
    // Timer override
    void timerCallback() override;         // ZACHOVAT: Původní timer logiku
    
    // Public control methods
    void startUpdates();
    void stopUpdates(); 
    void setDebugMode(bool enabled);

private:
    // Reference k processoru
    IthacaPluginProcessor& processorRef_;
    
    // ZACHOVAT: Všechny původní info labely
    std::unique_ptr<juce::Label> activeVoicesLabel;
    std::unique_ptr<juce::Label> sustainingVoicesLabel;
    std::unique_ptr<juce::Label> releasingVoicesLabel;
    std::unique_ptr<juce::Label> sampleRateLabel;
    std::unique_ptr<juce::Label> instrumentLabel;
    std::unique_ptr<juce::Label> versionLabel;
    
    // State
    bool debugMode_ = false;
    static bool staticInfoSet_;
    
    // ZACHOVAT: Původní timer konstany
    static constexpr int TIMER_INTERVAL_MS = 300;
    
    // Setup methods
    void setupAllLabels();
    
    // ZACHOVAT: Původní update logiku
    void updateLiveData();        
    void applyFallbackBounds();   // ZACHOVAT: Původní fallback bounds
    
    // Layout methods
    void layoutBackgroundMode(juce::Rectangle<int> bounds);
    void layoutDebugMode(juce::Rectangle<int> bounds);
    
    // Disable copy
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoPanelComponent)
};