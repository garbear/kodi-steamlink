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

#include "interfaces/IAnnouncer.h"
#include "threads/CriticalSection.h"
#include "threads/SystemClock.h"
#include "threads/Thread.h"

#include <memory>
#include <deque>
#include <stdint.h>

struct CSLAudioContext;
struct CSLAudioStream;

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
