# FindXkeyboard-config
# ----------
# Finds the xkeyboard-config distribution files
#
# This will define the following variables::
#
# XKEYBOARD-CONFIG_FOUND - system has xkeyboard-config
#
# and install the keyboard configs to the data root directory.
#

pkg_check_modules(PC_XKEYBOARD_CONFIG xkeyboard-config)

if(PC_XKEYBOARD_CONFIG_FOUND)
  set(XKEYBOARD_CONFIG_PREFIX ${PC_XKEYBOARD_CONFIG_PREFIX})
  set(XKEYBOARD_CONFIG_VERSION ${PC_XKEYBOARD_CONFIG_VERSION})
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xkeyboard-config
  REQUIRED_VARS XKEYBOARD_CONFIG_PREFIX
  VERSION_VAR XKEYBOARD_CONFIG_VERSION)

find_file(XKEYBOARD_CONFIG_DIR X11 PATHS ${XKEYBOARD_CONFIG_PREFIX})

if(XKEYBOARD-CONFIG_FOUND)
  install(DIRECTORY ${XKEYBOARD_CONFIG_DIR}/ DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/X11)
endif()
