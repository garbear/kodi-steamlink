#.rst:
# FindSdl2
# -------
# Finds the SDL2 library
#
# This will will define the following variables::
#
# SDL2_FOUND - system has SDL2
# SDL2_INCLUDE_DIRS - the SDL2 include directory
# SDL2_LIBRARIES - the SDL2 libraries
# SDL2_DEFINITIONS - the SDL2 compile definitions

find_path(SDL2_INCLUDE_DIR NAMES SDL2/SDL.h)
find_library(SDL2_LIBRARY NAMES SDL2)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sdl2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR)

if(SDL2_FOUND)
  set(SDL2_LIBRARIES ${SDL2_LIBRARY})
  set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
  set(SDL2_DEFINITIONS -DHAVE_SDL=1 -DHAVE_SDL_VERSION=2)
endif()

mark_as_advanced(SDL2_LIBRARY SDL2_INCLUDE_DIR)
