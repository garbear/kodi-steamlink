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
#include "AESinkSteamLinkStream.h"
#include "AESinkSteamLinkTranslator.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

// Steam Link audio API
#include "SLAudio.h"

#include <cstring>
#include <unistd.h>

#define SL_SAMPLE_RATE                 48000
#define SINK_FEED_MS                   50 // Steam Link game streaming uses 10ms
#define CACHE_TOTAL_MS                 200
#define INITIAL_ATTENUATION_TIME_SECS  6.0

using namespace STEAMLINK;

namespace
{
  void LogFunction(void *pContext, ESLAudioLog eLogLevel, const char *pszMessage)
  {
    int level = CAESinkSteamLinkTranslator::TranslateLogLevel(eLogLevel);
    CLog::Log(level, "%s", pszMessage);
  }
}

CAESinkSteamLink::CAESinkSteamLink() :
  m_context(nullptr),
  m_startTimeSecs(0.0),
  m_framesSinceStart(0)
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
  bool bSuccess = false;

  Deinitialize();

  m_startTimeSecs = 0.0; // Set in first call to AddPackets()
  m_framesSinceStart = 0;
  
  format.m_dataFormat    = AE_FMT_S16NE;
  format.m_sampleRate    = SL_SAMPLE_RATE;
  format.m_frames        = format.m_sampleRate * SINK_FEED_MS / 1000;
  format.m_frameSize     = format.m_channelLayout.Count() * (CAEUtil::DataFormatToBits(format.m_dataFormat) >> 3);

  if (format.m_channelLayout.Count() == 0 || format.m_frameSize == 0)
    return false;

  CSLAudioContext* context = SLAudio_CreateContext();
  if (!context)
  {
    CLog::Log(LOGERROR, "SteamLinkAudio: Failed to create context");
  }
  else
  {
    std::unique_ptr<CAESinkSteamLinkStream> stream(new CAESinkSteamLinkStream(context, format.m_sampleRate, format.m_channelLayout.Count(), format.m_frames * format.m_frameSize));
    if (stream->Open())
    {
      m_format = format;
      m_context = context;
      m_stream = std::move(stream);
      bSuccess = true;
    }
    else
    {
      SLAudio_FreeContext(context);
    }
  }

  return bSuccess;
}

void CAESinkSteamLink::Deinitialize()
{
  m_stream.reset();

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
  if (offset >= frames)
    return 0;

  if (!m_stream)
    return 0;

  // Calculate frame count from given parameters
  const unsigned int frameCount = frames - offset;

  // Calculate start time and present time
  const double nowSecs = static_cast<double>(CurrentHostCounter()) / CurrentHostFrequency();

  if (m_startTimeSecs == 0.0)
    m_startTimeSecs = nowSecs;

  double presentTimeSecs = m_startTimeSecs + static_cast<double>(m_framesSinceStart) / m_format.m_sampleRate;

  // Detect underrun
  if (presentTimeSecs < nowSecs)
  {
    CLog::Log(LOGDEBUG, "SteamLinkAudio: Buffer underrun detected");
    presentTimeSecs = m_startTimeSecs = nowSecs;
    m_framesSinceStart = 0;
  }

  // Ensure space in the buffer
  const double delaySecs = presentTimeSecs - nowSecs;

  const double availableSecs = GetCacheTotal() - delaySecs;

  const int sleepTimeUs = static_cast<int>((SINK_FEED_MS - availableSecs * 1000.0) * 1000);

  if (sleepTimeUs > 0)
    usleep(sleepTimeUs);

  // Create buffer and copy data
  const size_t packetSize = frameCount * m_format.m_frameSize;
  std::unique_ptr<uint8_t[]> buffer(new uint8_t[packetSize]);

  std::memcpy(buffer.get(), *data + offset * m_format.m_frameSize, packetSize);

  // Attenuate if necessary
  const double elapsedSinceStartSecs = (presentTimeSecs - m_startTimeSecs);
  const bool bAttenuate = (elapsedSinceStartSecs < INITIAL_ATTENUATION_TIME_SECS);
  if (bAttenuate)
  {
    double flVolume = elapsedSinceStartSecs / INITIAL_ATTENUATION_TIME_SECS;
    AttenuateChunk(buffer.get(), packetSize, flVolume * flVolume);
  }

  // Add packet
  if (!m_stream->AddPacket(std::move(buffer), packetSize, presentTimeSecs))
    return 0;

  m_framesSinceStart += frameCount;

  return frameCount;
}

void CAESinkSteamLink::GetDelay(AEDelayStatus &status)
{
  double delaySecs = 0.0;

  if (m_startTimeSecs != 0.0)
  {
    const double nowSecs = static_cast<double>(CurrentHostCounter()) / CurrentHostFrequency();

    double nextPresentTimeSecs = m_startTimeSecs + static_cast<double>(m_framesSinceStart) / m_format.m_sampleRate;

    if (nextPresentTimeSecs > nowSecs)
      delaySecs = nextPresentTimeSecs - nowSecs;
  }

  status.SetDelay(delaySecs);
}

void CAESinkSteamLink::Drain()
{
  if (m_stream)
  {
    if (!m_stream->Flush())
      m_stream.reset();
  }

  m_startTimeSecs = 0.0;
  m_framesSinceStart = 0;
}

void CAESinkSteamLink::AttenuateChunk(uint8_t *pChunk, unsigned int size, double flVolume)
{
  int16_t *pData = reinterpret_cast<int16_t*>(pChunk);
  int nCount = size / sizeof(*pData);
  while (nCount--)
  {
    *pData = static_cast<int16_t>(*pData * flVolume);
    ++pData;
  }
}

void CAESinkSteamLink::EnumerateDevicesEx(AEDeviceInfoList &deviceInfoList, bool force /* = false */)
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
