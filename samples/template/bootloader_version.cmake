find_package(PythonInterp 3.6 REQUIRED)
find_package(Python 3.6 REQUIRED)

execute_process(COMMAND ${PYTHON_EXECUTABLE} $ENV{ZEPHYR_BASE}/../sidewalk/scripts/generate_version_info_overlay.py
OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
list(APPEND OVERLAY_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
