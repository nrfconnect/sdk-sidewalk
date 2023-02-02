find_package(PythonInterp)
find_package(Python)

execute_process(COMMAND ${PYTHON_EXECUTABLE} $ENV{ZEPHYR_BASE}/../sidewalk/scripts/generate_version_info_overlay.py
OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
list(APPEND OVERLAY_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
