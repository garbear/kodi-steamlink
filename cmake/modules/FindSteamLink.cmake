# FindSteamLink
# ----------
# Finds the FindSteamLink headers and libraries
#
# This will will define the following variables::
#
# STEAMLINK_FOUND - system has Steam Link
# STEAMLINK_INCLUDE_DIRS - the Steam Link include directory
# STEAMLINK_LIBRARIES - the Steam Link libraries
# STEAMLINK_DEFINITIONS  - the Steam Link definitions
#
# and the following imported targets::
#
#   STEAMLINK::STEAMLINK   - The Steam Link library

find_path(STEAMLINK_INCLUDE_DIR NAMES SLVideo.h)

find_library(STEAMLINK_VIDEO_LIBRARY NAMES SLVideo)
find_library(STEAMLINK_AUDIO_LIBRARY NAMES SLAudio)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SteamLink
                                  REQUIRED_VARS STEAMLINK_INCLUDE_DIR
                                                STEAMLINK_VIDEO_LIBRARY
                                                STEAMLINK_AUDIO_LIBRARY)

if(STEAMLINK_FOUND)
  set(STEAMLINK_LIBRARIES ${STEAMLINK_VIDEO_LIBRARY}
                          ${STEAMLINK_AUDIO_LIBRARY})
  set(STEAMLINK_INCLUDE_DIRS ${STEAMLINK_INCLUDE_DIR})
  set(STEAMLINK_DEFINITIONS -DHAS_STEAMLINK)
    if(NOT TARGET STEAMLINK::STEAMLINK)
    add_library(STEAMLINK::STEAMLINK UNKNOWN IMPORTED)
    set_target_properties(STEAMLINK::STEAMLINK PROPERTIES
                                               IMPORTED_LOCATION "${STEAMLINK_VIDEO_LIBRARY}" # TODO
                                               INTERFACE_INCLUDE_DIRECTORIES "${STEAMLINK_INCLUDE_DIR}")
  endif()
endif()

mark_as_advanced(STEAMLINK_INCLUDE_DIR STEAMLINK_VIDEO_LIBRARY STEAMLINK_AUDIO_LIBRARY)
