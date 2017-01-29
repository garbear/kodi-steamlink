/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "SLVideo.h"
#include "libavcodec/avcodec.h"

namespace KODI
{
namespace STEAMLINK
{

class CSteamLinkTranslator
{
public:
  static bool TranslateFormat(AVCodecID format, ESLVideoFormat& slFormat);

  static const char* TranslateFormatToString(ESLVideoFormat format);

  static int TranslateLogLevel(ESLVideoLog logLevel);
};

}
}
