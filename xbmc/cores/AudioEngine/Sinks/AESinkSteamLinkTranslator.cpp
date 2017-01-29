/*
 *      Copyright (C) 2016 Team Kodi
 *      Copyright (C) 2016 Valve Corporation
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "AESinkSteamLinkTranslator.h"
#include "commons/ilog.h"

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
