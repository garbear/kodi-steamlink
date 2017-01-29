/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SteamLinkVideoBuffer.h"

#include <stdlib.h>

using namespace KODI;
using namespace STEAMLINK;

CSteamLinkVideoBuffer::CSteamLinkVideoBuffer(IVideoBufferPool& pool, int id) :
  CVideoBuffer(id)
{
}

void CSteamLinkVideoBuffer::SetBuffer(SteamLinkUniqueBuffer buffer, size_t size, std::shared_ptr<CSteamLinkVideoStream> stream)
{
  this->buffer = std::move(buffer);
  this->size = size;
  this->stream = std::move(stream);
}

void CSteamLinkVideoBuffer::ClearBuffer()
{
  buffer.reset();
  size = 0;
  stream.reset();
}
