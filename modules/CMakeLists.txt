add_subdirectory(pmw3610-peripheral)

if(CONFIG_SHIELD_CHARYBDIS_DONGLE AND CONFIG_SHIELD_PROSPECTOR_ADAPTER)
  add_subdirectory(prospector)
endif()
