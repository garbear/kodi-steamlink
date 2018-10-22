set(SOURCES GLContextEGL.cpp
            SteamLinkPerformance.cpp
            WinEventsSteamLink.cpp
            WinSystemSteamLink.cpp
)

set(HEADERS GLContextEGL.h
            SteamLinkPerformance.h
            WinEventsSteamLink.h
            WinSystemSteamLink.h
)

if(OPENGLES_FOUND)
  list(APPEND SOURCES WinSystemSteamLinkGLESContext.cpp)
  list(APPEND HEADERS WinSystemSteamLinkGLESContext.h)
endif()

core_add_library(windowing_steamlink)
