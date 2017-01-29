set(SOURCES SteamLinkTranslator.cpp
            SteamLinkUniqueBuffer.cpp
            SteamLinkVideo.cpp
            SteamLinkVideoBuffer.cpp
            SteamLinkVideoBufferPool.cpp
            SteamLinkVideoStream.cpp
)

set(HEADERS SteamLinkTranslator.h
            SteamLinkUniqueBuffer.h
            SteamLinkVideo.h
            SteamLinkVideoBuffer.h
            SteamLinkVideoBufferPool.h
            SteamLinkVideoStream.h
)

core_add_library(steamlink_videocodec)
