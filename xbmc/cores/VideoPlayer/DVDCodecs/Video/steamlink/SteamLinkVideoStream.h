/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "threads/CriticalSection.h"
#include "threads/SystemClock.h"

// Steam Link video API
#include "SLVideo.h"

#include <stdint.h>

namespace KODI
{
namespace STEAMLINK
{

class CSteamLinkVideoStream
{
public:
  CSteamLinkVideoStream(CSLVideoContext *pContext,
                        ESLVideoFormat format,
                        unsigned int fpsRate = 0,
                        unsigned int fpsScale = 0);

  ~CSteamLinkVideoStream() { Close(); }

  bool Open();
  void Close();

  bool Flush();

  bool WriteData(const uint8_t* data, size_t size);

  void SetSpeed(float speed);

  unsigned int GetDelayMs();

private:
  // Construction parameters
  CSLVideoContext * const m_pContext;
  const ESLVideoFormat m_eFormat;
  const unsigned int m_fpsRate;
  const unsigned int m_fpsScale;

  // Stream parameters
  CSLVideoStream *m_stream;
  CCriticalSection m_streamMutex;
  XbmcThreads::EndTime m_delay;;
  CCriticalSection m_delayMutex;
};

}
}
