/*
 *      Copyright (C) 2016 Team Kodi
 *      Copyright (C) 2016 Valve Corporation
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

#include "AESinkSteamLinkStream.h"
#include "AESinkSteamLink.h"
#include "cores/VideoPlayer/DVDCodecs/Video/SteamLinkVideo.h"
#include "interfaces/AnnouncementManager.h"
#include "threads/SingleLock.h"
//#include "threads/SystemClock.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

// Steam Link audio API
#include "SLAudio.h"

#include <algorithm>
#include <cmath>
#include <cstring>

using namespace STEAMLINK;

#define MAX_AUDIO_DELAY_MS  100 // Skip packets if audio delay exceeds this value

CAESinkSteamLinkStream::CAESinkSteamLinkStream(CSLAudioContext* context, unsigned int sampleRateHz, unsigned int channels, unsigned int packetSize) :
  CThread("SteamLinkAudio"),
  m_context(context),
  m_sampleRateHz(sampleRateHz),
  m_channels(channels),
  m_packetSize(packetSize),
  m_stream(nullptr),
  m_bInVideo(false),
  m_steamLinkBuffer(nullptr),
  m_remainingBytes(0)
{
  ANNOUNCEMENT::CAnnouncementManager::GetInstance().AddAnnouncer(this);
}

CAESinkSteamLinkStream::~CAESinkSteamLinkStream()
{
  ANNOUNCEMENT::CAnnouncementManager::GetInstance().RemoveAnnouncer(this);

  Close();
}

bool CAESinkSteamLinkStream::Open()
{
  Close();

  CSingleLock lock(m_streamMutex);

  bool bSuccess = false;

  m_stream = SLAudio_CreateStream(m_context, m_sampleRateHz, m_channels, m_packetSize, true);

  if (m_stream)
  {
    bSuccess = true;
    lock.Leave();
    Create(false);
  }
  else
  {
    CLog::Log(LOGERROR, "SteamLinkAudio: Failed to create stream");
  }

  return bSuccess;
}

void CAESinkSteamLinkStream::Close()
{
  StopThread();

  CSingleLock lock(m_streamMutex);

  if (m_stream)
  {
    SLAudio_FreeStream(m_stream);
    m_stream = nullptr;
  }
}

bool CAESinkSteamLinkStream::Flush()
{
  Close();

  {
    CSingleLock lock(m_queueMutex);
    m_queue.clear();
  }

  return Open();
}

bool CAESinkSteamLinkStream::AddPacket(std::unique_ptr<uint8_t[]> data, unsigned int size, double presentTimeSecs)
{
  {
    CSingleLock lock(m_streamMutex);
    if (m_stream == nullptr)
      return true; // This might have been called during a Flush()
  }

  const unsigned int delayMs = CSteamLinkVideo::GetDelayMs();

  if (delayMs > 0)
    presentTimeSecs += delayMs / 1000.0;

  {
    CSingleLock lock(m_queueMutex);
    m_queue.emplace_back(std::move(AudioPacket{std::move(data), size, presentTimeSecs}));
  }

  // Notify thread that a packet is ready
  m_queueEvent.Set();

  return true;
}

void CAESinkSteamLinkStream::Process()
{
  AudioPacket packet;

  while (!m_bStop)
  {
    if (packet.buffer || GetNextPacket(packet))
    {
      // Sleep until we're ready to show the frame
      WaitUntilReady(packet.presentTimeSecs);

      if (m_bStop)
        break;

      SendPacket(std::move(packet));

      if (GetNextPacket(packet))
        continue;
    }

    AbortableWait(m_queueEvent);
  }

  // Make sure we haven't left a Steam Link packet open
  if (m_steamLinkBuffer != nullptr)
    EndPacket();
}

void CAESinkSteamLinkStream::Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  if (flag == ANNOUNCEMENT::Player && strcmp(sender, "xbmc") == 0)
  {
    if (strcmp(message, "OnPlay") == 0 ||
        strcmp(message, "OnPause") == 0 ||
        strcmp(message, "OnStop") == 0 ||
        strcmp(message, "OnSpeedChanged") == 0 ||
        strcmp(message, "OnSeek") == 0)
    {
      Flush();
    }
    else
    {
      CLog::Log(LOGDEBUG, "CAESinkSteamLinkStream: Unknown player announcement \"%s\"", message);
    }
  }
}

bool CAESinkSteamLinkStream::GetNextPacket(AudioPacket& packet)
{
  CSingleLock lock(m_queueMutex);

  if (!m_queue.empty())
  {
    packet = std::move(m_queue.front());
    m_queue.pop_front();
    return true;
  }

  return false;
}

void CAESinkSteamLinkStream::WaitUntilReady(double targetTimeSecs)
{
  const double nowSecs = static_cast<double>(CurrentHostCounter()) / CurrentHostFrequency();

  const int sleepTimeMs = static_cast<int>((targetTimeSecs - nowSecs) * 1000.0);

  if (sleepTimeMs > 0)
    Sleep(sleepTimeMs);
}

bool CAESinkSteamLinkStream::HasLatePacket(double nextPresentTimeSecs) const
{
  return std::find_if(m_queue.begin(), m_queue.end(),
    [nextPresentTimeSecs](const AudioPacket& packet)
    {
      return packet.presentTimeSecs >= nextPresentTimeSecs;
    }) != m_queue.end();
}

void CAESinkSteamLinkStream::ClearLatePackets(double nextPresentTimeSecs)
{
  while (HasLatePacket(nextPresentTimeSecs))
    m_queue.pop_front();
}

void CAESinkSteamLinkStream::BeginPacket()
{
  m_steamLinkBuffer = static_cast<uint8_t*>(SLAudio_BeginFrame(m_stream));
  m_remainingBytes = m_packetSize;
}

void CAESinkSteamLinkStream::EndPacket()
{
  SLAudio_SubmitFrame(m_stream);
  m_steamLinkBuffer = nullptr;
  m_remainingBytes = 0;
}

void CAESinkSteamLinkStream::SendPacket(AudioPacket packet)
{
  CSingleLock lock(m_streamMutex);

  if (GetSLDelaySecs() > MAX_AUDIO_DELAY_MS)
    Flush();

  if (m_stream == nullptr)
    return;

  unsigned int bytesWritten = 0;

  // Loop until all bytes have been written
  while (bytesWritten < packet.size)
  {
    if (m_steamLinkBuffer == nullptr)
      BeginPacket();

    const unsigned int bytesToWrite = std::min(m_remainingBytes, packet.size - bytesWritten);

    // Sanity check (shouldn't happen)
    if (bytesToWrite == 0 || m_remainingBytes == 0)
      break;

    const unsigned int bufferOffset = m_packetSize - m_remainingBytes;
    std::memcpy(m_steamLinkBuffer + bufferOffset, packet.buffer.get() + bytesWritten, bytesToWrite);

    m_remainingBytes -= bytesToWrite;
    bytesWritten += bytesToWrite;

    if (m_remainingBytes == 0)
      EndPacket();
  }
}

double CAESinkSteamLinkStream::GetSLDelaySecs()
{
  CSingleLock lock(m_streamMutex);

  uint32_t queuedFrames = 0;

  if (m_stream)
    queuedFrames = SLAudio_GetQueuedAudioSamples(m_stream) / m_channels;

  return static_cast<double>(queuedFrames) / m_sampleRateHz;
}
