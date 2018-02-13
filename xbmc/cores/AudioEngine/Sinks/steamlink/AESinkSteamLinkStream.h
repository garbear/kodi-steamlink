/*
 *  Copyright (C) 2018 Team Kodi
 *  Copyright (C) 2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "interfaces/IAnnouncer.h"
#include "threads/CriticalSection.h"
#include "threads/SystemClock.h"
#include "threads/Thread.h"

#include <memory>
#include <deque>
#include <stdint.h>

struct CSLAudioContext;
struct CSLAudioStream;

namespace KODI
{
namespace STEAMLINK
{

class CAESinkSteamLinkStream : public CThread,
                               public ANNOUNCEMENT::IAnnouncer
{
public:
  CAESinkSteamLinkStream(CSLAudioContext* context, unsigned int sampleRateHz, unsigned int channels, unsigned int packetSize);
  virtual ~CAESinkSteamLinkStream();

  bool Open();
  void Close();
  bool Flush();
  bool AddPacket(std::unique_ptr<uint8_t[]> data, unsigned int size, double presentTimeSecs);

  // implementation of IAnnouncer
  virtual void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data) override;

protected:
  // implementation of CThread
  virtual void Process() override;

private:
  struct AudioPacket
  {
    AudioPacket() :
      size(0),
      presentTimeSecs(0.0)
    {
    }

    AudioPacket(std::unique_ptr<uint8_t[]> buffer, unsigned int size, double presentTimeSecs) :
      buffer(std::move(buffer)),
      size(size),
      presentTimeSecs(presentTimeSecs)
    {
    }

    std::unique_ptr<uint8_t[]> buffer;
    unsigned int size;
    double presentTimeSecs;
  };

  bool GetNextPacket(AudioPacket& packet);

  void WaitUntilReady(double targetTimeSecs);

  bool HasLatePacket(double nextPresentTimeSecs) const;

  void ClearLatePackets(double nextPresentTimeSecs);

  void BeginPacket();
  void EndPacket();

  void SendPacket(AudioPacket packet);

  double GetSLDelaySecs();

  // Construction parameters
  CSLAudioContext* const m_context;
  const unsigned int m_sampleRateHz;
  const unsigned int m_channels;
  const unsigned int m_packetSize;

  // Stream parameters
  CSLAudioStream* m_stream;
  CCriticalSection m_streamMutex;
  std::deque<AudioPacket> m_queue;
  CCriticalSection m_queueMutex;
  CEvent m_queueEvent;
  uint8_t* m_steamLinkBuffer;
  unsigned int m_remainingBytes; // Bytes remaining in the Steam Link audio buffer
  XbmcThreads::EndTime m_videoDelay;
};

}
}
