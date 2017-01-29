/*
 *  Copyright (C) 2018 Team Kodi
 *  Copyright (C) 2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/RetroPlayer/process/RPProcessInfo.h"

namespace KODI
{
namespace STEAMLINK
{
  class CRPProcessInfoSteamLink : public RETRO::CRPProcessInfo
  {
  public:
    CRPProcessInfoSteamLink();

    static CRPProcessInfo* Create();
    static void Register();
  };
}
}
