/**
 * @file SampleBankPathManager.cpp
 * @brief Implementation of sample bank path management with JSON support
 */

#include "ithaca/audio/SampleBankPathManager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Platform-specific includes
#if defined(_WIN32)
    #include <windows.h>
    #include <shlobj.h>
    #pragma comment(lib, "shell32.lib")
#elif defined(__APPLE__)
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

using json = nlohmann::json;

namespace SampleBankPathManager
{
    namespace
    {
        // Company and plugin names from CMakeLists.txt
        constexpr const char* COMPANY_NAME = "LordAudio";
        constexpr const char* PLUGIN_NAME = "IthacaPlayer";
        constexpr const char* CONFIG_FILENAME = "samplebank_config.json";
        constexpr const char* JSON_KEY_PATH = "sampleBankPath";
        constexpr const char* JSON_KEY_VERSION = "version";
    }

    std::filesystem::path getPluginDataDirectory()
    {
        std::filesystem::path basePath;

#if defined(_WIN32)
        // Windows: C:\Users\<user>\AppData\Roaming
        PWSTR pathTmp = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathTmp)))
        {
            basePath = std::filesystem::path(pathTmp);
            CoTaskMemFree(pathTmp);
        }
        else
        {
            // Fallback to temp directory if API fails
            basePath = std::filesystem::temp_directory_path();
        }

#elif defined(__APPLE__)
        // macOS: ~/Library/Application Support
        const char* home = getenv("HOME");
        if (!home)
        {
            struct passwd* pw = getpwuid(getuid());
            if (pw)
                home = pw->pw_dir;
        }
        if (home)
        {
            basePath = std::filesystem::path(home) / "Library" / "Application Support";
        }
        else
        {
            basePath = std::filesystem::temp_directory_path();
        }

#elif defined(__linux__)
        // Linux: ~/.local/share
        const char* home = getenv("HOME");
        if (!home)
        {
            struct passwd* pw = getpwuid(getuid());
            if (pw)
                home = pw->pw_dir;
        }
        if (home)
        {
            basePath = std::filesystem::path(home) / ".local" / "share";
        }
        else
        {
            basePath = std::filesystem::temp_directory_path();
        }
#endif

        // Append company and plugin name
        std::filesystem::path pluginDir = basePath / COMPANY_NAME / PLUGIN_NAME;

        // Create directories if they don't exist
        try
        {
            std::filesystem::create_directories(pluginDir);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Failed to create plugin data directory: " << e.what() << std::endl;
            // Return the path anyway - caller will handle errors
        }

        return pluginDir;
    }

    bool saveSampleBankPath(const std::filesystem::path& path)
    {
        try
        {
            // Get plugin data directory
            std::filesystem::path dataDir = getPluginDataDirectory();
            std::filesystem::path configFile = dataDir / CONFIG_FILENAME;

            // Create JSON object with forward slashes (JSON standard)
            std::string pathStr = path.string();
            // Replace backslashes with forward slashes for JSON compatibility
            std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

            json config;
            config[JSON_KEY_PATH] = pathStr;
            config[JSON_KEY_VERSION] = "1.0";

            // Write JSON to file
            std::ofstream file(configFile, std::ios::out | std::ios::trunc);
            if (!file.is_open())
            {
                std::cerr << "Failed to open config file for writing: " << configFile << std::endl;
                return false;
            }

            file << config.dump(2);  // Pretty print with 2-space indentation
            file.close();

            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to save sample bank config: " << e.what() << std::endl;
            return false;
        }
    }

    std::optional<std::filesystem::path> loadSampleBankPath()
    {
        try
        {
            // Get plugin data directory
            std::filesystem::path dataDir = getPluginDataDirectory();
            std::filesystem::path configFile = dataDir / CONFIG_FILENAME;

            // Check if file exists
            if (!std::filesystem::exists(configFile))
            {
                return std::nullopt;
            }

            // Read JSON from file
            std::ifstream file(configFile);
            if (!file.is_open())
            {
                std::cerr << "Failed to open config file for reading: " << configFile << std::endl;
                return std::nullopt;
            }

            json config;
            file >> config;
            file.close();

            // Extract path from JSON
            if (!config.contains(JSON_KEY_PATH))
            {
                std::cerr << "Config file missing '" << JSON_KEY_PATH << "' key" << std::endl;
                return std::nullopt;
            }

            std::string pathStr = config[JSON_KEY_PATH].get<std::string>();

            // Validate that the path is not empty
            if (pathStr.empty())
            {
                return std::nullopt;
            }

            std::filesystem::path loadedPath(pathStr);

            // Optionally verify that the path exists
            if (!std::filesystem::exists(loadedPath))
            {
                std::cerr << "Warning: Sample bank path from config does not exist: " << pathStr << std::endl;
                // Return it anyway - user might have moved files, plugin can show error
            }

            return loadedPath;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to load sample bank config: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    bool hasSavedPath()
    {
        try
        {
            std::filesystem::path dataDir = getPluginDataDirectory();
            std::filesystem::path configFile = dataDir / CONFIG_FILENAME;
            return std::filesystem::exists(configFile);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::optional<std::filesystem::path> getSampleBankPath()
    {
        // Load path from JSON config in roaming profile
        return loadSampleBankPath();
    }

} // namespace SampleBankPathManager
