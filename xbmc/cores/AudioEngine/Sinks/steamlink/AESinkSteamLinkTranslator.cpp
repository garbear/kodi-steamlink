/*
 *  Copyright (C) 2018 Team Kodi
 *  Copyright (C) 2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "AESinkSteamLinkTranslator.h"
#include "commons/ilog.h"

using namespace KODI;
using namespace STEAMLINK;

int CAESinkSteamLinkTranslator::TranslateLogLevel(ESLAudioLog logLevel)
{
  switch (logLevel)
  {
  case k_ESLAudioLogDebug:   return LOGDEBUG;
  case k_ESLAudioLogInfo:    return LOGINFO;
  case k_ESLAudioLogWarning: return LOGWARNING;
  case k_ESLAudioLogError:   return LOGERROR;
    break;
  default:
    break;
  }

  return LOGDEBUG;
}
