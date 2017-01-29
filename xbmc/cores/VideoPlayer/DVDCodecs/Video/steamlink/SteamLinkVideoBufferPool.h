/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/VideoPlayer/Process/VideoBuffer.h"
#include "threads/CriticalSection.h"

#include <deque>
#include <vector>

namespace KODI
{
namespace STEAMLINK
{

class CSteamLinkVideoBuffer;

class CSteamLinkVideoBufferPool : public IVideoBufferPool
{
public:
  ~CSteamLinkVideoBufferPool();

  // Implementation of IVideoBufferPool
  void Return(int id) override;
  CVideoBuffer* Get() override;

protected:
  CCriticalSection m_critSection;
  std::vector<CSteamLinkVideoBuffer*> m_all;
  std::deque<int> m_used;
  std::deque<int> m_free;
};

}
}
