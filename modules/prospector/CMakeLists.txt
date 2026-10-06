zephyr_library()
zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library_include_directories(${ZEPHYR_PROSPECTOR_ZMK_MODULE_MODULE_DIR}/include)
zephyr_library_include_directories(${ZEPHYR_PROSPECTOR_ZMK_MODULE_MODULE_DIR}/boards/shields/prospector_adapter/include)
zephyr_library_include_directories(${ZEPHYR_PROSPECTOR_ZMK_MODULE_MODULE_DIR}/boards/shields/prospector_adapter/src)
zephyr_library_sources(dpi.c brightness.c display.c)

# Keep upstream drivers, fonts and battery widgets; replace only the screen layout.
# The shield's directory is processed later, so apply this at the end of configure.
function(charybdis_replace_status_screen)
  set_source_files_properties(
    ${ZEPHYR_PROSPECTOR_ZMK_MODULE_MODULE_DIR}/boards/shields/prospector_adapter/src/custom_status_screen.c
    DIRECTORY ${ZEPHYR_PROSPECTOR_ZMK_MODULE_MODULE_DIR}/boards/shields/prospector_adapter
    PROPERTIES HEADER_FILE_ONLY ON)
endfunction()
cmake_language(DEFER DIRECTORY ${APPLICATION_SOURCE_DIR} CALL charybdis_replace_status_screen)
