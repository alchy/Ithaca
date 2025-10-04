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
    // Create root XML with all state data
    auto rootXml = createStateXml(parameters, midiLearnManager);

    // Convert to binary
    juce::AudioProcessor::copyXmlToBinary(*rootXml, destData);

    // Log success
    if (logCallback) {
        logCallback("PluginStateManager", "info",
                   "Plugin state saved (including MIDI Learn mappings)");
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
    // Parse binary data to XML
    std::unique_ptr<juce::XmlElement> xmlState(
        juce::AudioProcessor::getXmlFromBinary(data, sizeInBytes));

    if (!xmlState) {
        if (logCallback) {
            logCallback("PluginStateManager", "error",
                       "Failed to parse state data");
        }
        return false;
    }

    // Restore from XML
    return restoreFromXml(xmlState.get(), parameters, midiLearnManager, logCallback);
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
        return false;
    }

    // Check format: new (with MIDI Learn) or legacy (parameters only)
    if (isNewFormat(xmlState)) {
        // New format with MIDI Learn
        // 1. Restore parameter state
        auto* parameterXml = xmlState->getChildByName(parameters.state.getType());
        if (parameterXml) {
            parameters.replaceState(juce::ValueTree::fromXml(*parameterXml));
            if (logCallback) {
                logCallback("PluginStateManager", "info", "Parameters restored");
            }
        }

        // 2. Restore MIDI Learn mappings
        auto* midiLearnXml = xmlState->getChildByName(MIDI_LEARN_TAG);
        if (midiLearnXml && midiLearnManager) {
            midiLearnManager->loadFromXml(midiLearnXml);
            if (logCallback) {
                logCallback("PluginStateManager", "info", "MIDI Learn mappings restored");
            }
        }

        return true;
    }
    else if (isLegacyFormat(xmlState, parameters)) {
        // Legacy format (parameters only, no MIDI Learn)
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        if (logCallback) {
            logCallback("PluginStateManager", "info",
                       "Legacy state restored (no MIDI Learn data)");
        }
        return true;
    }

    // Unknown format
    if (logCallback) {
        logCallback("PluginStateManager", "error", "Unknown state format");
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
