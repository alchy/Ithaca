#include "MidiCCManager.h"
#include "ithaca-core/sampler/voice_manager.h"

//==============================================================================
MidiCCManager::MidiCCManager(juce::AudioProcessorValueTreeState& parameters)
    : parameters_(parameters)
    , masterGainParam_(nullptr)
    , masterPanParam_(nullptr)
    , attackParam_(nullptr)
    , releaseParam_(nullptr)
    , sustainLevelParam_(nullptr)
    , lfoPanSpeedParam_(nullptr)
    , lfoPanDepthParam_(nullptr)
{
}

void MidiCCManager::setParameterPointers(std::atomic<float>* masterGain,
                                         std::atomic<float>* masterPan,
                                         std::atomic<float>* attack,
                                         std::atomic<float>* release,
                                         std::atomic<float>* sustainLevel,
                                         std::atomic<float>* lfoPanSpeed,
                                         std::atomic<float>* lfoPanDepth)
{
    masterGainParam_ = masterGain;
    masterPanParam_ = masterPan;
    attackParam_ = attack;
    releaseParam_ = release;
    sustainLevelParam_ = sustainLevel;
    lfoPanSpeedParam_ = lfoPanSpeed;
    lfoPanDepthParam_ = lfoPanDepth;
}

//==============================================================================
void MidiCCManager::processMidiControlChange(VoiceManager* voiceManager, 
                                            uint8_t channel, 
                                            uint8_t controller, 
                                            uint8_t value) noexcept
{
    juce::ignoreUnused(channel); // Currently we don't distinguish MIDI channels
    
    // RT-SAFE: All processing must be real-time safe
    switch (controller) {
        case CC_MASTER_GAIN:
            handleMasterGainCC(value);
            break;
            
        case CC_MASTER_PAN:
            handleMasterPanCC(voiceManager, value);
            break;
            
        case CC_ATTACK:
            handleAttackCC(voiceManager, value);
            break;
            
        case CC_RELEASE:
            handleReleaseCC(voiceManager, value);
            break;
            
        case CC_SUSTAIN_LEVEL:
            handleSustainLevelCC(voiceManager, value);
            break;
            
        case CC_LFO_PAN_SPEED:
            handleLfoPanSpeedCC(voiceManager, value);
            break;
            
        case CC_LFO_PAN_DEPTH:
            handleLfoPanDepthCC(voiceManager, value);
            break;
            
        case CC_ALL_SOUND_OFF:
        case CC_ALL_NOTES_OFF:
            handleSystemCC(voiceManager, controller, value);
            break;
            
        default:
            // Unknown CC - ignore silently in RT context
            break;
    }
}

//==============================================================================
// PRIVATE HELPER METHODS

void MidiCCManager::handleMasterGainCC(uint8_t value) noexcept
{
    // Master gain requires Logger - not RT-safe to call VoiceManager directly
    // Instead, update JUCE parameter which will be applied in updateSamplerParametersRTSafe()
    if (masterGainParam_) {
        masterGainParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleMasterPanCC(VoiceManager* voiceManager, uint8_t value) noexcept
{
    if (voiceManager) {
        voiceManager->setAllVoicesPanMIDI(value);
    }
    
    // Sync with JUCE parameter
    if (masterPanParam_) {
        masterPanParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleAttackCC(VoiceManager* voiceManager, uint8_t value) noexcept
{
    if (voiceManager) {
        voiceManager->setAllVoicesAttackMIDI(value);
    }
    
    // Sync with JUCE parameter
    if (attackParam_) {
        attackParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleReleaseCC(VoiceManager* voiceManager, uint8_t value) noexcept
{
    if (voiceManager) {
        voiceManager->setAllVoicesReleaseMIDI(value);
    }
    
    // Sync with JUCE parameter
    if (releaseParam_) {
        releaseParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleSustainLevelCC(VoiceManager* voiceManager, uint8_t value) noexcept
{
    if (voiceManager) {
        voiceManager->setAllVoicesSustainLevelMIDI(value);
    }
    
    // Sync with JUCE parameter
    if (sustainLevelParam_) {
        sustainLevelParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleLfoPanSpeedCC(VoiceManager* voiceManager, uint8_t value) noexcept
{
    if (voiceManager) {
        voiceManager->setAllVoicesPanSpeedMIDI(value);
    }
    
    // Sync with JUCE parameter
    if (lfoPanSpeedParam_) {
        lfoPanSpeedParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleLfoPanDepthCC(VoiceManager* voiceManager, uint8_t value) noexcept
{
    if (voiceManager) {
        voiceManager->setAllVoicesPanDepthMIDI(value);
    }
    
    // Sync with JUCE parameter
    if (lfoPanDepthParam_) {
        lfoPanDepthParam_->store(static_cast<float>(value));
    }
}

void MidiCCManager::handleSystemCC(VoiceManager* voiceManager, uint8_t controller, uint8_t value) noexcept
{
    if (!voiceManager) return;
    
    // MIDI standard: value >= 64 activates function
    if (value >= 64) {
        switch (controller) {
            case CC_ALL_SOUND_OFF:
            case CC_ALL_NOTES_OFF:
                voiceManager->stopAllVoices();
                break;
        }
    }
}