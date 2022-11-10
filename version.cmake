add_custom_command(
    OUTPUT  ${PROJECT_BINARY_DIR}/sidewalk_version.c         
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_version_info.py > ${PROJECT_BINARY_DIR}/sidewalk_version.c
)

zephyr_library()
zephyr_library_sources(${PROJECT_BINARY_DIR}/sidewalk_version.c)
