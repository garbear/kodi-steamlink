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

#include "SteamLinkVideoStream.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

using namespace STEAMLINK;

CSteamLinkVideoStream::CSteamLinkVideoStream(CSLVideoContext *pContext,
                                             ESLVideoFormat eFormat,
                                             unsigned int fpsRate /* = 0 */,
                                             unsigned int fpsScale /* = 0 */) :
  m_pContext(pContext),
  m_eFormat(eFormat),
  m_fpsRate(fpsRate),
  m_fpsScale(fpsScale),
  m_stream(nullptr)
{
}

bool CSteamLinkVideoStream::Open()
{
  CSingleLock lock(m_streamMutex);

  m_stream = SLVideo_CreateStream(m_pContext, m_eFormat, false);

  if (m_stream != nullptr)
  {
    if (m_fpsRate > 0 && m_fpsScale > 0)
      SLVideo_SetStreamTargetFramerate(m_stream, m_fpsRate, m_fpsScale);

    return true;
  }

  return false;
}

void CSteamLinkVideoStream::Close()
{
  CSingleLock lock(m_streamMutex);

  if (m_stream)
  {
    SLVideo_FreeStream(m_stream);
    m_stream = nullptr;
  }

  m_delay.SetExpired();
}

bool CSteamLinkVideoStream::Flush()
{
  CSingleLock lock(m_streamMutex);

  Close();

  return Open();
}

bool CSteamLinkVideoStream::WriteData(const uint8_t* data, size_t size)
{
  uint32_t delayMs = 0;

  {
    CSingleLock lock(m_streamMutex);

    if (m_stream)
    {
      if (SLVideo_BeginFrame(m_stream, size) != 0)
      {
        CLog::Log(LOGERROR, "SteamLinkVideo: Failed to begin frame of size %u", size);
        return false;
      }

      if (SLVideo_WriteFrameData(m_stream, const_cast<uint8_t*>(data), size) != 0)
      {
        CLog::Log(LOGERROR, "SteamLinkVideo: Error writing data of size %u", size);
        return false;
      }

      if (SLVideo_SubmitFrame(m_stream) != 0)
      {
        CLog::Log(LOGERROR, "SteamLinkVideo: Error submitting frame of size %u", size);
        return false;
      }

      delayMs = SLVideo_GetQueuedVideoMS(m_stream);
    }
  }

  {
    CSingleLock lock(m_delayMutex);

    if (delayMs > 0)
    {
      //CLog::Log(LOGERROR, "CSteamLinkVideoStream - Delay = %u ms", delayMs);
      m_delay.Set(delayMs);
    }
    else
      ;//CLog::Log(LOGERROR, "CSteamLinkVideoStream - No delay!!!"); //! @todo
  }

  return true;
}

void CSteamLinkVideoStream::SetSpeed(float fps)
{
  CSingleLock lock(m_streamMutex);

  //! @todo
  CLog::Log(LOGERROR, "CSteamLinkVideoStream - SetSpeed() not implemented!");
}

unsigned int CSteamLinkVideoStream::GetDelayMs()
{
  CSingleLock lock(m_delayMutex);

  return m_delay.MillisLeft();
}
