/**
 * @file IthacaPluginEditor.h (Refactored)
 * @brief Zjednodušený hlavní editor s horizontálními slidery - maximálně 200-300 řádků
 */

#pragma once

#include "IthacaPluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// Forward declarations pro specializované komponenty
class SliderPanelComponent;
class InfoPanelComponent;

//==============================================================================
/**
 * @class IthacaPluginEditor - REFAKTOROVANÁ VERZE s horizontálními slidery
 * @brief Hlavní GUI editor s delegací na specializované komponenty
 */
class IthacaPluginEditor final : public juce::AudioProcessorEditor {
public:
    explicit IthacaPluginEditor(IthacaPluginProcessor&);
    ~IthacaPluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;   // ZACHOVAT: Původní paint logiku
    void resized() override;                // ZJEDNODUŠIT: Pouze layout komponent
    void parentHierarchyChanged() override; // ZACHOVAT: Timer management

private:
    IthacaPluginProcessor& processorRef;

    //==============================================================================
    // Specializované komponenty
    std::unique_ptr<SliderPanelComponent> sliderPanel;
    std::unique_ptr<InfoPanelComponent> infoPanel;
    juce::ImageComponent imageComponent;  // ZACHOVAT: Původní background

    //==============================================================================
    // ZACHOVAT: Debug mode handling a původní konstanty
    bool debugMode_;
    
    // ZACHOVAT: Všechny původní makra a konstanty
    static constexpr bool BACKGROUND_PICTURE_OFF_STATIC = false; // Compile-time constant
    
    //==============================================================================
    // Setup methods
    void initializeComponents();
    void setupBackground();    // ZACHOVAT: Původní background logic
    
    // Utility methods
    bool isDebugModeEnabled() const;
    
    //==============================================================================
    // ZACHOVAT: Původní VoiceActivityComponent pro kompatibilitu (prázdná implementace)
    class VoiceActivityComponent : public juce::Component
    {
    public:
        VoiceActivityComponent();
        void paint(juce::Graphics& g) override;
        void resized() override;
        
        void updateVoiceStates(int active, int sustaining, int releasing);
        void setVoiceState(uint8_t midiNote, bool isActive, int voiceState);
        
    private:
        static constexpr int GRID_COLS = 16;
        static constexpr int GRID_ROWS = 8;
        static constexpr int CELL_SIZE = 12;
        static constexpr int CELL_PADDING = 1;
        
        std::array<int, 128> voiceStates{};
        
        juce::Rectangle<int> getCellBounds(int row, int col) const;
        juce::Colour getStateColour(int state) const;
        int getMidiNoteFromGrid(int row, int col) const;
    };
    
    // Pro kompatibilitu s původním kódem
    std::unique_ptr<VoiceActivityComponent> voiceActivityGrid;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IthacaPluginEditor)
};