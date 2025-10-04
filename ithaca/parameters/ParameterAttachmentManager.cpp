/**
 * @file ParameterAttachmentManager.cpp
 * @brief Implementace správy parameter attachments
 */

#include "ithaca/parameters/ParameterAttachmentManager.h"
#include <iostream>

// Makro pro debug výpisy (zachovat původní logiku)
#define BACKGROUND_PICTURE_OFF 0

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

bool ParameterAttachmentManager::createAllAttachments(
    juce::AudioProcessorValueTreeState& parameters, 
    const SliderSet& sliders)
{
    GUI_DEBUG("ParameterAttachmentManager: Creating all attachments - START");
    
    // Vymazat existující attachments
    clearAttachments();
    
    bool allSuccess = true;
    size_t successCount = 0;
    
    // ZACHOVAT: Původní parameter IDs a logiku
    
    // Master controls
    if (sliders.masterGain && createAttachment(parameters, "masterGain", sliders.masterGain)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: masterGain attachment created");
    } else {
        GUI_DEBUG("ParameterAttachmentManager: ERROR - masterGain attachment failed!");
        allSuccess = false;
    }
    
    if (sliders.masterPan && createAttachment(parameters, "masterPan", sliders.masterPan)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: masterPan attachment created");
    } else {
        GUI_DEBUG("ParameterAttachmentManager: ERROR - masterPan attachment failed!");
        allSuccess = false;
    }
    
    // ADSR Envelope
    if (sliders.attack && createAttachment(parameters, "attack", sliders.attack)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: attack attachment created");
    } else if (sliders.attack) {
        GUI_DEBUG("ParameterAttachmentManager: WARNING - attack parameter not found!");
    }
    
    if (sliders.release && createAttachment(parameters, "release", sliders.release)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: release attachment created");
    } else if (sliders.release) {
        GUI_DEBUG("ParameterAttachmentManager: WARNING - release parameter not found!");
    }
    
    if (sliders.sustainLevel && createAttachment(parameters, "sustainLevel", sliders.sustainLevel)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: sustainLevel attachment created");
    } else if (sliders.sustainLevel) {
        GUI_DEBUG("ParameterAttachmentManager: WARNING - sustainLevel parameter not found!");
    }
    
    // LFO Panning
    if (sliders.lfoPanSpeed && createAttachment(parameters, "lfoPanSpeed", sliders.lfoPanSpeed)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: lfoPanSpeed attachment created");
    } else if (sliders.lfoPanSpeed) {
        GUI_DEBUG("ParameterAttachmentManager: WARNING - lfoPanSpeed parameter not found!");
    }
    
    if (sliders.lfoPanDepth && createAttachment(parameters, "lfoPanDepth", sliders.lfoPanDepth)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: lfoPanDepth attachment created");
    } else if (sliders.lfoPanDepth) {
        GUI_DEBUG("ParameterAttachmentManager: WARNING - lfoPanDepth parameter not found!");
    }

    // Stereo Field
    if (sliders.stereoField && createAttachment(parameters, "stereoField", sliders.stereoField)) {
        successCount++;
        GUI_DEBUG("ParameterAttachmentManager: stereoField attachment created");
    } else if (sliders.stereoField) {
        GUI_DEBUG("ParameterAttachmentManager: WARNING - stereoField parameter not found!");
    }
    
    GUI_DEBUG("ParameterAttachmentManager: Attachments completed - " << successCount 
              << "/" << attachments_.size() << " attachments created successfully");
    
    return allSuccess;
}

bool ParameterAttachmentManager::createAttachment(
    juce::AudioProcessorValueTreeState& parameters,
    const juce::String& parameterID,
    juce::Slider* slider)
{
    if (!slider) {
        GUI_DEBUG("ParameterAttachmentManager: ERROR - null slider for parameter: " << parameterID.toStdString());
        return false;
    }
    
    if (!parameterExists(parameters, parameterID)) {
        GUI_DEBUG("ParameterAttachmentManager: ERROR - parameter not found: " << parameterID.toStdString());
        return false;
    }
    
    try {
        auto attachment = std::make_unique<SliderAttachment>(parameters, parameterID, *slider);
        attachments_.push_back(std::move(attachment));
        return true;
    } catch (const std::exception&) {
        GUI_DEBUG("ParameterAttachmentManager: EXCEPTION creating attachment for " 
                  << parameterID.toStdString());
        return false;
    } catch (...) {
        GUI_DEBUG("ParameterAttachmentManager: UNKNOWN EXCEPTION creating attachment for " 
                  << parameterID.toStdString());
        return false;
    }
}

bool ParameterAttachmentManager::parameterExists(
    juce::AudioProcessorValueTreeState& parameters,
    const juce::String& parameterID) const
{
    return parameters.getParameter(parameterID) != nullptr;
}

void ParameterAttachmentManager::logAttachmentStatus() const
{
    GUI_DEBUG("ParameterAttachmentManager: Status - " << attachments_.size() << " attachments active");
    
    for (size_t i = 0; i < attachments_.size(); ++i) {
        if (attachments_[i]) {
            GUI_DEBUG("  Attachment " << i << ": OK");
        } else {
            GUI_DEBUG("  Attachment " << i << ": NULL");
        }
    }
}