/*
 *  Copyright (C) 2018 Team Kodi
 *  Copyright (C) 2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RPProcessInfoSteamLink.h"

using namespace KODI;
using namespace STEAMLINK;

CRPProcessInfoSteamLink::CRPProcessInfoSteamLink() :
  CRPProcessInfo("SteamLink")
{
}

RETRO::CRPProcessInfo* CRPProcessInfoSteamLink::Create()
{
  return new CRPProcessInfoSteamLink();
}

void CRPProcessInfoSteamLink::Register()
{
  CRPProcessInfo::RegisterProcessControl(CRPProcessInfoSteamLink::Create);
}
