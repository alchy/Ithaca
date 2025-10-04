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

    notifyLearningStateChanged();
}

void MidiLearnManager::cancelLearning()
{
    if (!isLearning_) return;

    isLearning_ = false;
    learningParameterID_.clear();
    learningDisplayName_.clear();

    notifyLearningStateChanged();
}

bool MidiLearnManager::tryLearnCC(uint8_t ccNumber)
{
    if (!isLearning_) return false;

    // Ignoruj CC 120-127 (reserved pro channel messages)
    if (ccNumber >= 120) {
        return false;
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
    // Odstraň předchozí mapping pro tento parametr (pokud existuje)
    removeMappingForParameter(parameterID);

    // Vytvoř nový mapping
    Mapping mapping;
    mapping.ccNumber = ccNumber;
    mapping.parameterID = parameterID;
    mapping.displayName = displayName;

    mappings_[ccNumber] = mapping;
}

void MidiLearnManager::removeMapping(uint8_t ccNumber)
{
    auto it = mappings_.find(ccNumber);
    if (it != mappings_.end()) {
        mappings_.erase(it);
    }
}

void MidiLearnManager::removeMappingForParameter(const juce::String& parameterID)
{
    for (auto it = mappings_.begin(); it != mappings_.end(); ) {
        if (it->second.parameterID == parameterID) {
            it = mappings_.erase(it);
        } else {
            ++it;
        }
    }
}

void MidiLearnManager::clearAllMappings()
{
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
    auto xml = std::make_unique<juce::XmlElement>("MidiLearnMappings");

    for (const auto& pair : mappings_) {
        auto mappingXml = xml->createNewChildElement("Mapping");
        mappingXml->setAttribute("ccNumber", (int)pair.first);
        mappingXml->setAttribute("parameterID", pair.second.parameterID);
        mappingXml->setAttribute("displayName", pair.second.displayName);
    }

    if (logger_) {
        logger_->log("MidiLearnManager/saveToXml", LogSeverity::Info,
                    "Saved " + std::to_string(mappings_.size()) + " mappings to XML");
    }

    return xml;
}

void MidiLearnManager::loadFromXml(const juce::XmlElement* xml)
{
    if (!xml || !xml->hasTagName("MidiLearnMappings")) return;

    clearAllMappings();

    for (auto* mappingXml : xml->getChildIterator()) {
        if (mappingXml->hasTagName("Mapping")) {
            uint8_t ccNumber = (uint8_t)mappingXml->getIntAttribute("ccNumber", 0);
            juce::String parameterID = mappingXml->getStringAttribute("parameterID");
            juce::String displayName = mappingXml->getStringAttribute("displayName");

            if (!parameterID.isEmpty()) {
                setMapping(ccNumber, parameterID, displayName);
            }
        }
    }

    if (logger_) {
        logger_->log("MidiLearnManager/loadFromXml", LogSeverity::Info,
                    "Loaded " + std::to_string(mappings_.size()) + " mappings from XML");
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