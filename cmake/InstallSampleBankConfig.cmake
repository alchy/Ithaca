# CMake script to install samplebank_config.json to user's roaming profile
# This script is executed as a POST_BUILD step
#
# MULTIPLE INSTRUMENT INSTANCES support:
# Each instrument instance gets its own config directory
# Example: IthacaPlayer-VintageV → %APPDATA%/LordAudio/IthacaPlayer-VintageV/
#
# This allows multiple instrument instances to coexist with separate configs

cmake_minimum_required(VERSION 3.22)

set(COMPANY_NAME "LordAudio")

# PLUGIN_NAME is passed from CMakeLists.txt via -DPLUGIN_NAME=...
# It includes the instrument suffix (e.g., "IthacaPlayer-VintageV")
if(NOT DEFINED PLUGIN_NAME)
    message(FATAL_ERROR "PLUGIN_NAME must be defined via -DPLUGIN_NAME=<name>")
endif()

# PLUGIN_CODE is passed from CMakeLists.txt via -DPLUGIN_CODE=...
# It's the 4-character code (e.g., "VntV" for VintageV)
if(NOT DEFINED PLUGIN_CODE)
    message(FATAL_ERROR "PLUGIN_CODE must be defined via -DPLUGIN_CODE=<code>")
endif()

# Config filename includes PLUGIN_CODE for unique identification
# Example: "samplebank-config-VntV.json"
# Naming convention matches instrument-definition.json pattern
set(CONFIG_FILENAME "samplebank-config-${PLUGIN_CODE}.json")

message(STATUS "=== Installing Sample Bank Config ===")
message(STATUS "Plugin Name: ${PLUGIN_NAME}")
message(STATUS "Config File: ${CONFIG_FILENAME}")

# Determine platform-specific roaming directory
if(WIN32)
    # Windows: %APPDATA%\LordAudio\IthacaPlayer-VintageV
    file(TO_CMAKE_PATH "$ENV{APPDATA}" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")

elseif(APPLE)
    # macOS: ~/Library/Application Support/LordAudio/IthacaPlayer-VintageV
    file(TO_CMAKE_PATH "$ENV{HOME}/Library/Application Support" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")

else()
    # Linux: ~/.local/share/LordAudio/IthacaPlayer-VintageV
    file(TO_CMAKE_PATH "$ENV{HOME}/.local/share" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")
endif()

message(STATUS "Target Directory: ${PLUGIN_DATA_DIR}")

# Create directory if it doesn't exist
file(MAKE_DIRECTORY "${PLUGIN_DATA_DIR}")

# Copy JSON config to roaming profile (only if it doesn't exist yet)
set(TARGET_FILE "${PLUGIN_DATA_DIR}/${CONFIG_FILENAME}")
if(NOT EXISTS "${TARGET_FILE}")
    message(STATUS "Installing sample bank config to: ${TARGET_FILE}")

    # Read source JSON and fix backslashes to forward slashes for JSON compatibility
    file(READ "${SOURCE_JSON}" JSON_CONTENT)
    string(REPLACE "\\" "/" JSON_CONTENT_FIXED "${JSON_CONTENT}")

    # Write fixed JSON to target
    file(WRITE "${TARGET_FILE}" "${JSON_CONTENT_FIXED}")
    message(STATUS "Sample bank config installed with forward slashes for JSON compatibility")
else()
    message(STATUS "Sample bank config already exists (preserving user settings): ${TARGET_FILE}")
endif()
