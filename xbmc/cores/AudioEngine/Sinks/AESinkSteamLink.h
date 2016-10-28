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
#pragma once

#include "cores/AudioEngine/Interfaces/AESink.h"
#include "cores/AudioEngine/Utils/AEDeviceInfo.h"

#include <stdint.h>
#include <queue>

#define STEAM_LINK_SINK_NAME  "SteamLinkAudio"

struct CSLAudioContext;
struct CSLAudioStream;

namespace STEAMLINK
{

class CAESinkSteamLink : public IAESink
{
public:
  virtual const char* GetName() override { return STEAM_LINK_SINK_NAME; }

  CAESinkSteamLink();
  virtual ~CAESinkSteamLink();

  // implementation of IAESink
  virtual bool Initialize(AEAudioFormat &format, std::string &device) override;
  virtual void Deinitialize() override;
  virtual double GetCacheTotal() override;
  virtual unsigned int AddPackets(uint8_t **data, unsigned int frames, unsigned int offset) override;
  virtual void GetDelay(AEDelayStatus &status) override;
  virtual void Drain() override;

  static void EnumerateDevicesEx(AEDeviceInfoList &deviceInfoList, bool force = false);

private:
  double GetDelaySecs();

  // AE stuff
  AEAudioFormat m_format;
  double        m_lastPackageStamp;
  double        m_delaySec; // Estimated delay in seconds

  // Steam Link stuff
  CSLAudioContext *m_context;
  CSLAudioStream *m_stream;

  // Queued audio for video sync
  struct CAudioChunk
  {
    uint8_t *m_pData;
    unsigned int m_nSize;
    double m_nNowSecs;
  };
  std::queue<CAudioChunk> m_AudioQueue;

  // Initial attenuation to cover audio sync
  double m_startTime;
  bool m_initialAttenuation;
  void AttenuateChunk(CAudioChunk *pChunk, double flVolume);
};

}
