/**
 * @file MidiProcessor.cpp
 * @brief Implementation of MIDI event processing
 */

#include "MidiProcessor.h"
#include "ithaca-core/sampler/voice_manager.h"
#include "MidiCCDefinitions.h"

//==============================================================================
// Constructor / Destructor

MidiProcessor::MidiProcessor()
    : totalMidiEventsProcessed_(0)
{
}

//==============================================================================
// Main MIDI Processing

void MidiProcessor::processMidiBuffer(const juce::MidiBuffer& midiMessages,
                                      VoiceManager* voiceManager,
                                      juce::AudioProcessorValueTreeState& parameters)
{
    // Early exit if no VoiceManager
    if (!voiceManager) {
        return;
    }
    
    // Process each MIDI message in the buffer
    for (const auto& midiMetadata : midiMessages) {
        auto message = midiMetadata.getMessage();
        
        // Increment event counter
        totalMidiEventsProcessed_.fetch_add(1, std::memory_order_relaxed);
        
        // Route message to appropriate handler
        if (message.isNoteOn()) {
            // MIDI Note On event
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            uint8_t velocity = static_cast<uint8_t>(message.getVelocity());
            
            voiceManager->setNoteStateMIDI(midiNote, true, velocity);
        }
        else if (message.isNoteOff()) {
            // MIDI Note Off event
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            
            voiceManager->setNoteStateMIDI(midiNote, false);
        }
        else if (message.isController()) {
            // MIDI Control Change event
            uint8_t ccNumber = static_cast<uint8_t>(message.getControllerNumber());
            uint8_t ccValue = static_cast<uint8_t>(message.getControllerValue());
            
            processMidiControlChange(ccNumber, ccValue, parameters);
        }
        // Future expansion: pitch bend, aftertouch, program change, etc.
        // else if (message.isPitchWheel()) { ... }
        // else if (message.isAftertouch()) { ... }
    }
}

//==============================================================================
// Statistics

int MidiProcessor::getTotalMidiEventsProcessed() const
{
    return totalMidiEventsProcessed_.load(std::memory_order_relaxed);
}

void MidiProcessor::resetStatistics()
{
    totalMidiEventsProcessed_.store(0, std::memory_order_relaxed);
}

//==============================================================================
// Private Processing Methods

void MidiProcessor::processMidiControlChange(uint8_t ccNumber, 
                                            uint8_t ccValue,
                                            juce::AudioProcessorValueTreeState& parameters)
{
    // Get parameter for this CC number
    auto* param = getParameterForCC(ccNumber, parameters);
    
    if (!param) {
        // CC not mapped to any parameter - ignore silently for RT performance
        return;
    }
    
    // Convert MIDI CC value (0-127) to normalized parameter value (0.0-1.0)
    // Special handling for pan parameter if needed
    float normalizedValue;
    
    if (ccNumber == MidiCC::MASTER_PAN) {
        // Pan has special conversion (MIDI 0-127 → GUI -64 to +63)
        normalizedValue = MidiCC::ccPanToNormalized(ccValue);
    } else {
        // Standard linear conversion
        normalizedValue = MidiCC::ccValueToNormalized(ccValue);
    }
    
    // Update parameter value and notify host
    // This is RT-safe as it only sets an atomic value
    param->setValueNotifyingHost(normalizedValue);
}

juce::RangedAudioParameter* MidiProcessor::getParameterForCC(
    uint8_t ccNumber,
    juce::AudioProcessorValueTreeState& parameters)
{
    // Use MidiCCDefinitions to map CC number to parameter ID
    const char* parameterID = MidiCC::getParameterIDForCC(ccNumber);
    
    if (!parameterID) {
        // CC not mapped
        return nullptr;
    }
    
    // Get parameter from APVTS
    return parameters.getParameter(parameterID);
}