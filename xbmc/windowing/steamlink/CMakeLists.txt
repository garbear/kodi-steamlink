set(SOURCES GLContextEGL.cpp
            WinSystemSteamLink.cpp
)

set(HEADERS GLContextEGL.h
            WinSystemSteamLink.h
)

if(OPENGLES_FOUND)
  list(APPEND SOURCES WinSystemSteamLinkGLESContext.cpp)
  list(APPEND HEADERS WinSystemSteamLinkGLESContext.h)
endif()

core_add_library(windowing_steamlink)
