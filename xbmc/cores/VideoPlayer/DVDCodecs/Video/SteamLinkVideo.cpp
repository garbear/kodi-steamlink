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

#include "SteamLinkVideo.h"
#include "SteamLinkTranslator.h"
#include "SteamLinkVideoStream.h"
#include "cores/VideoPlayer/DVDClock.h"
#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFormats.h"
#include "settings/AdvancedSettings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

// Steam Link video API
#include "SLVideo.h"

#include <string.h>
#include <utility>
#include "SteamLinkTranslator.h"

using namespace STEAMLINK;

namespace
{
  void LogFunction(void *pContext, ESLVideoLog eLogLevel, const char *pszMessage)
  {
    int level = CSteamLinkTranslator::TranslateLogLevel(eLogLevel);
    CLog::Log(level, "%s", pszMessage);
  }
}

CSteamLinkVideo* CSteamLinkVideo::m_globalVideo = nullptr;
unsigned int CSteamLinkVideo::m_globalInstances = 0;
CCriticalSection CSteamLinkVideo::m_globalLock;

CSteamLinkVideo::CSteamLinkVideo(CProcessInfo& processInfo) :
  CDVDVideoCodec(processInfo),
  m_context(nullptr),
  m_stream(nullptr),
  m_bufferSize(0),
  m_dts(0.0),
  m_sps_pps_size(0),
  m_convert_bitstream(false)
{
  // Set global parameters
  {
    CSingleLock lock(m_globalLock);

    if (m_globalVideo == nullptr)
      m_globalVideo = this;
    else
      CLog::Log(LOGERROR, "%s: Already a global video instance!!!", GetName());

    if (m_globalInstances++ == 0)
    {
      SLVideo_SetLogLevel(g_advancedSettings.CanLogComponent(LOGVIDEO) ? k_ESLVideoLogDebug : k_ESLVideoLogError);
      SLVideo_SetLogFunction(LogFunction, nullptr);
    }
  }
}

CSteamLinkVideo::~CSteamLinkVideo()
{
  Dispose();

  // Unset global parameters
  {
    CSingleLock lock(m_globalLock);

    if (m_globalVideo == this)
      m_globalVideo = nullptr;

    if (m_globalInstances > 0 && --m_globalInstances == 0)
      SLVideo_SetLogFunction(nullptr, nullptr);
  }
}

bool CSteamLinkVideo::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  if (hints.software)
    return false;

  bool bSuccess = false;

  Dispose();

  CSLVideoContext* context = SLVideo_CreateContext();
  if (!context)
  {
    CLog::Log(LOGERROR, "%s: Failed to create context", GetName());
  }
  else
  {
    std::unique_ptr<CSteamLinkVideoStream> stream;

    ESLVideoFormat slFormat;
    if (!CSteamLinkTranslator::TranslateFormat(hints.codec, slFormat))
    {
      if (g_advancedSettings.CanLogComponent(LOGVIDEO))
        CLog::Log(LOGDEBUG, "%s: Codec not supported", GetName());
    }
    else
    {
      if (hints.extrasize < 7 || hints.extradata == nullptr)
      {
        if (g_advancedSettings.CanLogComponent(LOGVIDEO))
          CLog::Log(LOGNOTICE, "%s: avcC data too small or missing", GetName());
      }
      else
      {
        // valid avcC data (bitstream) always starts with the value 1 (version)
        if (*static_cast<char*>(hints.extradata) == 1)
          m_convert_bitstream = bitstream_convert_init(hints.extradata, hints.extrasize);

        if (hints.fpsrate > 0 && hints.fpsscale > 0)
          stream.reset(new CSteamLinkVideoStream(context, k_ESLVideoFormatH264, hints.fpsrate, hints.fpsscale));
        else
          stream.reset(new CSteamLinkVideoStream(context, k_ESLVideoFormatH264));

        if (!stream->Open())
          stream.reset();
      }
    }

    if (!stream)
    {
      if (g_advancedSettings.CanLogComponent(LOGVIDEO))
        CLog::Log(LOGNOTICE, "%s: Failed to create stream", GetName());
      SLVideo_FreeContext(context);
    }
    else
    {
      // Success
      m_context = context;
      {
        CSingleLock lock(m_streamLock);
        m_stream = std::move(stream);
      }
      bSuccess = true;

      // Initialize buffer for DVDVideoPicture
      memset(&m_videoPictureBuffer, 0, sizeof(m_videoPictureBuffer));
      m_videoPictureBuffer.dts = DVD_NOPTS_VALUE;
      m_videoPictureBuffer.pts = DVD_NOPTS_VALUE;
      m_videoPictureBuffer.format = RENDER_FMT_STEAMLINK;
      m_videoPictureBuffer.color_range = 0;
      m_videoPictureBuffer.color_matrix = 4; // FCC
      m_videoPictureBuffer.iFlags  = DVP_FLAG_ALLOCATED;
      m_videoPictureBuffer.iWidth  = hints.width;
      m_videoPictureBuffer.iHeight = hints.height;
      m_videoPictureBuffer.steamlink = nullptr;

      m_videoPictureBuffer.iDisplayWidth  = m_videoPictureBuffer.iWidth;
      m_videoPictureBuffer.iDisplayHeight = m_videoPictureBuffer.iHeight;

      m_processInfo.SetVideoDecoderName(GetName(), true);
      m_processInfo.SetVideoPixelFormat(CSteamLinkTranslator::TranslateFormatToString(slFormat));
      m_processInfo.SetVideoDimensions(hints.width, hints.height);
      m_processInfo.SetVideoFps(static_cast<float>(hints.fpsrate) / static_cast<float>(hints.fpsscale));
      m_processInfo.SetVideoDAR(hints.aspect);

      if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      {
        CLog::Log(LOGINFO, "%s: Opened codec %s", GetName(), CSteamLinkTranslator::TranslateFormatToString(slFormat));

        int width = 0;
        int height = 0;
        SLVideo_GetDisplayResolution(m_context, &width, &height);

        CLog::Log(LOGDEBUG, "%s: Display resolution = %d x %d", GetName(), width, height);
      }
    }
  }

  return bSuccess;
}

void CSteamLinkVideo::Dispose()
{
  if (m_stream)
  {
    m_stream->Close();
    {
      CSingleLock lock(m_streamLock);
      m_stream.reset();
    }
  }

  if (m_context)
  {
    SLVideo_FreeContext(m_context);
    m_context = nullptr;
  }

  if (m_convert_bitstream)
  {
    if (m_sps_pps_context.sps_pps_data)
    {
      free(m_sps_pps_context.sps_pps_data);
      m_sps_pps_context.sps_pps_data = NULL;
    }
    m_convert_bitstream = false;
  }
}

int CSteamLinkVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
  if (pData == nullptr || iSize <= 0)
    return VC_BUFFER;

  if (!m_stream)
    return VC_ERROR;

  m_buffer.reset();
  m_bufferSize = 0;
  m_dts = dts;

  if (m_convert_bitstream)
  {
    // convert demuxer packet from bitstream to bytestream (AnnexB)
    uint8_t *bytestream_buff = nullptr;
    int bytestream_size = 0;

    bitstream_convert(pData, iSize, &bytestream_buff, &bytestream_size);
    if (bytestream_buff != nullptr && (bytestream_size > 0))
    {
      m_buffer.reset(bytestream_buff);
      m_bufferSize = bytestream_size;
    }
  }
  else
  {
    uint8_t *bytestream_buff = static_cast<uint8_t*>(malloc(iSize));
    if (bytestream_buff)
    {
      memcpy(bytestream_buff, pData, iSize);
      m_buffer.reset(bytestream_buff);
      m_bufferSize = iSize;
    }
  }

  return VC_PICTURE;
}

void CSteamLinkVideo::Reset(void)
{
  if (!m_stream)
    return;

  if (!m_stream->Flush())
  {
    CSingleLock lock(m_streamLock);
    m_stream.reset();
    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGERROR, "%s: Failed to flush stream", GetName());
  }
}

bool CSteamLinkVideo::GetPicture(DVDVideoPicture *pDvdVideoPicture)
{
  if (!m_stream)
    return false;

  *pDvdVideoPicture = m_videoPictureBuffer;

  // Steam link video accepts decode packet, so use dts for timing
  pDvdVideoPicture->pts = m_dts;
  pDvdVideoPicture->steamlink = new CSteamLinkBuffer(std::move(m_buffer), m_bufferSize, m_stream);

  return true;
}

bool CSteamLinkVideo::ClearPicture(DVDVideoPicture* pDvdVideoPicture)
{
  delete pDvdVideoPicture->steamlink;
  pDvdVideoPicture->steamlink = nullptr;

  return true;
}

std::shared_ptr<CSteamLinkVideoStream> CSteamLinkVideo::GetStream()
{
  std::shared_ptr<CSteamLinkVideoStream> stream;

  {
    CSingleLock lock(m_streamLock);
    stream = m_stream;
  }

  return stream;
}

bool CSteamLinkVideo::IsPlayingVideo()
{
  std::shared_ptr<CSteamLinkVideoStream> stream;

  {
    CSingleLock lock(m_globalLock);

    if (m_globalVideo != nullptr)
      stream = m_globalVideo->GetStream();
  }

  return stream.get() != nullptr;
}

unsigned int CSteamLinkVideo::GetDelayMs()
{
  std::shared_ptr<CSteamLinkVideoStream> stream;

  {
    CSingleLock lock(m_globalLock);

    if (m_globalVideo != nullptr)
      stream = m_globalVideo->GetStream();
  }

  return stream ? stream->GetDelayMs() : 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
bool CSteamLinkVideo::bitstream_convert_init(void *in_extradata, int in_extrasize)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  m_sps_pps_size = 0;
  m_sps_pps_context.sps_pps_data = NULL;
  
  // nothing to filter
  if (!in_extradata || in_extrasize < 6)
    return false;

  uint16_t unit_size;
  uint32_t total_size = 0;
  uint8_t *out = NULL, unit_nb, sps_done = 0;
  const uint8_t *extradata = (uint8_t*)in_extradata + 4;
  static const uint8_t nalu_header[4] = {0, 0, 0, 1};

  // retrieve length coded size
  m_sps_pps_context.length_size = (*extradata++ & 0x3) + 1;
  if (m_sps_pps_context.length_size == 3)
    return false;

  // retrieve sps and pps unit(s)
  unit_nb = *extradata++ & 0x1f;  // number of sps unit(s)
  if (!unit_nb)
  {
    unit_nb = *extradata++;       // number of pps unit(s)
    sps_done++;
  }
  while (unit_nb--)
  {
    unit_size = extradata[0] << 8 | extradata[1];
    total_size += unit_size + 4;
    if ( (extradata + 2 + unit_size) > ((uint8_t*)in_extradata + in_extrasize) )
    {
      free(out);
      return false;
    }
    uint8_t* new_out = (uint8_t*)realloc(out, total_size);
    if (new_out)
    {
      out = new_out;
    }
    else
    {
      CLog::Log(LOGERROR, "bitstream_convert_init failed - %s : could not realloc the buffer out",  __FUNCTION__);
      free(out);
      return false;
    }

    memcpy(out + total_size - unit_size - 4, nalu_header, 4);
    memcpy(out + total_size - unit_size, extradata + 2, unit_size);
    extradata += 2 + unit_size;

    if (!unit_nb && !sps_done++)
      unit_nb = *extradata++;     // number of pps unit(s)
  }

  m_sps_pps_context.sps_pps_data = out;
  m_sps_pps_context.size = total_size;
  m_sps_pps_context.first_idr = 1;

  return true;
}

bool CSteamLinkVideo::bitstream_convert(uint8_t* pData, int iSize, uint8_t **poutbuf, int *poutbuf_size)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  uint8_t *buf = pData;
  uint32_t buf_size = iSize;
  uint8_t  unit_type;
  int32_t  nal_size;
  uint32_t cumul_size = 0;
  const uint8_t *buf_end = buf + buf_size;

  do
  {
    if (buf + m_sps_pps_context.length_size > buf_end)
      goto fail;

    if (m_sps_pps_context.length_size == 1)
      nal_size = buf[0];
    else if (m_sps_pps_context.length_size == 2)
      nal_size = buf[0] << 8 | buf[1];
    else
      nal_size = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

    buf += m_sps_pps_context.length_size;
    unit_type = *buf & 0x1f;

    if (buf + nal_size > buf_end || nal_size < 0)
      goto fail;

    // prepend only to the first type 5 NAL unit of an IDR picture
    if (m_sps_pps_context.first_idr && unit_type == 5)
    {
      bitstream_alloc_and_copy(poutbuf, poutbuf_size,
        m_sps_pps_context.sps_pps_data, m_sps_pps_context.size, buf, nal_size);
      m_sps_pps_context.first_idr = 0;
    }
    else
    {
      bitstream_alloc_and_copy(poutbuf, poutbuf_size, NULL, 0, buf, nal_size);
      if (!m_sps_pps_context.first_idr && unit_type == 1)
          m_sps_pps_context.first_idr = 1;
    }

    if (!*poutbuf)
      goto fail;

    buf += nal_size;
    cumul_size += nal_size + m_sps_pps_context.length_size;
  } while (cumul_size < buf_size);

  return true;

fail:
  free(*poutbuf);
  *poutbuf = NULL;
  *poutbuf_size = 0;
  return false;
}

void CSteamLinkVideo::bitstream_alloc_and_copy(
  uint8_t **poutbuf,      int *poutbuf_size,
  const uint8_t *sps_pps, uint32_t sps_pps_size,
  const uint8_t *in,      uint32_t in_size)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  #define CHD_WB32(p, d) { \
    ((uint8_t*)(p))[3] = (d); \
    ((uint8_t*)(p))[2] = (d) >> 8; \
    ((uint8_t*)(p))[1] = (d) >> 16; \
    ((uint8_t*)(p))[0] = (d) >> 24; }

  uint32_t offset = *poutbuf_size;
  uint8_t nal_header_size = 4;//offset ? 3 : 4;

  *poutbuf_size += sps_pps_size + nal_header_size + in_size;
  *poutbuf = static_cast<uint8_t*>(realloc(*poutbuf, *poutbuf_size));

  if (!*poutbuf)
    return;

  if (sps_pps)
    memcpy(*poutbuf + offset, sps_pps, sps_pps_size);

  memcpy(*poutbuf + offset + sps_pps_size + nal_header_size, in, in_size);
  if (true || !offset)
  {
    CHD_WB32(*poutbuf + offset + sps_pps_size, 1);
  }
  else
  {
    (*poutbuf + offset + sps_pps_size)[0] = 0;
    (*poutbuf + offset + sps_pps_size)[1] = 0;
    (*poutbuf + offset + sps_pps_size)[2] = 1;
  }
}
