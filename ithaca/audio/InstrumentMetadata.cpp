/**
 * @file InstrumentMetadata.cpp
 * @brief Implementace načítání metadat nástroje z JSON
 */

#include "ithaca/audio/InstrumentMetadata.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

// ============================================================================
// InstrumentMetadata - Static methods
// ============================================================================

std::optional<InstrumentMetadata> InstrumentMetadata::loadFromFile(const juce::File& jsonFilePath)
{
    if (!jsonFilePath.existsAsFile()) {
        std::cerr << "InstrumentMetadata: File not found: "
                  << jsonFilePath.getFullPathName().toStdString() << std::endl;
        return std::nullopt;
    }

    try {
        // Načíst soubor
        auto content = jsonFilePath.loadFileAsString();
        return loadFromString(content);
    }
    catch (const std::exception& e) {
        std::cerr << "InstrumentMetadata: Error loading file: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<InstrumentMetadata> InstrumentMetadata::loadFromString(const juce::String& jsonString)
{
    try {
        // Parse JSON
        auto j = json::parse(jsonString.toStdString());

        InstrumentMetadata metadata;

        // Načíst povinné pole
        if (j.contains("instrumentName") && j["instrumentName"].is_string()) {
            metadata.instrumentName = juce::String(j["instrumentName"].get<std::string>());
        } else {
            std::cerr << "InstrumentMetadata: Missing required field 'instrumentName'" << std::endl;
            return std::nullopt;
        }

        // Načíst volitelná pole
        if (j.contains("instrumentVersion") && j["instrumentVersion"].is_string()) {
            metadata.instrumentVersion = juce::String(j["instrumentVersion"].get<std::string>());
        }

        if (j.contains("author") && j["author"].is_string()) {
            metadata.author = juce::String(j["author"].get<std::string>());
        }

        if (j.contains("description") && j["description"].is_string()) {
            metadata.description = juce::String(j["description"].get<std::string>());
        }

        if (j.contains("category") && j["category"].is_string()) {
            metadata.category = juce::String(j["category"].get<std::string>());
        }

        if (j.contains("sampleCount") && j["sampleCount"].is_number_integer()) {
            metadata.sampleCount = j["sampleCount"].get<int>();
        }

        std::cout << "InstrumentMetadata: Loaded '"
                  << metadata.instrumentName.toStdString() << "'" << std::endl;

        return metadata;
    }
    catch (const json::parse_error& e) {
        std::cerr << "InstrumentMetadata: JSON parse error: " << e.what() << std::endl;
        return std::nullopt;
    }
    catch (const std::exception& e) {
        std::cerr << "InstrumentMetadata: Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

InstrumentMetadata InstrumentMetadata::createDefault(const juce::String& fallbackName)
{
    InstrumentMetadata metadata;
    metadata.instrumentName = fallbackName;
    metadata.instrumentVersion = "1.0.0";
    metadata.author = "Unknown";
    metadata.description = "No description available";
    metadata.category = "Unknown";
    metadata.sampleCount = 0;
    return metadata;
}

bool InstrumentMetadata::saveToFile(const juce::File& jsonFilePath) const
{
    try {
        json j;
        j["instrumentName"] = instrumentName.toStdString();
        j["instrumentVersion"] = instrumentVersion.toStdString();
        j["author"] = author.toStdString();
        j["description"] = description.toStdString();
        j["category"] = category.toStdString();
        j["sampleCount"] = sampleCount;

        // Zapsat do souboru s odsazením
        std::ofstream file(jsonFilePath.getFullPathName().toStdString());
        if (!file.is_open()) {
            std::cerr << "InstrumentMetadata: Cannot open file for writing: "
                      << jsonFilePath.getFullPathName().toStdString() << std::endl;
            return false;
        }

        file << j.dump(4); // 4 spaces indent
        file.close();

        std::cout << "InstrumentMetadata: Saved to "
                  << jsonFilePath.getFullPathName().toStdString() << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "InstrumentMetadata: Error saving file: " << e.what() << std::endl;
        return false;
    }
}

// ============================================================================
// InstrumentMetadataLoader - Static methods
// ============================================================================

InstrumentMetadata InstrumentMetadataLoader::loadFromDirectory(const juce::File& sampleDirectory)
{
    if (!sampleDirectory.isDirectory()) {
        std::cerr << "InstrumentMetadataLoader: Not a directory: "
                  << sampleDirectory.getFullPathName().toStdString() << std::endl;
        return InstrumentMetadata::createDefault();
    }

    // Cesta k JSON souboru
    auto jsonFile = sampleDirectory.getChildFile(METADATA_FILENAME);

    // Zkusit načíst JSON
    auto metadataOpt = InstrumentMetadata::loadFromFile(jsonFile);

    if (metadataOpt.has_value()) {
        return metadataOpt.value();
    }

    // Fallback - použít název adresáře jako název nástroje
    std::cout << "InstrumentMetadataLoader: Using directory name as fallback" << std::endl;
    return InstrumentMetadata::createDefault(sampleDirectory.getFileName());
}

bool InstrumentMetadataLoader::hasMetadataFile(const juce::File& sampleDirectory)
{
    auto jsonFile = sampleDirectory.getChildFile(METADATA_FILENAME);
    return jsonFile.existsAsFile();
}
