/**
 * @file ParameterManager.cpp
 * @brief Implementace správy JUCE parametrů pro IthacaCore
 */

#include "ParameterManager.h"
#include "ithaca-core/sampler/voice_manager.h"
#include "ithaca-core/sampler/core_logger.h"
#include <algorithm>

// ===== PARAMETER LAYOUT CREATION =====

juce::AudioProcessorValueTreeState::ParameterLayout ParameterManager::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Master Gain (0-127, default 100)
    parameters.push_back(createMidiParameter("masterGain", "Master Gain", 100.0f));
    
    // Master Pan (-64 to +63, default 0 = center)
    parameters.push_back(createPanParameter("masterPan", "Master Pan"));
    
    // Attack Time (0-127, default 0 = quick attack)
    parameters.push_back(createMidiParameter("attack", "Attack", 0.0f));
    
    // Release Time (0-127, default 4)
    parameters.push_back(createMidiParameter("release", "Release", 4.0f));
    
    // Sustain Level (0-127, default 127 = maximum)
    parameters.push_back(createMidiParameter("sustainLevel", "Sustain Level", 127.0f));
    
    // LFO Pan Speed (0-127, default 0 = disabled)
    parameters.push_back(createMidiParameter("lfoPanSpeed", "LFO Pan Speed", 0.0f));
    
    // LFO Pan Depth (0-127, default 0 = no effect)
    parameters.push_back(createMidiParameter("lfoPanDepth", "LFO Pan Depth", 0.0f));
    
    return { parameters.begin(), parameters.end() };
}

// ===== PARAMETER POINTER INITIALIZATION =====

bool ParameterManager::initializeParameterPointers(juce::AudioProcessorValueTreeState& parameters)
{
    // Získej pointery na všechny parametry
    masterGainParam_ = parameters.getRawParameterValue("masterGain");
    masterPanParam_ = parameters.getRawParameterValue("masterPan");
    attackParam_ = parameters.getRawParameterValue("attack");
    releaseParam_ = parameters.getRawParameterValue("release");
    sustainLevelParam_ = parameters.getRawParameterValue("sustainLevel");
    lfoPanSpeedParam_ = parameters.getRawParameterValue("lfoPanSpeed");
    lfoPanDepthParam_ = parameters.getRawParameterValue("lfoPanDepth");
    
    // Zkontroluj zda byly všechny parametry nalezeny
    bool allValid = areParametersValid();
    
    if (!allValid) {
        // Log které parametry chybí
        if (!masterGainParam_) { /* log error */ }
        if (!masterPanParam_) { /* log error */ }
        if (!attackParam_) { /* log error */ }
        if (!releaseParam_) { /* log error */ }
        if (!sustainLevelParam_) { /* log error */ }
        if (!lfoPanSpeedParam_) { /* log error */ }
        if (!lfoPanDepthParam_) { /* log error */ }
    }
    
    return allValid;
}

// ===== RT-SAFE PARAMETER UPDATES =====

void ParameterManager::updateSamplerParametersRTSafe(VoiceManager* voiceManager, Logger& logger)
{
    // Early exit pokud VoiceManager není dostupný
    if (!voiceManager || !areParametersValid()) {
        return;
    }
    
    // RT-SAFE version - bez loggingu, používá VoiceManager API
    
    // Master Gain (0-127 MIDI value)
    if (masterGainParam_) {
        uint8_t midiGain = getCurrentMasterGain();
        // POZOR: setAllVoicesMasterGainMIDI není RT-safe (má logging)
        // Pro RT použití by bylo lepší mít RT-safe verzi
        // Pro nyní ponecháme, ale je třeba upravit VoiceManager API
        voiceManager->setAllVoicesMasterGainMIDI(midiGain, logger);
    }
    
    // Master Pan (MIDI value: 0-127, kde 64 = center)
    if (masterPanParam_) {
        uint8_t midiPan = getCurrentMasterPan();
        voiceManager->setAllVoicesPanMIDI(midiPan); // RT-safe
    }
    
    // Attack Time (0-127 MIDI value)
    if (attackParam_) {
        uint8_t midiAttack = getCurrentAttack();
        voiceManager->setAllVoicesAttackMIDI(midiAttack); // RT-safe
    }
    
    // Release Time (0-127 MIDI value)
    if (releaseParam_) {
        uint8_t midiRelease = getCurrentRelease();
        voiceManager->setAllVoicesReleaseMIDI(midiRelease); // RT-safe
    }
    
    // Sustain Level (0-127 MIDI value)
    if (sustainLevelParam_) {
        uint8_t midiSustain = getCurrentSustainLevel();
        voiceManager->setAllVoicesSustainLevelMIDI(midiSustain); // RT-safe
    }
    
    // LFO Pan Speed (0-127 MIDI value)
    if (lfoPanSpeedParam_) {
        uint8_t midiSpeed = getCurrentLfoPanSpeed();
        voiceManager->setAllVoicesPanSpeedMIDI(midiSpeed); // RT-safe
    }
    
    // LFO Pan Depth (0-127 MIDI value)
    if (lfoPanDepthParam_) {
        uint8_t midiDepth = getCurrentLfoPanDepth();
        voiceManager->setAllVoicesPanDepthMIDI(midiDepth); // RT-safe
    }
}

// ===== PARAMETER ACCESS =====

uint8_t ParameterManager::getCurrentMasterGain() const
{
    return masterGainParam_ ? convertToMidiValue(masterGainParam_->load()) : 100;
}

uint8_t ParameterManager::getCurrentMasterPan() const
{
    return masterPanParam_ ? convertPanToMidi(masterPanParam_->load()) : 64;
}

uint8_t ParameterManager::getCurrentAttack() const
{
    return attackParam_ ? convertToMidiValue(attackParam_->load()) : 0;
}

uint8_t ParameterManager::getCurrentRelease() const
{
    return releaseParam_ ? convertToMidiValue(releaseParam_->load()) : 4;
}

uint8_t ParameterManager::getCurrentSustainLevel() const
{
    return sustainLevelParam_ ? convertToMidiValue(sustainLevelParam_->load()) : 127;
}

uint8_t ParameterManager::getCurrentLfoPanSpeed() const
{
    return lfoPanSpeedParam_ ? convertToMidiValue(lfoPanSpeedParam_->load()) : 0;
}

uint8_t ParameterManager::getCurrentLfoPanDepth() const
{
    return lfoPanDepthParam_ ? convertToMidiValue(lfoPanDepthParam_->load()) : 0;
}

// ===== VALIDATION =====

bool ParameterManager::areParametersValid() const
{
    return masterGainParam_ != nullptr &&
           masterPanParam_ != nullptr &&
           attackParam_ != nullptr &&
           releaseParam_ != nullptr &&
           sustainLevelParam_ != nullptr &&
           lfoPanSpeedParam_ != nullptr &&
           lfoPanDepthParam_ != nullptr;
}

// ===== HELPER METHODS =====

uint8_t ParameterManager::convertToMidiValue(float value, float min, float max)
{
    // Clamping na valid range
    float clampedValue = std::clamp(value, min, max);
    
    // Lineární mapování na 0-127 range
    float normalized = (clampedValue - min) / (max - min);
    int midiValue = static_cast<int>(normalized * 127.0f + 0.5f); // Round to nearest
    
    // Finální clamping na MIDI range
    return static_cast<uint8_t>(std::clamp(midiValue, 0, 127));
}

uint8_t ParameterManager::convertPanToMidi(float panValue)
{
    // Pan hodnota: -64 to +63 (GUI format)
    // MIDI pan: 0 to 127 (kde 64 = center)
    
    float clampedPan = std::clamp(panValue, -64.0f, 63.0f);
    int midiPan = static_cast<int>(clampedPan + 64.0f + 0.5f); // +64 offset, round
    
    return static_cast<uint8_t>(std::clamp(midiPan, 0, 127));
}

// ===== PARAMETER CREATION HELPERS =====

std::unique_ptr<juce::AudioParameterFloat> ParameterManager::createMidiParameter(
    const juce::String& id, const juce::String& name, float defaultValue)
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(id, 1),
        name,
        juce::NormalisableRange<float>(0.0f, 127.0f, 1.0f),
        defaultValue,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction([](float value, int) { 
                return juce::String(static_cast<int>(value)); 
            })
    );
}

std::unique_ptr<juce::AudioParameterFloat> ParameterManager::createPanParameter(
    const juce::String& id, const juce::String& name)
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(id, 1),
        name,
        juce::NormalisableRange<float>(-64.0f, 63.0f, 1.0f),
        0.0f, // Center
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction([](float value, int) { 
                if (value < -0.5f) return "L" + juce::String(static_cast<int>(-value));
                else if (value > 0.5f) return "R" + juce::String(static_cast<int>(value));
                else return juce::String("Center");
            })
    );
}