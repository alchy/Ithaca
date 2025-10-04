/**
 * @file InstrumentMetadata.h
 * @brief Načítání a správa metadat nástroje z JSON souboru
 *
 * Načítá instrument-definition.json z adresáře se samply.
 * Podporuje atributy jako instrumentName, instrumentVersion, etc.
 */

#pragma once

#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include "ithaca-core/sampler/core_logger.h"

/**
 * @struct InstrumentMetadata
 * @brief Metadata nástroje načtená z JSON souboru
 */
struct InstrumentMetadata {
    juce::String instrumentName;        // Název nástroje (např. "VintageV Electric Piano")
    juce::String instrumentVersion;     // Verze nástroje (např. "1.0.0")
    juce::String author;                // Autor/tvůrce nástroje
    juce::String description;           // Popis nástroje
    juce::String category;              // Kategorie (Piano, Synth, etc.)
    int sampleCount = 0;                // Počet samplů (optional)

    /**
     * @brief Načte metadata z JSON souboru
     * @param jsonFilePath Cesta k instrument-definition.json
     * @return InstrumentMetadata nebo nullopt při chybě
     */
    static std::optional<InstrumentMetadata> loadFromFile(const juce::File& jsonFilePath);

    /**
     * @brief Načte metadata z JSON stringu
     * @param jsonString JSON string
     * @return InstrumentMetadata nebo nullopt při chybě
     */
    static std::optional<InstrumentMetadata> loadFromString(const juce::String& jsonString);

    /**
     * @brief Vytvoří výchozí metadata (fallback)
     * @param fallbackName Záložní název
     * @return InstrumentMetadata s výchozími hodnotami
     */
    static InstrumentMetadata createDefault(const juce::String& fallbackName = "Unknown Instrument");

    /**
     * @brief Uloží metadata do JSON souboru
     * @param jsonFilePath Cesta k výstupnímu souboru
     * @return true při úspěchu
     */
    bool saveToFile(const juce::File& jsonFilePath) const;
};

/**
 * @class InstrumentMetadataLoader
 * @brief Helper pro načítání metadat ze sample directory
 */
class InstrumentMetadataLoader {
public:
    /**
     * @brief Načte metadata z adresáře se samply
     * @param sampleDirectory Adresář s instrumentem
     * @param logger Optional logger pro zaznamenávání
     * @return InstrumentMetadata (fallback pokud JSON neexistuje)
     */
    static InstrumentMetadata loadFromDirectory(const juce::File& sampleDirectory, Logger* logger = nullptr);

    /**
     * @brief Zkontroluje zda existuje instrument-definition.json
     * @param sampleDirectory Adresář s instrumentem
     * @return true pokud JSON soubor existuje
     */
    static bool hasMetadataFile(const juce::File& sampleDirectory);

private:
    static constexpr const char* METADATA_FILENAME = "instrument-definition.json";
};
