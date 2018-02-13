/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "windowing/WinEvents.h"

namespace KODI
{
namespace STEAMLINK
{

class CWinEventsSteamLink : public IWinEvents
{
public:
  // Implementation of IWinEvents
  bool MessagePump() override;
};

}
}
