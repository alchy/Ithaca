/**
 * @file ParameterAttachmentManager.h
 * @brief Správa parameter attachments pro slidery
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

class ParameterAttachmentManager {
public:
    // Struktura pro organizaci sliderů
    struct SliderSet {
        juce::Slider* masterGain = nullptr;
        juce::Slider* masterPan = nullptr;
        juce::Slider* attack = nullptr;
        juce::Slider* release = nullptr;
        juce::Slider* sustainLevel = nullptr;
        juce::Slider* lfoPanSpeed = nullptr;
        juce::Slider* lfoPanDepth = nullptr;
    };
    
    ParameterAttachmentManager() = default;
    ~ParameterAttachmentManager() = default;
    
    // HLAVNÍ METODA: Vytvoří všechny attachments
    bool createAllAttachments(juce::AudioProcessorValueTreeState& parameters, 
                             const SliderSet& sliders);
    
    // Utility metody
    size_t getAttachmentCount() const { return attachments_.size(); }
    void clearAttachments() { attachments_.clear(); }
    
    // Debug metody
    void logAttachmentStatus() const;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SliderAttachment>> attachments_;
    
    // Helper metody pro individual attachments
    bool createAttachment(juce::AudioProcessorValueTreeState& parameters,
                         const juce::String& parameterID,
                         juce::Slider* slider);
    
    bool parameterExists(juce::AudioProcessorValueTreeState& parameters,
                        const juce::String& parameterID) const;
};