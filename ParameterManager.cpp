#include "ParameterManager.h"
#include "core_logger.h"

//==============================================================================
ParameterManager::ParameterManager()
    : parameters_(*static_cast<juce::AudioProcessor*>(nullptr), nullptr, juce::Identifier("IthacaParameters"), createParameterLayout()),
      masterGainParam_(nullptr),
      masterPanParam_(nullptr)
{
    // Create temporary logger for initialization
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/constructor", "info", 
               "=== PARAMETER MANAGER INITIALIZING ===");
    logger->log("ParameterManager/constructor", "info", 
               "Creating JUCE ValueTreeState with parameter layout...");
    
    // Initialize parameter pointers after layout is created
    logger->log("ParameterManager/constructor", "info", 
               "Initializing atomic parameter pointers...");
    initializeParameterPointers();
    
    // Validate parameter initialization
    if (isGainParameterValid() && isPanParameterValid()) {
        logger->log("ParameterManager/constructor", "info", 
                   "All parameters initialized successfully");
        logger->log("ParameterManager/constructor", "info", 
                   "Master Gain range: " + std::to_string(GAIN_MIN) + " to " + 
                   std::to_string(GAIN_MAX) + " (default: " + std::to_string(GAIN_DEFAULT) + ")");
        logger->log("ParameterManager/constructor", "info", 
                   "Master Pan range: " + std::to_string(PAN_MIN) + " to " + 
                   std::to_string(PAN_MAX) + " (default: " + std::to_string(PAN_DEFAULT) + ")");
    } else {
        logger->log("ParameterManager/constructor", "error", 
                   "Parameter initialization failed - some atomic pointers are null");
    }
    
    logger->log("ParameterManager/constructor", "info", 
               "=== PARAMETER MANAGER READY ===");
}

//==============================================================================
// RT-Safe Parameter Value Access

uint8_t ParameterManager::getMasterGainMIDI() const noexcept
{
    if (!masterGainParam_) return static_cast<uint8_t>(GAIN_DEFAULT);
    return clampMIDIValue(masterGainParam_->load());
}

float ParameterManager::getMasterGainNormalized() const noexcept
{
    return getMasterGainMIDI() / 127.0f;
}

uint8_t ParameterManager::getMasterPanMIDI() const noexcept
{
    if (!masterPanParam_) return 64; // Center in MIDI range
    float internalPan = masterPanParam_->load();
    return panToMIDI(internalPan);
}

float ParameterManager::getMasterPanNormalized() const noexcept
{
    if (!masterPanParam_) return 0.0f; // Center
    float internalPan = clampPanValue(masterPanParam_->load());
    return internalPan / 63.0f; // Normalize to -1.0 to 1.0 range
}

//==============================================================================
// State Management

void ParameterManager::saveState(juce::MemoryBlock& destData)
{
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/saveState", "info", 
               "Saving parameter state...");
    
    try {
        auto state = parameters_.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        
        if (xml) {
            juce::MemoryOutputStream stream(destData, true);
            xml->writeTo(stream, juce::XmlElement::TextFormat().singleLine());
            
            logger->log("ParameterManager/saveState", "info", 
                       "Parameter state saved successfully (" + 
                       std::to_string(destData.getSize()) + " bytes)");
            
            // Log current parameter values for debugging
            logger->log("ParameterManager/saveState", "info", 
                       "Saved Master Gain: " + std::to_string(getMasterGainMIDI()) + 
                       " (" + std::to_string(getMasterGainNormalized()) + " normalized)");
            logger->log("ParameterManager/saveState", "info", 
                       "Saved Master Pan: " + std::to_string(getMasterPanMIDI()) + 
                       " (" + std::to_string(getMasterPanNormalized()) + " normalized)");
        } else {
            logger->log("ParameterManager/saveState", "error", 
                       "Failed to create XML from parameter state");
        }
        
    } catch (const std::exception& e) {
        logger->log("ParameterManager/saveState", "error", 
                   "Exception during state save: " + std::string(e.what()));
    } catch (...) {
        logger->log("ParameterManager/saveState", "error", 
                   "Unknown exception during state save");
    }
}

void ParameterManager::loadState(const void* data, int sizeInBytes)
{
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/loadState", "info", 
               "Loading parameter state (" + std::to_string(sizeInBytes) + " bytes)...");
    
    if (!data || sizeInBytes <= 0) {
        logger->log("ParameterManager/loadState", "error", 
                   "Invalid input data - null pointer or zero size");
        return;
    }
    
    try {
        juce::String xmlText = juce::String::createStringFromData(data, sizeInBytes);
        juce::XmlDocument xmlDoc(xmlText);
        std::unique_ptr<juce::XmlElement> xmlState(xmlDoc.getDocumentElement());
        
        if (xmlState.get() != nullptr) {
            if (xmlState->hasTagName(parameters_.state.getType())) {
                parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
                logger->log("ParameterManager/loadState", "info", 
                           "Parameter state loaded successfully");
                
                // Log loaded parameter values for debugging
                logger->log("ParameterManager/loadState", "info", 
                           "Loaded Master Gain: " + std::to_string(getMasterGainMIDI()) + 
                           " (" + std::to_string(getMasterGainNormalized()) + " normalized)");
                logger->log("ParameterManager/loadState", "info", 
                           "Loaded Master Pan: " + std::to_string(getMasterPanMIDI()) + 
                           " (" + std::to_string(getMasterPanNormalized()) + " normalized)");
            } else {
                logger->log("ParameterManager/loadState", "error", 
                           "XML tag name mismatch - expected '" + 
                           parameters_.state.getType().toString().toStdString() + 
                           "', got '" + xmlState->getTagName().toStdString() + "'");
            }
        } else {
            logger->log("ParameterManager/loadState", "error", 
                       "Failed to parse XML from binary data - invalid XML format");
            
            // Log first few characters for debugging
            if (sizeInBytes > 0) {
                juce::String preview = juce::String::createStringFromData(data, std::min(100, sizeInBytes));
                logger->log("ParameterManager/loadState", "info", 
                           "Data preview: " + preview.toStdString());
            }
        }
        
    } catch (const std::exception& e) {
        logger->log("ParameterManager/loadState", "error", 
                   "Exception during state load: " + std::string(e.what()));
    } catch (...) {
        logger->log("ParameterManager/loadState", "error", 
                   "Unknown exception during state load");
    }
}

void ParameterManager::resetToDefaults()
{
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/resetToDefaults", "info", 
               "Resetting all parameters to default values...");
    
    try {
        // Reset Master Gain to default
        if (auto* gainParam = parameters_.getParameter(MASTER_GAIN_ID)) {
            float defaultValue = gainParam->getDefaultValue();
            gainParam->setValueNotifyingHost(defaultValue);
            logger->log("ParameterManager/resetToDefaults", "info", 
                       "Master Gain reset to default: " + std::to_string(defaultValue));
        } else {
            logger->log("ParameterManager/resetToDefaults", "error", 
                       "Master Gain parameter not found for reset");
        }
        
        // Reset Master Pan to default
        if (auto* panParam = parameters_.getParameter(MASTER_PAN_ID)) {
            float defaultValue = panParam->getDefaultValue();
            panParam->setValueNotifyingHost(defaultValue);
            logger->log("ParameterManager/resetToDefaults", "info", 
                       "Master Pan reset to default: " + std::to_string(defaultValue));
        } else {
            logger->log("ParameterManager/resetToDefaults", "error", 
                       "Master Pan parameter not found for reset");
        }
        
        logger->log("ParameterManager/resetToDefaults", "info", 
                   "Parameters reset completed successfully");
        logger->log("ParameterManager/resetToDefaults", "info", 
                   "Current Master Gain: " + std::to_string(getMasterGainMIDI()));
        logger->log("ParameterManager/resetToDefaults", "info", 
                   "Current Master Pan: " + std::to_string(getMasterPanMIDI()));
        
    } catch (const std::exception& e) {
        logger->log("ParameterManager/resetToDefaults", "error", 
                   "Exception during parameter reset: " + std::string(e.what()));
    } catch (...) {
        logger->log("ParameterManager/resetToDefaults", "error", 
                   "Unknown exception during parameter reset");
    }
}

//==============================================================================
// Parameter Change Notifications

void ParameterManager::setParameterChangeCallback(std::function<void(const juce::String&)> callback)
{
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/setParameterChangeCallback", "info", 
               "Setting parameter change callback");
    
    parameterChangeCallback_ = std::move(callback);
    
    if (parameterChangeCallback_) {
        logger->log("ParameterManager/setParameterChangeCallback", "info", 
                   "Parameter change callback set successfully");
    } else {
        logger->log("ParameterManager/setParameterChangeCallback", "warn", 
                   "Parameter change callback is null - no notifications will be sent");
    }
}

//==============================================================================
// Private Methods

juce::AudioProcessorValueTreeState::ParameterLayout ParameterManager::createParameterLayout()
{
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/createParameterLayout", "info", 
               "Creating parameter layout...");
    
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Master Gain Parameter (0-127 MIDI range)
    logger->log("ParameterManager/createParameterLayout", "info", 
               "Adding Master Gain parameter (0-127 MIDI range)");
    try {
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(MASTER_GAIN_ID, 1),
            "Master Gain",
            juce::NormalisableRange<float>(GAIN_MIN, GAIN_MAX, GAIN_STEP),
            GAIN_DEFAULT,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction([](float value, int) { 
                    return juce::String(static_cast<int>(value)); 
                })
                .withLabel("MIDI")
        ));
        logger->log("ParameterManager/createParameterLayout", "info", 
                   "Master Gain parameter added successfully");
    } catch (const std::exception& e) {
        logger->log("ParameterManager/createParameterLayout", "error", 
                   "Failed to create Master Gain parameter: " + std::string(e.what()));
    }
    
    // Master Pan Parameter (-64 to +63, center=0)
    logger->log("ParameterManager/createParameterLayout", "info", 
               "Adding Master Pan parameter (-64 to +63, center=0)");
    try {
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(MASTER_PAN_ID, 1),
            "Master Pan",
            juce::NormalisableRange<float>(PAN_MIN, PAN_MAX, PAN_STEP),
            PAN_DEFAULT,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction([](float value, int) { 
                    if (value < 0) return "L" + juce::String(static_cast<int>(-value));
                    else if (value > 0) return "R" + juce::String(static_cast<int>(value));
                    else return juce::String("Center");
                })
                .withLabel("Pan")
        ));
        logger->log("ParameterManager/createParameterLayout", "info", 
                   "Master Pan parameter added successfully");
    } catch (const std::exception& e) {
        logger->log("ParameterManager/createParameterLayout", "error", 
                   "Failed to create Master Pan parameter: " + std::string(e.what()));
    }
    
    logger->log("ParameterManager/createParameterLayout", "info", 
               "Parameter layout created with " + std::to_string(parameters.size()) + " parameters");
    
    if (parameters.empty()) {
        logger->log("ParameterManager/createParameterLayout", "error", 
                   "No parameters were successfully created - this will cause initialization failure");
    }
    
    return { parameters.begin(), parameters.end() };
}

void ParameterManager::initializeParameterPointers()
{
    auto logger = std::make_unique<Logger>(".");
    logger->log("ParameterManager/initializeParameterPointers", "info", 
               "Initializing atomic parameter pointers...");
    
    // Initialize Master Gain parameter pointer
    masterGainParam_ = parameters_.getRawParameterValue(MASTER_GAIN_ID);
    if (masterGainParam_) {
        logger->log("ParameterManager/initializeParameterPointers", "info", 
                   "Master Gain parameter pointer initialized successfully");
        logger->log("ParameterManager/initializeParameterPointers", "info", 
                   "Initial Master Gain value: " + std::to_string(masterGainParam_->load()));
    } else {
        logger->log("ParameterManager/initializeParameterPointers", "error", 
                   "Failed to initialize Master Gain parameter pointer - parameter not found");
    }
    
    // Initialize Master Pan parameter pointer
    masterPanParam_ = parameters_.getRawParameterValue(MASTER_PAN_ID);
    if (masterPanParam_) {
        logger->log("ParameterManager/initializeParameterPointers", "info", 
                   "Master Pan parameter pointer initialized successfully");
        logger->log("ParameterManager/initializeParameterPointers", "info", 
                   "Initial Master Pan value: " + std::to_string(masterPanParam_->load()));
    } else {
        logger->log("ParameterManager/initializeParameterPointers", "error", 
                   "Failed to initialize Master Pan parameter pointer - parameter not found");
    }
    
    // Summary validation
    int validPointers = (masterGainParam_ ? 1 : 0) + (masterPanParam_ ? 1 : 0);
    logger->log("ParameterManager/initializeParameterPointers", "info", 
               "Parameter pointer initialization completed: " + std::to_string(validPointers) + "/2 valid");
    
    if (validPointers == 2) {
        logger->log("ParameterManager/initializeParameterPointers", "info", 
                   "All parameter pointers initialized successfully");
    } else {
        logger->log("ParameterManager/initializeParameterPointers", "error", 
                   "Some parameter pointers failed to initialize - functionality may be limited");
    }
}

//==============================================================================
// Private Utility Methods

uint8_t ParameterManager::clampMIDIValue(float value) noexcept
{
    if (value < 0.0f) return 0;
    if (value > 127.0f) return 127;
    return static_cast<uint8_t>(value);
}

float ParameterManager::clampPanValue(float value) noexcept
{
    if (value < PAN_MIN) return PAN_MIN;
    if (value > PAN_MAX) return PAN_MAX;
    return value;
}

uint8_t ParameterManager::panToMIDI(float internalPan) noexcept
{
    // Convert internal pan (-64 to +63) to MIDI pan (0-127, center=64)
    float clampedPan = clampPanValue(internalPan);
    float midiPan = clampedPan + 64.0f;
    return clampMIDIValue(midiPan);
}