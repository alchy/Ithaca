# CMake script to install samplebank_config.json to user's roaming profile
# This script is executed as a POST_BUILD step

cmake_minimum_required(VERSION 3.22)

set(COMPANY_NAME "LordAudio")
set(PLUGIN_NAME "IthacaPlayer")
set(CONFIG_FILENAME "samplebank_config.json")

# Determine platform-specific roaming directory
if(WIN32)
    # Windows: %APPDATA%\LordAudio\IthacaPlayer
    file(TO_CMAKE_PATH "$ENV{APPDATA}" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")

elseif(APPLE)
    # macOS: ~/Library/Application Support/LordAudio/IthacaPlayer
    file(TO_CMAKE_PATH "$ENV{HOME}/Library/Application Support" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")

else()
    # Linux: ~/.local/share/LordAudio/IthacaPlayer
    file(TO_CMAKE_PATH "$ENV{HOME}/.local/share" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")
endif()

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
