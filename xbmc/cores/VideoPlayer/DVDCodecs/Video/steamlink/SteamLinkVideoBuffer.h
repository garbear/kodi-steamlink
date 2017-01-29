/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "SteamLinkUniqueBuffer.h"
#include "cores/VideoPlayer/Process/VideoBuffer.h"

#include <memory>
#include <stddef.h>

namespace KODI
{
namespace STEAMLINK
{

class CSteamLinkVideoStream;

class CSteamLinkVideoBuffer : public CVideoBuffer
{
public:
  CSteamLinkVideoBuffer(IVideoBufferPool& pool, int id);
  ~CSteamLinkVideoBuffer() override = default;

  void SetBuffer(SteamLinkUniqueBuffer buffer, size_t size, std::shared_ptr<CSteamLinkVideoStream> stream);
  void ClearBuffer();

  SteamLinkUniqueBuffer buffer;
  size_t size = 0;
  std::shared_ptr<CSteamLinkVideoStream> stream;
};

}
}
