/**
 * @file SliderFactory.h
 * @brief Factory pro vytváření sliderů s MIDI Learn funkčností (DRY princip)
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "MidiLearnSlider.h"
#include "MidiLearnManager.h"
#include <vector>
#include <unordered_map>
#include <functional>

// ============================================================================
// SLIDER CONFIGURATION STRUCTURE
// ============================================================================

struct SliderConfig {
    juce::String parameterID;      // ID pro APVTS (např. "masterGain")
    juce::String displayName;      // Zobrazované jméno (např. "Master Gain")
    juce::String labelText;        // Text labelu (může být stejný jako displayName)
    float defaultValue;            // Výchozí hodnota (0-127)
    float minValue = 0.0f;         // Minimální hodnota
    float maxValue = 127.0f;       // Maximální hodnota
    float interval = 1.0f;         // Krok změny
};

// ============================================================================
// SLIDER CONTAINER STRUCTURE
// ============================================================================

struct SliderContainer {
    std::unique_ptr<MidiLearnSlider> slider;
    std::unique_ptr<juce::Label> label;
    SliderConfig config;

    SliderContainer() = default;
    SliderContainer(SliderContainer&&) = default;
    SliderContainer& operator=(SliderContainer&&) = default;

    // Disable copying
    SliderContainer(const SliderContainer&) = delete;
    SliderContainer& operator=(const SliderContainer&) = delete;
};

// ============================================================================
// SLIDER FACTORY CLASS
// ============================================================================

class SliderFactory {
public:
    using RightClickCallback = std::function<void(juce::Slider*, juce::Point<int>)>;

    SliderFactory() = default;
    ~SliderFactory() = default;

    // ========================================================================
    // Factory Methods
    // ========================================================================

    /**
     * Vytvoří slider s labelem podle konfigurace
     */
    SliderContainer createSlider(
        const SliderConfig& config,
        bool debugMode,
        juce::Slider::Listener* listener = nullptr,
        RightClickCallback rightClickCallback = nullptr);

    /**
     * Vytvoří všechny slidery najednou podle pole konfigurací
     */
    std::vector<SliderContainer> createAllSliders(
        const std::vector<SliderConfig>& configs,
        bool debugMode,
        juce::Slider::Listener* listener = nullptr,
        RightClickCallback rightClickCallback = nullptr);

    // ========================================================================
    // Mapping Helpers
    // ========================================================================

    /**
     * Vytvoří mapy pro rychlý lookup mezi slider <-> parameterID
     */
    static void buildMappings(
        const std::vector<SliderContainer>& containers,
        std::unordered_map<juce::Slider*, juce::String>& sliderToID,
        std::unordered_map<juce::Slider*, juce::String>& sliderToDisplayName,
        std::unordered_map<juce::String, juce::Slider*>& idToSlider);

    // ========================================================================
    // Predefined Configurations (Ithaca specific)
    // ========================================================================

    /**
     * Vrátí standardní konfigurace pro Ithaca plugin
     */
    static std::vector<SliderConfig> getIthacaSliderConfigs();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderFactory)
};
