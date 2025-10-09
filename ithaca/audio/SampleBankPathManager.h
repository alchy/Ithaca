/**
 * @file SampleBankPathManager.h
 * @brief Manages sample bank installation paths across platforms
 *
 * Handles saving and loading the user's sample bank installation directory
 * to persistent storage in JSON format in the appropriate platform-specific location:
 * - Windows: C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer\samplebank_config.json
 * - macOS: ~/Library/Application Support/LordAudio/IthacaPlayer/samplebank_config.json
 * - Linux: ~/.local/share/LordAudio/IthacaPlayer/samplebank_config.json
 */

#pragma once

#include <filesystem>
#include <string>
#include <optional>

namespace SampleBankPathManager
{
    /**
     * @brief Get the platform-specific plugin data directory
     *
     * Returns the directory where plugin metadata should be stored:
     * - Windows: C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer\
     * - macOS: ~/Library/Application Support/LordAudio/IthacaPlayer/
     * - Linux: ~/.local/share/LordAudio/IthacaPlayer/
     *
     * @return Path to the plugin data directory (creates it if needed)
     */
    std::filesystem::path getPluginDataDirectory();

    /**
     * @brief Save the sample bank installation path to JSON config
     *
     * Saves the path to samplebank_config.json in the plugin data directory.
     * File: samplebank_config.json
     *
     * @param path The sample bank installation path to save
     * @return true if saved successfully, false otherwise
     */
    bool saveSampleBankPath(const std::filesystem::path& path);

    /**
     * @brief Load the sample bank installation path from JSON config
     *
     * Loads the sample bank path from samplebank_config.json in roaming profile.
     *
     * @return The saved path if it exists and is valid, std::nullopt otherwise
     */
    std::optional<std::filesystem::path> loadSampleBankPath();

    /**
     * @brief Check if a saved sample bank config exists
     *
     * @return true if samplebank_config.json exists in roaming profile, false otherwise
     */
    bool hasSavedPath();

    /**
     * @brief Get the sample bank path to use
     *
     * Loads path from JSON config in roaming profile.
     * If not found or invalid, returns empty optional.
     * This is the main function to use when initializing the sampler.
     *
     * @return The sample bank path from config, or std::nullopt if not found
     */
    std::optional<std::filesystem::path> getSampleBankPath();

} // namespace SampleBankPathManager
