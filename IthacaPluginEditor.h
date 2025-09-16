#pragma once

#include "IthacaPluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * @class IthacaPluginEditor - ROZŠÍŘENÁ VERZE s minimalistickým GUI
 * @brief Základní GUI editor pro IthacaCore sampler plugin s real-time monitoring
 */
class IthacaPluginEditor final : public juce::AudioProcessorEditor,
                                 private juce::Timer
{
public:
    explicit IthacaPluginEditor (IthacaPluginProcessor&);
    ~IthacaPluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override; // Pro bezpečné spuštění timeru

private:
    // Reference to processor
    IthacaPluginProcessor& processorRef;

    //==============================================================================
    // GUI UPDATE TIMING
    static constexpr int GUI_TIMER_FPS = 30;  // 30fps for voice activity
    static constexpr int STATS_UPDATE_DIVIDER = 10; // Every 10 timer calls = ~3fps for stats
    int timerCallCounter = 0;

    //==============================================================================
    // MAIN GUI COMPONENTS
    
    // Status display
    std::unique_ptr<juce::Label> engineStatusLabel;
    std::unique_ptr<juce::Label> sampleRateLabel;
    std::unique_ptr<juce::Label> totalSamplesLabel;
    
    // Voice counters
    std::unique_ptr<juce::Label> activeVoicesLabel;
    std::unique_ptr<juce::Label> sustainingVoicesLabel;
    std::unique_ptr<juce::Label> releasingVoicesLabel;
    
    // Master controls display
    std::unique_ptr<juce::Label> masterGainLabel;
    std::unique_ptr<juce::Label> masterPanLabel;
    
    // Sample directory info
    std::unique_ptr<juce::Label> sampleDirLabel;

    //==============================================================================
    // Voice Activity Grid Component - FORWARD DECLARATION
    class VoiceActivityComponent; // Forward declaration
    std::unique_ptr<VoiceActivityComponent> voiceActivityGrid;

    //==============================================================================
    // Timer callback for updates
    void timerCallback() override;
    
    // Setup methods
    void setupMainComponents();
    void setupLabels();
    void setupVoiceActivityGrid();
    
    // Update methods
    void updateStatsDisplay();      // ~3fps updates
    void updateVoiceActivityDisplay(); // 30fps updates
    void updateMasterControlsDisplay();
    
    //==============================================================================
    // Voice Activity Grid Component - FULL DEFINITION
    class VoiceActivityComponent : public juce::Component
    {
    public:
        VoiceActivityComponent();
        void paint (juce::Graphics& g) override;
        void resized() override;
        
        void updateVoiceStates(int active, int sustaining, int releasing);
        void setVoiceState(uint8_t midiNote, bool isActive, int voiceState); // 0=idle, 1=sustaining, 2=releasing
        
    private:
        static constexpr int GRID_COLS = 16; // 16 columns
        static constexpr int GRID_ROWS = 8;  // 8 rows = 128 notes
        static constexpr int CELL_SIZE = 12;
        static constexpr int CELL_PADDING = 1;
        
        // Voice states: 0=idle, 1=sustaining, 2=releasing
        std::array<int, 128> voiceStates{};
        
        // Helper methods
        juce::Rectangle<int> getCellBounds(int row, int col) const;
        juce::Colour getStateColour(int state) const;
        int getMidiNoteFromGrid(int row, int col) const;
    };

    //==============================================================================
    // Helper methods
    std::unique_ptr<juce::Label> createLabel(const juce::String& text, 
                                           juce::Justification justification = juce::Justification::centredLeft);
    void updateLabelText(juce::Label* label, const juce::String& newText);
    
    // Color scheme (white theme)
    static constexpr juce::uint32 BG_COLOR = 0xffffffff;          // White background  
    static constexpr juce::uint32 TEXT_COLOR = 0xff333333;        // Dark gray text
    static constexpr juce::uint32 ACCENT_COLOR = 0xff0066cc;      // Blue accent
    static constexpr juce::uint32 BORDER_COLOR = 0xffcccccc;      // Light gray borders
    static constexpr juce::uint32 ACTIVE_VOICE_COLOR = 0xff00cc66; // Green for active
    static constexpr juce::uint32 RELEASING_VOICE_COLOR = 0xffff6600; // Orange for releasing

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IthacaPluginEditor)
};