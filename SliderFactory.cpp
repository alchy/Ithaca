/**
 * @file SliderFactory.cpp
 * @brief Implementace Slider Factory (DRY princip pro vytváření sliderů)
 */

#include "SliderFactory.h"
#include "GuiHelpers.h"
#include "GuiConstants.h"
#include "ParameterDefaults.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// ============================================================================
// Factory Methods
// ============================================================================

SliderContainer SliderFactory::createSlider(
    const SliderConfig& config,
    bool debugMode,
    juce::Slider::Listener* listener,
    RightClickCallback rightClickCallback)
{
    SliderContainer container;
    container.config = config;

    // 1. Vytvoř label
    container.label = GuiHelpers::createSliderLabel(config.labelText, debugMode);

    // 2. Vytvoř slider
    container.slider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal,
        juce::Slider::NoTextBox);

    if (container.slider) {
        // 3. Nastav rozsah a hodnoty
        container.slider->setRange(config.minValue, config.maxValue, config.interval);
        container.slider->setValue(config.defaultValue);

        // 4. Aplikuj styling
        GuiHelpers::styleSlider(*container.slider, debugMode);

        // 5. Přidej listener
        if (listener) {
            container.slider->addListener(listener);
        }

        // 6. Nastav right-click callback
        if (rightClickCallback) {
            container.slider->setRightClickCallback([rightClickCallback, slider = container.slider.get()](juce::Point<int> pos) {
                rightClickCallback(slider, pos);
            });
        }

        GUI_DEBUG("SliderFactory: Created slider '" << config.parameterID.toStdString()
                  << "' (range: " << config.minValue << "-" << config.maxValue
                  << ", default: " << config.defaultValue << ")");
    }

    return container;
}

std::vector<SliderContainer> SliderFactory::createAllSliders(
    const std::vector<SliderConfig>& configs,
    bool debugMode,
    juce::Slider::Listener* listener,
    RightClickCallback rightClickCallback)
{
    std::vector<SliderContainer> containers;
    containers.reserve(configs.size());

    GUI_DEBUG("SliderFactory: Creating " << configs.size() << " sliders...");

    for (const auto& config : configs) {
        containers.push_back(createSlider(config, debugMode, listener, rightClickCallback));
    }

    GUI_DEBUG("SliderFactory: All sliders created successfully");

    return containers;
}

// ============================================================================
// Mapping Helpers
// ============================================================================

void SliderFactory::buildMappings(
    const std::vector<SliderContainer>& containers,
    std::unordered_map<juce::Slider*, juce::String>& sliderToID,
    std::unordered_map<juce::Slider*, juce::String>& sliderToDisplayName,
    std::unordered_map<juce::String, juce::Slider*>& idToSlider)
{
    sliderToID.clear();
    sliderToDisplayName.clear();
    idToSlider.clear();

    for (const auto& container : containers) {
        if (container.slider) {
            auto* sliderPtr = container.slider.get();
            sliderToID[sliderPtr] = container.config.parameterID;
            sliderToDisplayName[sliderPtr] = container.config.displayName;
            idToSlider[container.config.parameterID] = sliderPtr;
        }
    }

    GUI_DEBUG("SliderFactory: Built mappings for " << containers.size() << " sliders");
}

// ============================================================================
// Predefined Configurations (Ithaca specific)
// ============================================================================

std::vector<SliderConfig> SliderFactory::getIthacaSliderConfigs()
{
    return {
        // Master Controls
        {
            PARAM_ID_MASTER_GAIN,
            PARAM_NAME_MASTER_GAIN,
            GuiConstants::TextConstants::MASTER_GAIN_LABEL,
            static_cast<float>(DEFAULT_MASTER_GAIN)
        },
        {
            PARAM_ID_STEREO_FIELD,
            PARAM_NAME_STEREO_FIELD,
            GuiConstants::TextConstants::STEREO_FIELD_LABEL,
            static_cast<float>(DEFAULT_STEREO_FIELD)
        },

        // LFO Controls
        {
            PARAM_ID_LFO_PAN_DEPTH,
            PARAM_NAME_LFO_PAN_DEPTH,
            GuiConstants::TextConstants::LFO_DEPTH_LABEL,
            static_cast<float>(DEFAULT_LFO_PAN_DEPTH)
        },
        {
            PARAM_ID_LFO_PAN_SPEED,
            PARAM_NAME_LFO_PAN_SPEED,
            GuiConstants::TextConstants::LFO_SPEED_LABEL,
            static_cast<float>(DEFAULT_LFO_PAN_SPEED)
        },

        // ADSR Controls
        {
            PARAM_ID_ATTACK,
            PARAM_NAME_ATTACK,
            GuiConstants::TextConstants::ATTACK_LABEL,
            static_cast<float>(DEFAULT_ATTACK)
        },
        {
            PARAM_ID_RELEASE,
            PARAM_NAME_RELEASE,
            GuiConstants::TextConstants::RELEASE_LABEL,
            static_cast<float>(DEFAULT_RELEASE)
        },
        {
            PARAM_ID_SUSTAIN_LEVEL,
            PARAM_NAME_SUSTAIN_LEVEL,
            GuiConstants::TextConstants::SUSTAIN_LABEL,
            static_cast<float>(DEFAULT_SUSTAIN_LEVEL)
        },

        // Pan Control
        {
            PARAM_ID_MASTER_PAN,
            PARAM_NAME_MASTER_PAN,
            GuiConstants::TextConstants::MASTER_PAN_LABEL,
            static_cast<float>(DEFAULT_MASTER_PAN)
        },

        // BBE Effect Controls
        {
            PARAM_ID_BBE_DEFINITION,
            PARAM_NAME_BBE_DEFINITION,
            GuiConstants::TextConstants::BBE_DEFINITION_LABEL,
            static_cast<float>(DEFAULT_BBE_DEFINITION)
        },
        {
            PARAM_ID_BBE_BASS_BOOST,
            PARAM_NAME_BBE_BASS_BOOST,
            GuiConstants::TextConstants::BBE_BASS_BOOST_LABEL,
            static_cast<float>(DEFAULT_BBE_BASS_BOOST)
        }
    };
}
