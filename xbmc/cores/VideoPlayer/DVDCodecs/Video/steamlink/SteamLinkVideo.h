/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "SteamLinkUniqueBuffer.h"
#include "cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "threads/CriticalSection.h"

#include <memory>
#include <stdint.h>
#include <string>

struct CSLVideoContext;
struct CSLVideoStream;

namespace KODI
{
namespace STEAMLINK
{

class CSteamLinkVideoBufferPool;
class CSteamLinkVideoStream;

class CSteamLinkVideo : public CDVDVideoCodec
{
public:
  CSteamLinkVideo(CProcessInfo& processInfo);
  virtual ~CSteamLinkVideo();

  // Registration
  static CDVDVideoCodec* Create(CProcessInfo& processInfo);
  static void Register();

  // Implementation of CDVDVideoCodec
  bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options) override;
  bool AddData(const DemuxPacket& packet) override;
  void Reset() override;
  CDVDVideoCodec::VCReturn GetPicture(VideoPicture* pVideoPicture) override;
  const char* GetName() override;

  // Access global stream instance
  static bool IsPlayingVideo();
  static unsigned int GetDelayMs();

private:
  void Dispose();

  std::shared_ptr<CSteamLinkVideoStream> GetStream();

  // Steam Link data
  CSLVideoContext* m_context = nullptr;
  std::shared_ptr<CSteamLinkVideoStream> m_stream;
  CCriticalSection m_streamLock;

  // VideoPlayer data
  std::shared_ptr<CSteamLinkVideoBufferPool> m_videoBufferPool;
  VideoPicture m_videoPictureBuffer;
  SteamLinkUniqueBuffer m_buffer;
  size_t m_bufferSize = 0;
  double m_dts = 0.0;

  // Bitstream to bytestream (Annex B) conversion support
  bool bitstream_convert_init(void *in_extradata, int in_extrasize);
  bool bitstream_convert(uint8_t* pData, int iSize, uint8_t **poutbuf, int *poutbuf_size);
  static void bitstream_alloc_and_copy(uint8_t **poutbuf, int *poutbuf_size,
    const uint8_t *sps_pps, uint32_t sps_pps_size, const uint8_t *in, uint32_t in_size);

  struct bitstream_ctx
  {
    uint8_t length_size = 0;
    uint8_t first_idr = 0;
    uint8_t *sps_pps_data = nullptr;
    uint32_t size = 0;
  };

  bitstream_ctx m_sps_pps_context{};
  uint32_t m_sps_pps_size = 0;
  bool m_convert_bitstream = false;

  // Global instance
  static CSteamLinkVideo* m_globalVideo;
  static unsigned int m_globalInstances;
  static CCriticalSection m_globalLock;
};

}
}
