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

#include "AESinkSteamLink.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

// Steam Link audio API
#include "SLAudio.h"

#include <cstring>
#include <unistd.h>

#define SL_SAMPLE_RATE  48000
#define SINK_FEED_MS    50 // Steam Link game streaming uses 10ms
#define CACHE_TOTAL_MS  200
#define INITIAL_ATTENUATION_TIME 6.0
#define DELAY_OFFSET_SECONDS -0.025

using namespace STEAMLINK;

namespace STEAMLINK
{
  extern uint32_t g_unQueuedVideoMS;
}

namespace
{
  void LogFunction(void *pContext, ESLAudioLog eLogLevel, const char *pszMessage)
  {
    switch (eLogLevel)
    {
    case k_ESLAudioLogDebug:
      CLog::Log(LOGDEBUG, "%s", pszMessage);
      break;
    case k_ESLAudioLogInfo:
      CLog::Log(LOGINFO, "%s", pszMessage);
      break;
    case k_ESLAudioLogWarning:
      CLog::Log(LOGWARNING, "%s", pszMessage);
      break;
    case k_ESLAudioLogError:
      CLog::Log(LOGERROR, "%s", pszMessage);
      break;
    default:
      break;
    }
  }
}

CAESinkSteamLink::CAESinkSteamLink() :
  m_lastPackageStamp(0.0),
  m_delaySec(0.0),
  m_context(nullptr),
  m_stream(nullptr)
{
  SLAudio_SetLogLevel(k_ESLAudioLogDebug);
  SLAudio_SetLogFunction(LogFunction, nullptr);
}

CAESinkSteamLink::~CAESinkSteamLink()
{
  Deinitialize();
  SLAudio_SetLogFunction(nullptr, nullptr);
}

bool CAESinkSteamLink::Initialize(AEAudioFormat &format, std::string &device)
{
  Deinitialize();

  m_initialAttenuation = true;
  m_startTime = (double)CurrentHostCounter() / CurrentHostFrequency();
  
  format.m_dataFormat    = AE_FMT_S16NE;
  format.m_sampleRate    = SL_SAMPLE_RATE;
  format.m_frames        = format.m_sampleRate * SINK_FEED_MS / 1000;
  format.m_frameSize     = format.m_channelLayout.Count() * (CAEUtil::DataFormatToBits(format.m_dataFormat) >> 3);
  m_format = format;

  CSLAudioContext* context = SLAudio_CreateContext();
  if (context)
  {
    CSLAudioStream* stream = SLAudio_CreateStream(context, m_format.m_sampleRate, m_format.m_channelLayout.Count(), format.m_frames * format.m_frameSize, false);
    if (stream)
    {
      // Success
      m_context = context;
      m_stream = stream;
    }
    else
    {
      CLog::Log(LOGERROR, "SteamLinkAudio: Failed to create stream");
      SLAudio_FreeContext(context);
    }
  }
  else
  {
    CLog::Log(LOGERROR, "SteamLinkAudio: Failed to create context");
  }

  return m_context != nullptr;
}

void CAESinkSteamLink::Deinitialize()
{
  while (m_AudioQueue.size() > 0)
  {
    delete[] m_AudioQueue.front().m_pData;
    m_AudioQueue.pop();
  }
  if (m_stream)
  {
    SLAudio_FreeStream(m_stream);
    m_stream = nullptr;
  }
  if (m_context)
  {
    SLAudio_FreeContext(m_context);
    m_context = nullptr;
  }
}

double CAESinkSteamLink::GetCacheTotal()
{
  return CACHE_TOTAL_MS / 1000.0;
}

unsigned int CAESinkSteamLink::AddPackets(uint8_t **data, unsigned int frames, unsigned int offset)
{
  const double packetTimeSecs = 1.0 * (frames - offset) / m_format.m_sampleRate;

  // Update delay for elapsed time
  const double nowSecs = (double)CurrentHostCounter() / CurrentHostFrequency();
  if (m_lastPackageStamp > 0.0)
  {
    const double elapsedSecs = nowSecs - m_lastPackageStamp;
    m_delaySec -= elapsedSecs;
    if (m_delaySec < 0.0)
      m_delaySec = 0.0;
  }
  m_lastPackageStamp = nowSecs;

  // Ensure space in the buffer
  const double availableSecs = GetCacheTotal() - GetDelaySecs();

  const int sleepTimeUs = (int)((SINK_FEED_MS / 1000.0 - availableSecs) * 1000 * 1000);

  if (sleepTimeUs > 0)
    usleep(sleepTimeUs);

  CAudioChunk chunk;
  chunk.m_nSize = (frames - offset) * m_format.m_frameSize;
  chunk.m_pData = new uint8_t[chunk.m_nSize];
  std::memcpy(chunk.m_pData, data[0] + offset * m_format.m_frameSize, chunk.m_nSize);
  if (m_initialAttenuation)
  {
    double flVolume = (nowSecs - m_startTime) / INITIAL_ATTENUATION_TIME;
    flVolume *= flVolume;
    if (flVolume < 1.0)
      AttenuateChunk(&chunk, flVolume);
    else
      m_initialAttenuation = false;
  }
  chunk.m_nNowSecs = nowSecs + g_unQueuedVideoMS / 1000.0 + DELAY_OFFSET_SECONDS;
  m_AudioQueue.push( chunk );

  while (m_AudioQueue.size() > 0)
  {
    chunk = m_AudioQueue.front();
    if (chunk.m_nNowSecs > nowSecs + 0.001)
    {
      break;
    }

    m_AudioQueue.pop();

    void* buffer = SLAudio_BeginFrame(m_stream);
    std::memcpy(buffer, chunk.m_pData, chunk.m_nSize);
    SLAudio_SubmitFrame(m_stream);
    delete[] chunk.m_pData;
  }

  // Increase the delay for the added packet
  m_delaySec += packetTimeSecs;

  return frames - offset;
}

void CAESinkSteamLink::GetDelay(AEDelayStatus &status)
{
  status.SetDelay(GetDelaySecs());
}

void CAESinkSteamLink::AttenuateChunk(CAudioChunk *pChunk, double flVolume)
{
  int16_t *pData = (int16_t*)pChunk->m_pData;
  int nCount = pChunk->m_nSize / sizeof(*pData);
  while (nCount--)
  {
    *pData = static_cast<int16_t>(*pData * flVolume);
    ++pData;
  }
}

void CAESinkSteamLink::Drain()
{
  unsigned int usecs = (unsigned int)(GetDelaySecs() * 1000 * 1000);
  if (usecs > 0)
    usleep(usecs);

  m_lastPackageStamp = 0.0;
  m_delaySec = 0.0;

  while (m_AudioQueue.size() > 0)
  {
    delete[] m_AudioQueue.front().m_pData;
    m_AudioQueue.pop();
  }
  g_unQueuedVideoMS = 0;
}

double CAESinkSteamLink::GetDelaySecs()
{
  return m_delaySec;
}

void CAESinkSteamLink::EnumerateDevicesEx(AEDeviceInfoList &deviceInfoList)
{
  CAEDeviceInfo info;

  info.m_deviceType = AE_DEVTYPE_PCM;
  info.m_deviceName = "SteamLink";
  info.m_displayName = "Steam Link Low Latency Audio";
  info.m_displayNameExtra = "";
  info.m_channels += AE_CH_FL;
  info.m_channels += AE_CH_FR;
  info.m_sampleRates.push_back(SL_SAMPLE_RATE);
  info.m_dataFormats.push_back(AE_FMT_S16NE);

  deviceInfoList.push_back(info);
}
