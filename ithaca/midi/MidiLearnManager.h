/**
 * @file MidiLearnManager.h
 * @brief MIDI Learn funkčnost - mapování CC na parametry
 *
 * Umožňuje uživatelům:
 * - Pravý klik na slider → "Learn MIDI CC"
 * - Poslat MIDI CC zprávu z controlleru
 * - Automaticky přiřadit CC k parametru
 * - Uložit/načíst mappings
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include <functional>
#include "ithaca-core/sampler/core_logger.h"

/**
 * @class MidiLearnManager
 * @brief Správa MIDI Learn mappingů
 */
class MidiLearnManager {
public:
    /**
     * @brief Struktura pro jeden MIDI CC mapping
     */
    struct Mapping {
        uint8_t ccNumber = 0;           // MIDI CC číslo (0-127)
        juce::String parameterID;       // ID parametru v APVTS
        juce::String displayName;       // Název pro GUI
        
        bool isValid() const { return !parameterID.isEmpty(); }
    };
    
    /**
     * @brief Callback pro změnu learning state
     * @param isLearning true když je learning mode aktivní
     * @param parameterID ID parametru který se učí
     */
    using LearningStateCallback = std::function<void(bool isLearning, const juce::String& parameterID)>;
    
    // ========================================================================
    // Constructor
    // ========================================================================

    explicit MidiLearnManager(Logger* logger = nullptr);
    
    // ========================================================================
    // Learning Interface
    // ========================================================================
    
    /**
     * @brief Začne learning mode pro daný parametr
     * @param parameterID ID parametru k naučení
     * @param displayName Název parametru pro GUI
     */
    void startLearning(const juce::String& parameterID, const juce::String& displayName);
    
    /**
     * @brief Zruší learning mode
     */
    void cancelLearning();
    
    /**
     * @brief Pokus o naučení CC čísla (volá se z MIDI processoru)
     * @param ccNumber MIDI CC číslo (0-127)
     * @return true pokud bylo přiřazení úspěšné
     */
    bool tryLearnCC(uint8_t ccNumber);
    
    /**
     * @brief Zjistí, zda je learning mode aktivní
     * @return true pokud čekáme na MIDI CC
     */
    bool isLearning() const { return isLearning_; }
    
    /**
     * @brief Získá ID parametru, který se právě učí
     * @return Parameter ID nebo prázdný string
     */
    juce::String getCurrentLearningParameter() const { return learningParameterID_; }
    
    // ========================================================================
    // Mapping Management
    // ========================================================================
    
    /**
     * @brief Přidá nebo aktualizuje mapping
     * @param ccNumber MIDI CC číslo
     * @param parameterID ID parametru
     * @param displayName Název parametru
     */
    void setMapping(uint8_t ccNumber, const juce::String& parameterID, const juce::String& displayName);
    
    /**
     * @brief Odstraní mapping pro dané CC číslo
     * @param ccNumber MIDI CC číslo
     */
    void removeMapping(uint8_t ccNumber);
    
    /**
     * @brief Odstraní mapping pro daný parametr
     * @param parameterID ID parametru
     */
    void removeMappingForParameter(const juce::String& parameterID);
    
    /**
     * @brief Vymaže všechny mappings
     */
    void clearAllMappings();
    
    /**
     * @brief Získá mapping pro dané CC číslo
     * @param ccNumber MIDI CC číslo
     * @return Mapping nebo nullptr
     */
    const Mapping* getMapping(uint8_t ccNumber) const;
    
    /**
     * @brief Získá CC číslo pro daný parametr
     * @param parameterID ID parametru
     * @return CC číslo nebo -1 pokud není namapováno
     */
    int getCCNumberForParameter(const juce::String& parameterID) const;
    
    /**
     * @brief Získá všechny aktivní mappings
     * @return Mapa CC number → Mapping
     */
    const std::map<uint8_t, Mapping>& getAllMappings() const { return mappings_; }
    
    // ========================================================================
    // Persistence (Save/Load)
    // ========================================================================
    
    /**
     * @brief Uloží mappings do XML
     * @return XmlElement s mappings
     */
    std::unique_ptr<juce::XmlElement> saveToXml() const;
    
    /**
     * @brief Načte mappings z XML
     * @param xml XmlElement s mappings
     */
    void loadFromXml(const juce::XmlElement* xml);
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * @brief Nastaví callback pro změny learning state
     * @param callback Funkce volaná při změně stavu
     */
    void setLearningStateCallback(LearningStateCallback callback) {
        learningStateCallback_ = callback;
    }

private:
    // ========================================================================
    // State
    // ========================================================================

    Logger* logger_ = nullptr;                          // Logger instance (optional)
    bool isLearning_ = false;                           // Learning mode aktivní?
    juce::String learningParameterID_;                  // Který parametr se učí
    juce::String learningDisplayName_;                  // Název pro GUI

    std::map<uint8_t, Mapping> mappings_;              // CC number → Mapping

    LearningStateCallback learningStateCallback_;       // Callback pro GUI
    
    // ========================================================================
    // Helper Methods
    // ========================================================================
    
    /**
     * @brief Notifikuje callback o změně learning state
     */
    void notifyLearningStateChanged();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnManager)
};