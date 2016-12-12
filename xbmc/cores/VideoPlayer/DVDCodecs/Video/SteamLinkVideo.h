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
#pragma once

#include "DVDVideoCodec.h"
#include "SteamLinkVideoBuffer.h"
#include "threads/CriticalSection.h"

#include <memory>
#include <stdint.h>
#include <string>

#define STEAMLINK_VIDEO_CODEC_NAME  "SteamLinkVideo"

struct CSLVideoContext;
struct CSLVideoStream;

namespace STEAMLINK
{

class CSteamLinkVideoStream;

class CSteamLinkVideo : public CDVDVideoCodec
{
public:
  CSteamLinkVideo(CProcessInfo& processInfo);
  virtual ~CSteamLinkVideo();

  // implementation of CDVDVideoCodec
  virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options) override;
  virtual int Decode(uint8_t* pData, int iSize, double dts, double pts) override;
  virtual void Reset() override;
  virtual bool GetPicture(DVDVideoPicture* pDvdVideoPicture) override;
  virtual bool ClearPicture(DVDVideoPicture* pDvdVideoPicture) override;
  virtual void SetDropState(bool bDrop) override { }
  virtual const char* GetName() override { return STEAMLINK_VIDEO_CODEC_NAME; }

  // Access global stream instance
  static bool IsPlayingVideo();
  static unsigned int GetDelayMs();

private:
  void Dispose();

  std::shared_ptr<CSteamLinkVideoStream> GetStream();

  // Steam Link data
  CSLVideoContext* m_context;
  std::shared_ptr<CSteamLinkVideoStream> m_stream;
  CCriticalSection m_streamLock;

  // VideoPlayer data
  DVDVideoPicture m_videoPictureBuffer;
  SteamLinkUniqueBuffer m_buffer;
  size_t m_bufferSize;
  float m_dts;

  // bitstream to bytestream (Annex B) conversion support.
  bool bitstream_convert_init(void *in_extradata, int in_extrasize);
  bool bitstream_convert(uint8_t* pData, int iSize, uint8_t **poutbuf, int *poutbuf_size);
  static void bitstream_alloc_and_copy(uint8_t **poutbuf, int *poutbuf_size,
    const uint8_t *sps_pps, uint32_t sps_pps_size, const uint8_t *in, uint32_t in_size);

  typedef struct bitstream_ctx
  {
      uint8_t  length_size;
      uint8_t  first_idr;
      uint8_t *sps_pps_data;
      uint32_t size;

      bitstream_ctx()
      {
        length_size = 0;
        first_idr = 0;
        sps_pps_data = NULL;
        size = 0;
      }

  } bitstream_ctx;

  uint32_t      m_sps_pps_size;
  bitstream_ctx m_sps_pps_context;
  bool          m_convert_bitstream;

  // Global instance
  static CSteamLinkVideo* m_globalVideo;
  static unsigned int m_globalInstances;
  static CCriticalSection m_globalLock;
};

}
