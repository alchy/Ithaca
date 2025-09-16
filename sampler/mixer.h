#ifndef MIXER_H
#define MIXER_H

#include <vector>
#include <cstdint>

/**
 * @struct VoiceData
 * @brief Data structure obsahující audio a energy informace z jednoho Voice
 */
struct VoiceData {
    const float* audioLeft;      // Pointer na levý kanál audio dat
    const float* audioRight;     // Pointer na pravý kanál audio dat  
    const float* energyBuffer;   // Pointer na energy buffer (envelope_gain * velocity_gain)
    bool isActive;               // Zda je Voice aktivní
    
    VoiceData() : audioLeft(nullptr), audioRight(nullptr), energyBuffer(nullptr), isActive(false) {}
    
    VoiceData(const float* left, const float* right, const float* energy, bool active)
        : audioLeft(left), audioRight(right), energyBuffer(energy), isActive(active) {}
};

/**
 * @class Mixer
 * @brief Energy-based audio mixer pro kombinování multiple voice signals
 * 
 * Implementuje adaptivní gain algoritmus založený na energetické analýze místo
 * prostého počítání hlasů. Využívá gain-sharing princip kde celková energie
 * systému určuje míru gain redukce.
 * 
 * Klíčové vlastnosti:
 * - Energy-based gain calculation (envelope_gain * velocity_gain)
 * - Odmocninová gain redukce pro vysokou energii
 * - Ochrana při nízké energii (žádná zbytečná redukce)
 * - RT-safe processing bez alokací
 * - Soft saturation ochrana proti clipping
 */
class Mixer {
public:
    /**
     * @brief Konstruktor s konfiguračními parametry
     */
    Mixer();

    /**
     * @brief RT-SAFE: Hlavní mixovací metoda kombinující multiple voices
     * 
     * Implementuje energy-based gain sharing algoritmus:
     * 1. Pro každý sample spočítá celkovou energii všech voices
     * 2. Aplikuje adaptivní gain na základě energie (ne počtu hlasů)
     * 3. Kombinuje všechny voice signály do output bufferů
     * 
     * @param voices Vector s VoiceData strukturami (audio + energy data)
     * @param outputLeft Výstupní buffer pro levý kanál 
     * @param outputRight Výstupní buffer pro pravý kanál
     * @param numSamples Počet samples k zpracování
     * @return true pokud byl vyproducen audio output, false pro silence
     */
    bool mixVoices(const std::vector<VoiceData>& voices, 
                   float* outputLeft, float* outputRight, int numSamples) noexcept;

    /**
     * @brief RT-SAFE: Nastavení konfiguračních parametrů algoritmu
     */
    void setLowEnergyThreshold(float threshold) noexcept { lowEnergyThreshold_ = threshold; }
    void setSaturationThreshold(float threshold) noexcept { saturationThreshold_ = threshold; }
    void setGainReductionStrength(float strength) noexcept { gainReductionStrength_ = strength; }

    /**
     * @brief RT-SAFE: Gettery pro monitoring
     */
    float getLastTotalEnergy() const noexcept { return lastTotalEnergy_; }
    float getLastAdaptiveGain() const noexcept { return lastAdaptiveGain_; }
    int getLastActiveVoicesCount() const noexcept { return lastActiveVoicesCount_; }

private:
    // Konfigurační parametry
    float lowEnergyThreshold_;     // Hranice pro ochranu při nízké energii (default 0.1f)
    float saturationThreshold_;    // Hranice pro soft saturation (default 0.9f) 
    float gainReductionStrength_;  // Síla gain redukce (default 1.0f)

    // Monitoring proměnné (pro debugging/statistiky)
    mutable float lastTotalEnergy_;
    mutable float lastAdaptiveGain_;
    mutable int lastActiveVoicesCount_;

    /**
     * @brief RT-SAFE: Výpočet adaptivního gain na základě celkové energie
     * 
     * Algoritmus:
     * - Nízká energie (< threshold): žádná redukce (gain = 1.0)
     * - Vysoká energie: odmocninová redukce (gain = 1.0 / sqrt(totalEnergy))
     * - Soft saturation pro extrémní hodnoty
     * 
     * @param totalEnergy Celková energie všech voices v daném sample
     * @return Adaptivní gain hodnota (0.0-1.0)
     */
    float calculateAdaptiveGain(float totalEnergy) const noexcept;

    /**
     * @brief RT-SAFE: Soft saturation funkce pro zabránění hard clipping
     * @param value Input hodnota
     * @return Saturated hodnota
     */
    float applySoftSaturation(float value) const noexcept;
};

#endif // MIXER_H
