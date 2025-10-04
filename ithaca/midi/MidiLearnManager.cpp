/**
 * @file MidiLearnManager.cpp
 * @brief Implementace MIDI Learn manageru
 */

#include "ithaca/midi/MidiLearnManager.h"

// ============================================================================
// Constructor
// ============================================================================

MidiLearnManager::MidiLearnManager(Logger* logger)
    : logger_(logger),
      isLearning_(false)
{
}

// ============================================================================
// Learning Interface
// ============================================================================

void MidiLearnManager::startLearning(const juce::String& parameterID,
                                     const juce::String& displayName)
{
    isLearning_ = true;
    learningParameterID_ = parameterID;
    learningDisplayName_ = displayName;

    if (logger_) {
        logger_->log("MidiLearnManager/startLearning", LogSeverity::Info,
                    "Started learning for parameter: " + parameterID.toStdString() +
                    " (" + displayName.toStdString() + ")");
    }

    notifyLearningStateChanged();
}

void MidiLearnManager::cancelLearning()
{
    if (!isLearning_) return;

    if (logger_) {
        logger_->log("MidiLearnManager/cancelLearning", LogSeverity::Info,
                    "Cancelled learning for parameter: " + learningParameterID_.toStdString());
    }

    isLearning_ = false;
    learningParameterID_.clear();
    learningDisplayName_.clear();

    notifyLearningStateChanged();
}

bool MidiLearnManager::tryLearnCC(uint8_t ccNumber)
{
    if (!isLearning_) {
        if (logger_) {
            logger_->log("MidiLearnManager/tryLearnCC", LogSeverity::Warning,
                        "Received CC " + std::to_string(ccNumber) + " but not in learning mode");
        }
        return false;
    }

    // Ignoruj CC 120-127 (reserved pro channel messages)
    if (ccNumber >= 120) {
        if (logger_) {
            logger_->log("MidiLearnManager/tryLearnCC", LogSeverity::Warning,
                        "Ignored reserved CC number: " + std::to_string(ccNumber));
        }
        return false;
    }

    if (logger_) {
        logger_->log("MidiLearnManager/tryLearnCC", LogSeverity::Info,
                    "Learning successful: CC " + std::to_string(ccNumber) +
                    " -> " + learningParameterID_.toStdString());
    }

    // Přiřaď CC k parametru
    setMapping(ccNumber, learningParameterID_, learningDisplayName_);

    // Ukonči learning mode
    isLearning_ = false;
    learningParameterID_.clear();
    learningDisplayName_.clear();

    notifyLearningStateChanged();

    return true;
}

// ============================================================================
// Mapping Management
// ============================================================================

void MidiLearnManager::setMapping(uint8_t ccNumber,
                                  const juce::String& parameterID,
                                  const juce::String& displayName)
{
    // Check if there's an existing mapping for this CC number
    auto existingMapping = mappings_.find(ccNumber);
    if (existingMapping != mappings_.end()) {
        if (logger_) {
            logger_->log("MidiLearnManager/setMapping", LogSeverity::Info,
                        "Replacing existing mapping: CC " + std::to_string(ccNumber) +
                        " was " + existingMapping->second.parameterID.toStdString() +
                        ", now " + parameterID.toStdString());
        }
    }

    // Odstraň předchozí mapping pro tento parametr (pokud existuje)
    removeMappingForParameter(parameterID);

    // Vytvoř nový mapping
    Mapping mapping;
    mapping.ccNumber = ccNumber;
    mapping.parameterID = parameterID;
    mapping.displayName = displayName;

    mappings_[ccNumber] = mapping;

    if (logger_) {
        logger_->log("MidiLearnManager/setMapping", LogSeverity::Info,
                    "Created mapping: CC " + std::to_string(ccNumber) +
                    " -> " + parameterID.toStdString() +
                    " (" + displayName.toStdString() + ")");
    }
}

void MidiLearnManager::removeMapping(uint8_t ccNumber)
{
    auto it = mappings_.find(ccNumber);
    if (it != mappings_.end()) {
        if (logger_) {
            logger_->log("MidiLearnManager/removeMapping", LogSeverity::Info,
                        "Removed mapping: CC " + std::to_string(ccNumber) +
                        " -> " + it->second.parameterID.toStdString());
        }
        mappings_.erase(it);
    }
}

void MidiLearnManager::removeMappingForParameter(const juce::String& parameterID)
{
    int removedCount = 0;
    for (auto it = mappings_.begin(); it != mappings_.end(); ) {
        if (it->second.parameterID == parameterID) {
            if (logger_) {
                logger_->log("MidiLearnManager/removeMappingForParameter", LogSeverity::Info,
                            "Removing mapping for parameter " + parameterID.toStdString() +
                            ": CC " + std::to_string(it->first));
            }
            it = mappings_.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }

    if (logger_ && removedCount == 0) {
        logger_->log("MidiLearnManager/removeMappingForParameter", LogSeverity::Debug,
                    "No existing mapping found for parameter: " + parameterID.toStdString());
    }
}

void MidiLearnManager::clearAllMappings()
{
    if (logger_) {
        logger_->log("MidiLearnManager/clearAllMappings", LogSeverity::Info,
                    "Clearing all " + std::to_string(mappings_.size()) + " MIDI Learn mappings");
    }
    mappings_.clear();
}

const MidiLearnManager::Mapping* MidiLearnManager::getMapping(uint8_t ccNumber) const
{
    auto it = mappings_.find(ccNumber);
    return (it != mappings_.end()) ? &it->second : nullptr;
}

int MidiLearnManager::getCCNumberForParameter(const juce::String& parameterID) const
{
    for (const auto& pair : mappings_) {
        if (pair.second.parameterID == parameterID) {
            return pair.first;
        }
    }
    return -1;
}

// ============================================================================
// Persistence
// ============================================================================

std::unique_ptr<juce::XmlElement> MidiLearnManager::saveToXml() const
{
    if (logger_) {
        logger_->log("MidiLearnManager/saveToXml", LogSeverity::Info,
                    "=== Starting MIDI Learn save ===");
        logger_->log("MidiLearnManager/saveToXml", LogSeverity::Info,
                    "Total mappings to save: " + std::to_string(mappings_.size()));
    }

    auto xml = std::make_unique<juce::XmlElement>("MidiLearnMappings");

    int savedCount = 0;
    for (const auto& pair : mappings_) {
        auto mappingXml = xml->createNewChildElement("Mapping");
        mappingXml->setAttribute("ccNumber", (int)pair.first);
        mappingXml->setAttribute("parameterID", pair.second.parameterID);
        mappingXml->setAttribute("displayName", pair.second.displayName);

        if (logger_) {
            logger_->log("MidiLearnManager/saveToXml", LogSeverity::Debug,
                        "  Saved mapping #" + std::to_string(savedCount + 1) +
                        ": CC " + std::to_string(pair.first) +
                        " -> " + pair.second.parameterID.toStdString() +
                        " (" + pair.second.displayName.toStdString() + ")");
        }
        savedCount++;
    }

    if (logger_) {
        logger_->log("MidiLearnManager/saveToXml", LogSeverity::Info,
                    "Successfully saved " + std::to_string(savedCount) + " mappings to XML");
        logger_->log("MidiLearnManager/saveToXml", LogSeverity::Info,
                    "=== MIDI Learn save complete ===");
    }

    return xml;
}

void MidiLearnManager::loadFromXml(const juce::XmlElement* xml)
{
    if (logger_) {
        logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Info,
                    "=== Starting MIDI Learn load ===");
    }

    if (!xml) {
        if (logger_) {
            logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Error,
                        "Load failed: XML element is null");
        }
        return;
    }

    if (!xml->hasTagName("MidiLearnMappings")) {
        if (logger_) {
            logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Error,
                        "Load failed: Unexpected tag name '" + xml->getTagName().toStdString() +
                        "' (expected 'MidiLearnMappings')");
        }
        return;
    }

    if (logger_) {
        logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Info,
                    "XML validation passed, clearing existing mappings");
    }

    clearAllMappings();

    int loadedCount = 0;
    int skippedCount = 0;

    for (auto* mappingXml : xml->getChildIterator()) {
        if (mappingXml->hasTagName("Mapping")) {
            uint8_t ccNumber = (uint8_t)mappingXml->getIntAttribute("ccNumber", 0);
            juce::String parameterID = mappingXml->getStringAttribute("parameterID");
            juce::String displayName = mappingXml->getStringAttribute("displayName");

            if (!parameterID.isEmpty()) {
                if (logger_) {
                    logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Debug,
                                "  Loading mapping #" + std::to_string(loadedCount + 1) +
                                ": CC " + std::to_string(ccNumber) +
                                " -> " + parameterID.toStdString() +
                                " (" + displayName.toStdString() + ")");
                }
                setMapping(ccNumber, parameterID, displayName);
                loadedCount++;
            } else {
                if (logger_) {
                    logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Warning,
                                "  Skipped mapping with empty parameterID (CC " +
                                std::to_string(ccNumber) + ")");
                }
                skippedCount++;
            }
        }
    }

    if (logger_) {
        logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Info,
                    "Successfully loaded " + std::to_string(loadedCount) + " mappings from XML");
        if (skippedCount > 0) {
            logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Warning,
                        "Skipped " + std::to_string(skippedCount) + " invalid mappings");
        }
        logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Info,
                    "=== MIDI Learn load complete ===");
    }
}

// ============================================================================
// Private Helper Methods
// ============================================================================

void MidiLearnManager::notifyLearningStateChanged()
{
    if (learningStateCallback_) {
        learningStateCallback_(isLearning_, learningParameterID_);
    }
}