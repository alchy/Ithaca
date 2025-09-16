#pragma once

#include "IthacaPluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * @class IthacaPluginEditor - MINIMÁLNÍ VERZE PRO DEBUGGING HANG PROBLÉMU
 * @brief Základní GUI editor pro IthacaCore sampler plugin
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

private:
    // Reference to processor
    IthacaPluginProcessor& processorRef;

    //==============================================================================
    // MINIMÁLNÍ GUI komponenty pro test
    std::unique_ptr<juce::Label> testLabel;

    //==============================================================================
    // Timer callback - VYPNUTÝ pro debugging
    void timerCallback() override;
    
    // Prázdné metody pro postupné přidávání funkcí
    void setupParameterControls();
    void setupMonitoringComponents();
    void setupSampleManagement();
    void browseSamplesButtonClicked();
    void updateStatsDisplay();
    void updateVoiceActivityDisplay();
    
    //==============================================================================
    // Minimální VoiceActivityComponent
    class VoiceActivityComponent : public juce::Component
    {
    public:
        VoiceActivityComponent();
        void paint (juce::Graphics& g) override;
        void updateVoiceStates(int active, int sustaining, int releasing);
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IthacaPluginEditor)
};