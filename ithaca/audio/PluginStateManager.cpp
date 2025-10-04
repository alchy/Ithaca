/**
 * @file PluginStateManager.cpp
 * @brief Implementation of plugin state management
 */

#include "ithaca/audio/PluginStateManager.h"
#include "ithaca/midi/MidiLearnManager.h"

//==============================================================================
// Public Interface - Save

void PluginStateManager::saveState(juce::MemoryBlock& destData,
                                   juce::AudioProcessorValueTreeState& parameters,
                                   MidiLearnManager* midiLearnManager,
                                   LogCallback logCallback)
{
    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "=== PLUGIN STATE SAVE STARTED ===");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
    }

    // Create root XML with all state data
    auto rootXml = createStateXml(parameters, midiLearnManager);

    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Info,
                   "Created XML structure with root tag: " + rootXml->getTagName().toStdString());
        logCallback("PluginStateManager", LogSeverity::Info,
                   "XML child count: " + std::to_string(rootXml->getNumChildElements()));
    }

    // Convert to binary
    juce::AudioProcessor::copyXmlToBinary(*rootXml, destData);

    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Info,
                   "Binary data size: " + std::to_string(destData.getSize()) + " bytes");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "=== PLUGIN STATE SAVE COMPLETE ===");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
    }
}

//==============================================================================
// Public Interface - Load

bool PluginStateManager::loadState(const void* data,
                                   int sizeInBytes,
                                   juce::AudioProcessorValueTreeState& parameters,
                                   MidiLearnManager* midiLearnManager,
                                   LogCallback logCallback)
{
    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "=== PLUGIN STATE LOAD STARTED ===");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
        logCallback("PluginStateManager", LogSeverity::Info,
                   "Binary data size: " + std::to_string(sizeInBytes) + " bytes");
    }

    // Parse binary data to XML
    std::unique_ptr<juce::XmlElement> xmlState(
        juce::AudioProcessor::getXmlFromBinary(data, sizeInBytes));

    if (!xmlState) {
        if (logCallback) {
            logCallback("PluginStateManager", LogSeverity::Error,
                       "Failed to parse binary data to XML");
            logCallback("PluginStateManager", LogSeverity::Info,
                       "========================================");
            logCallback("PluginStateManager", LogSeverity::Info,
                       "=== PLUGIN STATE LOAD FAILED ===");
            logCallback("PluginStateManager", LogSeverity::Info,
                       "========================================");
        }
        return false;
    }

    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Info,
                   "Successfully parsed XML, root tag: " + xmlState->getTagName().toStdString());
        logCallback("PluginStateManager", LogSeverity::Info,
                   "XML child count: " + std::to_string(xmlState->getNumChildElements()));
    }

    // Restore from XML
    bool success = restoreFromXml(xmlState.get(), parameters, midiLearnManager, logCallback);

    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
        if (success) {
            logCallback("PluginStateManager", LogSeverity::Info,
                       "=== PLUGIN STATE LOAD COMPLETE ===");
        } else {
            logCallback("PluginStateManager", LogSeverity::Error,
                       "=== PLUGIN STATE LOAD FAILED ===");
        }
        logCallback("PluginStateManager", LogSeverity::Info,
                   "========================================");
    }

    return success;
}

//==============================================================================
// Private Helpers - Save

std::unique_ptr<juce::XmlElement> PluginStateManager::createStateXml(
    juce::AudioProcessorValueTreeState& parameters,
    MidiLearnManager* midiLearnManager)
{
    // Create root XML element
    auto rootXml = std::make_unique<juce::XmlElement>(ROOT_TAG);

    // 1. Save parameter state
    auto parameterState = parameters.copyState();
    auto parameterXml = parameterState.createXml();
    if (parameterXml) {
        rootXml->addChildElement(parameterXml.release());
    }

    // 2. Save MIDI Learn mappings (if available)
    if (midiLearnManager) {
        auto midiLearnXml = midiLearnManager->saveToXml();
        if (midiLearnXml) {
            rootXml->addChildElement(midiLearnXml.release());
        } else {
            // This should never happen, but log it if it does
            juce::Logger::writeToLog("[PluginStateManager] Warning: MidiLearnManager returned null XML");
        }
    }

    return rootXml;
}

//==============================================================================
// Private Helpers - Load

bool PluginStateManager::restoreFromXml(juce::XmlElement* xmlState,
                                        juce::AudioProcessorValueTreeState& parameters,
                                        MidiLearnManager* midiLearnManager,
                                        LogCallback logCallback)
{
    if (!xmlState) {
        if (logCallback) {
            logCallback("PluginStateManager", LogSeverity::Error, "restoreFromXml: xmlState is null");
        }
        return false;
    }

    // Check format: new (with MIDI Learn) or legacy (parameters only)
    if (isNewFormat(xmlState)) {
        if (logCallback) {
            logCallback("PluginStateManager", LogSeverity::Info,
                       "Detected new format (IthacaPluginState with MIDI Learn support)");
        }

        // 1. Restore parameter state
        auto* parameterXml = xmlState->getChildByName(parameters.state.getType());
        if (parameterXml) {
            if (logCallback) {
                logCallback("PluginStateManager", LogSeverity::Info,
                           "Found parameter child element: " + parameters.state.getType().toString().toStdString());
            }
            parameters.replaceState(juce::ValueTree::fromXml(*parameterXml));
            if (logCallback) {
                logCallback("PluginStateManager", LogSeverity::Info, "Parameters restored successfully");
            }
        } else {
            if (logCallback) {
                logCallback("PluginStateManager", LogSeverity::Warning,
                           "Parameter child element not found (expected tag: " +
                           parameters.state.getType().toString().toStdString() + ")");
            }
        }

        // 2. Restore MIDI Learn mappings
        auto* midiLearnXml = xmlState->getChildByName(MIDI_LEARN_TAG);
        if (midiLearnXml) {
            if (logCallback) {
                logCallback("PluginStateManager", LogSeverity::Info,
                           "Found MIDI Learn child element with " +
                           std::to_string(midiLearnXml->getNumChildElements()) + " mappings");
            }
            if (midiLearnManager) {
                midiLearnManager->loadFromXml(midiLearnXml);
                if (logCallback) {
                    logCallback("PluginStateManager", LogSeverity::Info, "MIDI Learn mappings restored successfully");
                }
            } else {
                if (logCallback) {
                    logCallback("PluginStateManager", LogSeverity::Warning,
                               "MIDI Learn data found but MidiLearnManager is null");
                }
            }
        } else {
            if (logCallback) {
                logCallback("PluginStateManager", LogSeverity::Info,
                           "No MIDI Learn child element found (this is normal for presets without MIDI Learn)");
            }
        }

        return true;
    }
    else if (isLegacyFormat(xmlState, parameters)) {
        if (logCallback) {
            logCallback("PluginStateManager", LogSeverity::Info,
                       "Detected legacy format (parameters only, no MIDI Learn)");
        }
        // Legacy format (parameters only, no MIDI Learn)
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        if (logCallback) {
            logCallback("PluginStateManager", LogSeverity::Info,
                       "Legacy state restored successfully");
        }
        return true;
    }

    // Unknown format
    if (logCallback) {
        logCallback("PluginStateManager", LogSeverity::Error,
                   "Unknown state format - tag name: " + xmlState->getTagName().toStdString());
    }
    return false;
}

//==============================================================================
// Private Helpers - Format Detection

bool PluginStateManager::isNewFormat(const juce::XmlElement* xmlState)
{
    return xmlState && xmlState->hasTagName(ROOT_TAG);
}

bool PluginStateManager::isLegacyFormat(const juce::XmlElement* xmlState,
                                       const juce::AudioProcessorValueTreeState& parameters)
{
    return xmlState && xmlState->hasTagName(parameters.state.getType());
}
