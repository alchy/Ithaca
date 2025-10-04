/**
 * @file SliderFactory.h
 * @brief Factory pro vytváření sliderů s DRY principem
 *
 * Zjednodušuje vytváření sliderů s konzistentním patternem:
 * - Vytvoření slideru a labelu
 * - Stylování podle debug mode
 * - Propojení s MIDI Learn
 * - Attachment na parametry
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ithaca/gui/components/MidiLearnSlider.h"
#include "ithaca/midi/MidiLearnManager.h"
#include "ithaca/gui/helpers/GuiHelpers.h"
#include <memory>
#include <functional>

/**
 * @brief Konfigurace pro vytvoření slideru
 */
struct SliderConfig {
    juce::String parameterID;      // ID parametru (např. "masterGain")
    juce::String displayName;       // Zobrazené jméno (např. "Master Gain")
    juce::String labelText;         // Text labelu (např. "MASTER GAIN")
    double minValue = 0.0;          // Minimální hodnota
    double maxValue = 127.0;        // Maximální hodnota
    double defaultValue = 0.0;      // Výchozí hodnota
    double interval = 1.0;          // Krok změny hodnoty

    // Default konstruktor
    SliderConfig() = default;

    // Konstruktor s parametry
    SliderConfig(const juce::String& paramID,
                const juce::String& dispName,
                const juce::String& lblText,
                double defVal = 0.0,
                double minVal = 0.0,
                double maxVal = 127.0,
                double intv = 1.0)
        : parameterID(paramID)
        , displayName(dispName)
        , labelText(lblText)
        , minValue(minVal)
        , maxValue(maxVal)
        , defaultValue(defVal)
        , interval(intv)
    {}
};

/**
 * @brief Výsledek vytvoření slideru (slider + label + attachment)
 */
struct SliderBundle {
    std::unique_ptr<MidiLearnSlider> slider;
    std::unique_ptr<juce::Label> label;
    std::unique_ptr<juce::SliderParameterAttachment> attachment;
    SliderConfig config;

    SliderBundle() = default;

    // Move-only (obsahuje unique_ptr)
    SliderBundle(SliderBundle&&) = default;
    SliderBundle& operator=(SliderBundle&&) = default;
    SliderBundle(const SliderBundle&) = delete;
    SliderBundle& operator=(const SliderBundle&) = delete;
};

/**
 * @brief Factory pro vytváření sliderů podle DRY principu
 */
class SliderFactory {
public:
    using RightClickCallback = std::function<void(MidiLearnSlider*, juce::Point<int>)>;

    /**
     * @brief Vytvoří kompletní slider bundle (slider + label + attachment)
     * @param config Konfigurace slideru
     * @param parameters APVTS pro attachment
     * @param debugMode Debug mode flag
     * @param rightClickCallback Callback pro right-click (MIDI Learn menu)
     * @return SliderBundle s vytvořenými komponentami
     */
    static SliderBundle createSlider(
        const SliderConfig& config,
        juce::AudioProcessorValueTreeState& parameters,
        bool debugMode,
        RightClickCallback rightClickCallback);

    /**
     * @brief Přidá slider bundle do parent komponenty (addAndMakeVisible)
     * @param parent Parent komponenta
     * @param bundle Bundle k přidání
     */
    static void addToComponent(juce::Component& parent, SliderBundle& bundle);

    /**
     * @brief Nastaví visual feedback pro MIDI Learn
     * @param slider Slider k upravení
     * @param isLearning true = červený thumb, false = normální styling
     * @param debugMode Debug mode flag pro styling
     */
    static void setLearningVisuals(MidiLearnSlider* slider, bool isLearning, bool debugMode);

    /**
     * @brief Vytvoří map parameterID -> slider pro rychlé vyhledávání
     * @param bundles Vektor bundlů
     * @return Map pro lookup podle parameterID
     */
    static std::map<juce::String, MidiLearnSlider*> createParameterMap(
        std::vector<SliderBundle>& bundles);

private:
    SliderFactory() = delete; // Disable instantiation
};
