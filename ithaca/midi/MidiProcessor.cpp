/**
 * @file MidiProcessor.cpp (s MIDI Learn podporou a Sustain Pedal)
 * @brief Implementace MIDI processingu s learning a sustain pedal
 */

#include "MidiProcessor.h"
#include "MidiLearnManager.h"
#include "ithaca-core/sampler/voice_manager.h"
#include "MidiCCDefinitions.h"

// ============================================================================
// Constructor
// ============================================================================

MidiProcessor::MidiProcessor()
    : totalMidiEventsProcessed_(0)
{
}

// ============================================================================
// Main MIDI Processing
// ============================================================================

void MidiProcessor::processMidiBuffer(const juce::MidiBuffer& midiMessages,
                                      VoiceManager* voiceManager,
                                      juce::AudioProcessorValueTreeState& parameters,
                                      MidiLearnManager* midiLearnManager)
{
    if (!voiceManager) return;
    
    for (const auto& midiMetadata : midiMessages) {
        auto message = midiMetadata.getMessage();
        
        totalMidiEventsProcessed_.fetch_add(1, std::memory_order_relaxed);
        
        if (message.isNoteOn()) {
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            uint8_t velocity = static_cast<uint8_t>(message.getVelocity());
            voiceManager->setNoteStateMIDI(midiNote, true, velocity);
        }
        else if (message.isNoteOff()) {
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            voiceManager->setNoteStateMIDI(midiNote, false);
        }
        else if (message.isController()) {
            uint8_t ccNumber = static_cast<uint8_t>(message.getControllerNumber());
            uint8_t ccValue = static_cast<uint8_t>(message.getControllerValue());
            
            // ================================================================
            // PRIORITA 1: Sustain Pedal (CC64) - NOVĚ PŘIDÁNO
            // ================================================================
            if (MidiCC::isDamperPedal(ccNumber)) {
                processSustainPedal(ccValue, voiceManager);
                continue; // Skip další processing pro CC64
            }
            
            // ================================================================
            // PRIORITA 2: MIDI Learn (pokud je aktivní)
            // ================================================================
            if (midiLearnManager && midiLearnManager->isLearning()) {
                if (midiLearnManager->tryLearnCC(ccNumber)) {
                    continue; // CC bylo naučeno, neprocházej dál
                }
            }
            
            // ================================================================
            // PRIORITA 3: Normální CC processing (s learned mappings)
            // ================================================================
            processMidiControlChange(ccNumber, ccValue, parameters, midiLearnManager);
        }
    }
}

// ============================================================================
// Statistics
// ============================================================================

int MidiProcessor::getTotalMidiEventsProcessed() const
{
    return totalMidiEventsProcessed_.load(std::memory_order_relaxed);
}

void MidiProcessor::resetStatistics()
{
    totalMidiEventsProcessed_.store(0, std::memory_order_relaxed);
}

// ============================================================================
// Private Processing Methods
// ============================================================================

void MidiProcessor::processSustainPedal(uint8_t ccValue, VoiceManager* voiceManager)
{
    if (!voiceManager) return;
    
    // Convert MIDI value to pedal state
    // Standard MIDI: ≤63 = OFF, ≥64 = ON
    bool pedalDown = MidiCC::ccValueToPedalState(ccValue);
    
    // Delegate to VoiceManager (RT-safe)
    voiceManager->setSustainPedalMIDI(pedalDown);
    
    // Optional: Log in debug mode (non-RT context only)
#if ENABLE_MIDI_CC_LOGGING
    std::cout << "[MidiProcessor] Sustain Pedal (CC64): " 
              << (pedalDown ? "DOWN" : "UP") 
              << " (value=" << static_cast<int>(ccValue) << ")" 
              << std::endl;
#endif
}

void MidiProcessor::processMidiControlChange(uint8_t ccNumber, 
                                            uint8_t ccValue,
                                            juce::AudioProcessorValueTreeState& parameters,
                                            MidiLearnManager* midiLearnManager)
{
    // Získej parametr pro toto CC (včetně learned mappings)
    auto* param = getParameterForCC(ccNumber, parameters, midiLearnManager);
    
    if (!param) return;
    
    // Convert MIDI CC value (0-127) → normalized (0.0-1.0)
    float normalizedValue;
    
    if (ccNumber == MidiCC::MASTER_PAN) {
        normalizedValue = MidiCC::ccPanToNormalized(ccValue);
    } else {
        normalizedValue = MidiCC::ccValueToNormalized(ccValue);
    }
    
    // Update parameter
    param->setValueNotifyingHost(normalizedValue);
}

juce::RangedAudioParameter* MidiProcessor::getParameterForCC(
    uint8_t ccNumber,
    juce::AudioProcessorValueTreeState& parameters,
    MidiLearnManager* midiLearnManager)
{
    // 1. PRIORITA: Zkus MIDI Learn mappings
    if (midiLearnManager) {
        const auto* mapping = midiLearnManager->getMapping(ccNumber);
        if (mapping && mapping->isValid()) {
            auto* param = parameters.getParameter(mapping->parameterID);
            if (param) {
                return param; // Našli jsme learned mapping
            }
        }
    }
    
    // 2. FALLBACK: Zkus default mappings z MidiCCDefinitions
    const char* parameterID = MidiCC::getParameterIDForCC(ccNumber);
    if (parameterID) {
        return parameters.getParameter(parameterID);
    }
    
    return nullptr;
}