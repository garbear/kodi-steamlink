/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SteamLinkVideoBufferPool.h"
#include "SteamLinkVideoBuffer.h"
#include "threads/SingleLock.h"

using namespace KODI;
using namespace STEAMLINK;

CSteamLinkVideoBufferPool::~CSteamLinkVideoBufferPool()
{
  for (auto buf : m_all)
    delete buf;
}

CVideoBuffer* CSteamLinkVideoBufferPool::Get()
{
  CSingleLock lock(m_critSection);

  CSteamLinkVideoBuffer* buf = nullptr;
  if (!m_free.empty())
  {
    int idx = m_free.front();
    m_free.pop_front();
    m_used.push_back(idx);
    buf = m_all[idx];
  }
  else
  {
    int id = m_all.size();
    buf = new CSteamLinkVideoBuffer(*this, id);
    m_all.push_back(buf);
    m_used.push_back(id);
  }

  buf->Acquire(GetPtr());
  return buf;
}

void CSteamLinkVideoBufferPool::Return(int id)
{
  CSingleLock lock(m_critSection);

  m_all[id]->ClearBuffer();
  auto it = m_used.begin();
  while (it != m_used.end())
  {
    if (*it == id)
    {
      m_used.erase(it);
      break;
    }
    else
      ++it;
  }
  m_free.push_back(id);
}
