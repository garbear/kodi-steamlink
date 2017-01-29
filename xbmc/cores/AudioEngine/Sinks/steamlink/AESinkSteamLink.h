/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/AudioEngine/Interfaces/AESink.h"
#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Utils/AEDeviceInfo.h"

#include <memory>
#include <stdint.h>

struct CSLAudioContext;

namespace KODI
{
namespace STEAMLINK
{
class CAESinkSteamLinkStream;

class CAESinkSteamLink : public IAESink
{
public:

  CAESinkSteamLink();
  ~CAESinkSteamLink() override;

  static void Register();
  static IAESink* Create(std::string &device, AEAudioFormat &desiredFormat);

  // implementation of IAESink
  virtual const char* GetName() override;
  virtual bool Initialize(AEAudioFormat &format, std::string &device) override;
  virtual void Deinitialize() override;
  virtual double GetCacheTotal() override;
  virtual unsigned int AddPackets(uint8_t **data, unsigned int frames, unsigned int offset) override;
  virtual void GetDelay(AEDelayStatus &status) override;
  virtual void Drain() override;

  static void EnumerateDevicesEx(AEDeviceInfoList &deviceInfoList, bool force = false);

private:
  void AttenuateChunk(uint8_t *pChunk, unsigned int size, double flVolume);

  // Steam Link stuff
  CSLAudioContext *m_context;
  std::unique_ptr<CAESinkSteamLinkStream> m_stream;

  // AE stuff
  AEAudioFormat m_format;
  double m_startTimeSecs;
  uint64_t m_framesSinceStart;
};

}
}
