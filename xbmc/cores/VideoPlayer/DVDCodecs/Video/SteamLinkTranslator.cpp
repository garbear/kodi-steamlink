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

#include "SteamLinkTranslator.h"
#include "commons/ilog.h"

using namespace STEAMLINK;

bool CSteamLinkTranslator::TranslateFormat(AVCodecID format, ESLVideoFormat& slFormat)
{
  switch (format)
  {
  case AV_CODEC_ID_H264:
    slFormat = k_ESLVideoFormatH264;
    break;
  default:
    return false;
  }

  return true;
}

const char* CSteamLinkTranslator::TranslateFormatToString(ESLVideoFormat format)
{
  switch (format)
  {
  case k_ESLVideoFormatH264: return "h264";
  default:
    break;
  }

  return "";
}

int CSteamLinkTranslator::TranslateLogLevel(ESLVideoLog logLevel)
{
  switch (logLevel)
  {
  case k_ESLVideoLogDebug:   return LOGDEBUG;
  case k_ESLVideoLogInfo:    return LOGINFO;
  case k_ESLVideoLogWarning: return LOGWARNING;
  case k_ESLVideoLogError:   return LOGERROR;
    break;
  default:
    break;
  }

  return LOGDEBUG;
}
