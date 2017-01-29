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

#include "SteamLinkVideoBuffer.h"

#include <stdlib.h>

using namespace STEAMLINK;

// --- SteamLinkBufferFree -----------------------------------------------------

void SteamLinkBufferFree::operator()(void* x)
{
  free(x);
}

// --- CSteamLinkBuffer --------------------------------------------------------

CSteamLinkBuffer::CSteamLinkBuffer(SteamLinkUniqueBuffer buffer, size_t size, std::shared_ptr<CSteamLinkVideoStream> stream) :
  buffer(std::move(buffer)),
  size(size),
  stream(std::move(stream))
{
}

CSteamLinkBuffer::CSteamLinkBuffer(CSteamLinkBuffer&& other) :
  buffer(std::move(other.buffer)),
  size(other.size),
  stream(std::move(other.stream))
{
  other.size = 0;
}
