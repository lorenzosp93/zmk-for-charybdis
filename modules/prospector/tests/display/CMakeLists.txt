cmake_minimum_required(VERSION 3.20)
project(chary_display_test C)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NOT LVGL_ROOT OR NOT PROSPECTOR_ROOT)
  message(FATAL_ERROR "Set LVGL_ROOT and PROSPECTOR_ROOT to the pinned dependency checkouts")
endif()
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "")
add_subdirectory(${LVGL_ROOT} lvgl EXCLUDE_FROM_ALL)
set(BATTERY_SRC "${PROSPECTOR_ROOT}/boards/shields/prospector_adapter/src/widgets/battery_bar.c")
set(DISPLAY_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../display.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BATTERY_SRC}" "${DISPLAY_SRC}" prefix.c main.c)
execute_process(COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/generate.py"
  "${BATTERY_SRC}" "${DISPLAY_SRC}" "${CMAKE_CURRENT_BINARY_DIR}/fixture.c"
  COMMAND_ERROR_IS_FATAL ANY)
add_executable(display_test "${CMAKE_CURRENT_BINARY_DIR}/fixture.c"
  "${PROSPECTOR_ROOT}/boards/shields/prospector_adapter/src/fonts/FoundryGridnikMedium_20.c")
target_link_libraries(display_test lvgl m)
enable_testing()
add_test(NAME display_regressions COMMAND display_test)
