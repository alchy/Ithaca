#pragma once

#include "IthacaPluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * @class IthacaPluginEditor - VERZE s kompaktními slidery pro MIDI parametry
 * @brief GUI editor pro IthacaCore sampler plugin s real-time monitoringem a ovládáním
 */
class IthacaPluginEditor final : public juce::AudioProcessorEditor,
                                 private juce::Timer,
                                 private juce::Slider::Listener
{
public:
    explicit IthacaPluginEditor (IthacaPluginProcessor&);
    ~IthacaPluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;

    //==============================================================================
    // Slider listener
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // Reference to processor
    IthacaPluginProcessor& processorRef;

    //==============================================================================
    // GUI UPDATE TIMING
    static constexpr int GUI_TIMER_FPS = 30;
    static constexpr int STATS_UPDATE_DIVIDER = 10;
    int timerCallCounter = 0;

    //==============================================================================
    // MAIN GUI COMPONENTS
    
    // Status display (existující)
    std::unique_ptr<juce::Label> engineStatusLabel;
    std::unique_ptr<juce::Label> sampleRateLabel;
    std::unique_ptr<juce::Label> totalSamplesLabel;
    
    // Voice counters (existující)
    std::unique_ptr<juce::Label> activeVoicesLabel;
    std::unique_ptr<juce::Label> sustainingVoicesLabel;
    std::unique_ptr<juce::Label> releasingVoicesLabel;
    
    // Info labels (existující)
    std::unique_ptr<juce::Label> instrumentLabel;
    std::unique_ptr<juce::Label> versionLabel;

    //==============================================================================
    // COMPACT SLIDER CONTROLS - kompaktní ovládání pro omezený prostor
    
    // Master controls
    std::unique_ptr<juce::Slider> masterGainSlider;
    std::unique_ptr<juce::Label> masterGainLabel;
    std::unique_ptr<juce::Slider> masterPanSlider;
    std::unique_ptr<juce::Label> masterPanLabel;
    
    // ADSR Envelope
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> releaseLabel;
    std::unique_ptr<juce::Slider> sustainLevelSlider;
    std::unique_ptr<juce::Label> sustainLevelLabel;
    
    // LFO Panning
    std::unique_ptr<juce::Slider> lfoPanSpeedSlider;
    std::unique_ptr<juce::Label> lfoPanSpeedLabel;
    std::unique_ptr<juce::Slider> lfoPanDepthSlider;
    std::unique_ptr<juce::Label> lfoPanDepthLabel;

    //==============================================================================
    // SLIDER ATTACHMENTS - propojení se JUCE parametry
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;

    //==============================================================================
    // Voice Activity Grid Component (existující)
    class VoiceActivityComponent;
    std::unique_ptr<VoiceActivityComponent> voiceActivityGrid;

    // Background image component (existující)
    juce::ImageComponent imageComponent;

    //==============================================================================
    // Timer callback for updates
    void timerCallback() override;
    
    // Setup methods
    void setupMainComponents();
    void setupCompactSliders();        // NOVÉ: nastavení kompaktních sliderů
    void setupSliderAttachments();     // NOVÉ: propojení s JUCE parametry
    void setupLabels();
    void setupVoiceActivityGrid();
    
    // Update methods
    void updateStatsDisplay();
    void updateVoiceActivityDisplay();
    void updateMasterControlsDisplay();
    void updateSliderValues();          // NOVÉ: aktualizace slider hodnot

    //==============================================================================
    // Slider creation helpers
    std::unique_ptr<juce::Slider> createCompactSlider(double min, double max, double defaultVal, 
                                                      double interval = 1.0, const juce::String& suffix = "");
    std::unique_ptr<juce::Label> createSliderLabel(const juce::String& text);
    
    //==============================================================================
    // Voice Activity Grid Component (existující definice)
    class VoiceActivityComponent : public juce::Component
    {
    public:
        VoiceActivityComponent();
        void paint (juce::Graphics& g) override;
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

    //==============================================================================
    // Helper methods (existující)
    std::unique_ptr<juce::Label> createLabel(const juce::String& text, 
                                           juce::Justification justification = juce::Justification::centredLeft);
    std::unique_ptr<juce::Label> createSmallLabel(const juce::String& text, 
                                                 juce::Justification justification = juce::Justification::centredLeft);
    void updateLabelText(juce::Label* label, const juce::String& newText);
    
    // Color scheme (existující)
    static constexpr juce::uint32 BG_COLOR = 0xffffffff;
    static constexpr juce::uint32 TEXT_COLOR = 0xff333333;
    static constexpr juce::uint32 ACCENT_COLOR = 0xff0066cc;
    static constexpr juce::uint32 BORDER_COLOR = 0xffcccccc;
    static constexpr juce::uint32 ACTIVE_VOICE_COLOR = 0xff00cc66;
    static constexpr juce::uint32 RELEASING_VOICE_COLOR = 0xffff6600;

    // Slider specific colors
    static constexpr juce::uint32 SLIDER_TRACK_COLOR = 0xff444444;
    static constexpr juce::uint32 SLIDER_THUMB_COLOR = 0xff0066cc;
    static constexpr juce::uint32 SLIDER_TEXT_COLOR = 0xffffffff;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IthacaPluginEditor)
};