/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinEventsSteamLink.h"

#include <SDL2/SDL_events.h>

using namespace KODI;
using namespace STEAMLINK;

bool CWinEventsSteamLink::MessagePump()
{
  bool bReturn = false;

  SDL_Event event;
  while (SDL_PollEvent(&event))
    bReturn |= true;

  return bReturn;
}
