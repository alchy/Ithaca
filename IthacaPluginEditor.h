#pragma once

#include "IthacaPluginProcessor.h"
#include "decorators/BinaryData.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

/**
 * @class IthacaPluginEditor
 * @brief GUI editor pro IthacaCore sampler plugin s real-time voice monitoring a obrázkovým pozadím.
 */
class IthacaPluginEditor final : public juce::AudioProcessorEditor,
                                 private juce::Timer
{
public:
    explicit IthacaPluginEditor (IthacaPluginProcessor&);
    ~IthacaPluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;

private:
    IthacaPluginProcessor& processorRef;
    int timerCallCounter = 0;

    // ===== GUI Components =====
    std::unique_ptr<juce::Label> engineStatusLabel;
    std::unique_ptr<juce::Label> sampleRateLabel;
    std::unique_ptr<juce::Label> totalSamplesLabel;

    std::unique_ptr<juce::Label> activeVoicesLabel;
    std::unique_ptr<juce::Label> sustainingVoicesLabel;
    std::unique_ptr<juce::Label> releasingVoicesLabel;

    std::unique_ptr<juce::Label> masterGainLabel;
    std::unique_ptr<juce::Label> masterPanLabel;

    std::unique_ptr<juce::Label> sampleDirLabel;

    // Obrázkový komponent pro pozadí
    std::unique_ptr<juce::ImageComponent> imageComponent;

    // ===== Voice Activity Grid Component - FULL DEFINITION =====
    class VoiceActivityComponent : public juce::Component
    {
    public:
        VoiceActivityComponent();
        void paint (juce::Graphics& g) override;
        void resized() override;

        void updateVoiceStates (int active, int sustaining, int releasing);
        void setVoiceState (uint8_t midiNote, bool isActive, int voiceState);

    private:
        static constexpr int GRID_COLS = 16;
        static constexpr int GRID_ROWS = 8;
        static constexpr int CELL_SIZE = 12;
        static constexpr int CELL_PADDING = 1;

        std::array<int, 128> voiceStates {};
        juce::uint32 lastRepaintTime = 0;

        juce::Rectangle<int> getCellBounds (int row, int col) const;
        juce::Colour getStateColour (int state) const;
        int getMidiNoteFromGrid (int row, int col) const;
    };

    std::unique_ptr<VoiceActivityComponent> voiceActivityGrid;

    // ===== Internal Methods =====
    void timerCallback() override;
    void setupMainComponents();
    void setupLabels();
    void setupVoiceActivityGrid();
    void updateStatsDisplay();
    void updateVoiceActivityDisplay();
    void updateMasterControlsDisplay();

    std::unique_ptr<juce::Label> createLabel (const juce::String& text,
                                              juce::Justification justification = juce::Justification::centredLeft);
    void updateLabelText (juce::Label* label, const juce::String& newText);

    static constexpr juce::uint32 BG_COLOR = 0xffffffff;
    static constexpr juce::uint32 TEXT_COLOR = 0xff333333;
    static constexpr juce::uint32 ACCENT_COLOR = 0xff0066cc;
    static constexpr juce::uint32 BORDER_COLOR = 0xffcccccc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IthacaPluginEditor)
};
