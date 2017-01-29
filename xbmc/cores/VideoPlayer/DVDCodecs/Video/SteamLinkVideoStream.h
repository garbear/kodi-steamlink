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
#pragma once

#include "threads/CriticalSection.h"
#include "threads/SystemClock.h"

// Steam Link video API
#include "SLVideo.h"

#include <stdint.h>

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
