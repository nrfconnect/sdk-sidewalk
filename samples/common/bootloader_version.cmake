execute_process(COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/generate_version_info_overlay.py
OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
list(APPEND OVERLAY_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
