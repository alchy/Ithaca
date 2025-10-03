/**
 * @file ParameterManager.h
 * @brief Správa JUCE parametrů a jejich propojení s IthacaCore VoiceManager
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

// Forward declarations
class VoiceManager;
class Logger;

/**
 * @class ParameterManager
 * @brief Specializovaná třída pro správu všech JUCE parametrů
 * 
 * Zodpovědnost:
 * - Vytvoření parameter layout pro JUCE ValueTreeState
 * - Udržování pointerů na atomic parameter hodnoty
 * - RT-safe update parametrů do VoiceManager
 * - Konverze GUI hodnot na MIDI formát (0-127)
 */
class ParameterManager {
public:
    ParameterManager() = default;
    ~ParameterManager() = default;
    
    // ===== PARAMETER LAYOUT CREATION =====
    
    /**
     * @brief Vytvoří kompletní parameter layout pro ValueTreeState
     * @return JUCE ParameterLayout se všemi 7 parametry
     * 
     * Parametry:
     * - masterGain: 0-127, default 100
     * - masterPan: -64 to +63, default 0 (center)
     * - attack: 0-127, default 0
     * - release: 0-127, default 4
     * - sustainLevel: 0-127, default 127
     * - lfoPanSpeed: 0-127, default 0
     * - lfoPanDepth: 0-127, default 0
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // ===== PARAMETER POINTER INITIALIZATION =====
    
    /**
     * @brief Inicializuje všechny parameter pointery ze ValueTreeState
     * @param parameters Reference na ValueTreeState
     * @return true pokud byly všechny parametry nalezeny
     */
    bool initializeParameterPointers(juce::AudioProcessorValueTreeState& parameters);
    
    // ===== RT-SAFE PARAMETER UPDATES =====
    
    /**
     * @brief RT-safe update všech parametrů do VoiceManager
     * @param voiceManager Pointer na VoiceManager (může být nullptr)
     * @param logger Reference na Logger pro non-RT parametry (používá se pro BBE)
     *
     * @note Volána z processBlock() - musí být RT-safe
     * @note Konvertuje GUI hodnoty na MIDI formát (0-127)
     */
    void updateSamplerParametersRTSafe(VoiceManager* voiceManager, Logger& logger);
    
    // ===== PARAMETER ACCESS =====
    
    /**
     * @brief Získej aktuální hodnotu master gain
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentMasterGain() const;
    
    /**
     * @brief Získej aktuální hodnotu master pan
     * @return MIDI hodnota 0-127 (64 = center)
     */
    uint8_t getCurrentMasterPan() const;
    
    /**
     * @brief Získej aktuální hodnotu attack
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentAttack() const;
    
    /**
     * @brief Získej aktuální hodnotu release
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentRelease() const;
    
    /**
     * @brief Získej aktuální hodnotu sustain level
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentSustainLevel() const;
    
    /**
     * @brief Získej aktuální hodnotu LFO pan speed
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentLfoPanSpeed() const;
    
    /**
     * @brief Získej aktuální hodnotu LFO pan depth
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentLfoPanDepth() const;

    /**
     * @brief Získej aktuální hodnotu stereo field
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentStereoField() const;

    /**
     * @brief Získej aktuální hodnotu BBE definition
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentBbeDefinition() const;

    /**
     * @brief Získej aktuální hodnotu BBE bass boost
     * @return MIDI hodnota 0-127
     */
    uint8_t getCurrentBbeBassBoost() const;

    // ===== VALIDATION =====
    
    /**
     * @brief Zkontroluj zda jsou všechny parameter pointery platné
     * @return true pokud jsou všechny pointery non-null
     */
    bool areParametersValid() const;

private:
    // ===== PARAMETER POINTERS =====
    // Atomic pointery na parameter hodnoty ze JUCE ValueTreeState
    
    std::atomic<float>* masterGainParam_ = nullptr;
    std::atomic<float>* masterPanParam_ = nullptr;
    std::atomic<float>* attackParam_ = nullptr;
    std::atomic<float>* releaseParam_ = nullptr;
    std::atomic<float>* sustainLevelParam_ = nullptr;
    std::atomic<float>* lfoPanSpeedParam_ = nullptr;
    std::atomic<float>* lfoPanDepthParam_ = nullptr;
    std::atomic<float>* stereoFieldParam_ = nullptr;
    std::atomic<float>* bbeDefinitionParam_ = nullptr;
    std::atomic<float>* bbeBassBoostParam_ = nullptr;

    // ===== CHANGE DETECTION =====
    // Cached hodnoty pro detekci změn - eliminuje zbytečné VoiceManager volání

    uint8_t lastMasterGain_ = 100;
    uint8_t lastMasterPan_ = 64;
    uint8_t lastAttack_ = 0;
    uint8_t lastRelease_ = 4;
    uint8_t lastSustainLevel_ = 127;
    uint8_t lastLfoPanSpeed_ = 0;
    uint8_t lastLfoPanDepth_ = 0;
    uint8_t lastStereoField_ = 0;
    uint8_t lastBbeDefinition_ = 64;
    uint8_t lastBbeBassBoost_ = 32;
    
    // ===== HELPER METHODS =====
    
    /**
     * @brief Bezpečná konverze float na uint8_t MIDI hodnotu
     * @param value Float hodnota
     * @param min Minimální hodnota
     * @param max Maximální hodnota
     * @return MIDI hodnota 0-127
     */
    static uint8_t convertToMidiValue(float value, float min = 0.0f, float max = 127.0f);
    
    /**
     * @brief Konverze pan hodnoty z GUI formátu (-64 to +63) na MIDI (0-127)
     * @param panValue Pan hodnota z GUI
     * @return MIDI pan hodnota 0-127 (64 = center)
     */
    static uint8_t convertPanToMidi(float panValue);
    
    // ===== PARAMETER CREATION HELPERS =====
    
    /**
     * @brief Helper pro vytvoření standardního MIDI parametru (0-127)
     * @param id Parameter ID
     * @param name Display name
     * @param defaultValue Default hodnota
     * @return Unique pointer na AudioParameterFloat
     */
    static std::unique_ptr<juce::AudioParameterFloat> createMidiParameter(
        const juce::String& id, const juce::String& name, float defaultValue);
    
    /**
     * @brief Helper pro vytvoření pan parametru (-64 to +63)
     * @param id Parameter ID
     * @param name Display name
     * @return Unique pointer na AudioParameterFloat
     */
    static std::unique_ptr<juce::AudioParameterFloat> createPanParameter(
        const juce::String& id, const juce::String& name);
    
    // Disable copy/move
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterManager)
};