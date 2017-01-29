/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SteamLinkTranslator.h"
#include "commons/ilog.h"

using namespace KODI;
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
