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
#include "cores/dvdplayer/DVDClock.h"
#include "cores/dvdplayer/DVDStreamInfo.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

// Steam Link video API
#include "SLVideo.h"

#include <string.h>

using namespace STEAMLINK;

namespace STEAMLINK
{
  uint32_t g_unQueuedVideoMS = 0;
}

namespace
{
  void LogFunction(void *pContext, ESLVideoLog eLogLevel, const char *pszMessage)
  {
    switch (eLogLevel)
    {
    case k_ESLVideoLogDebug:
      CLog::Log(LOGDEBUG, "%s", pszMessage);
      break;
    case k_ESLVideoLogInfo:
      CLog::Log(LOGINFO, "%s", pszMessage);
      break;
    case k_ESLVideoLogWarning:
      CLog::Log(LOGWARNING, "%s", pszMessage);
      break;
    case k_ESLVideoLogError:
      CLog::Log(LOGERROR, "%s", pszMessage);
      break;
    default:
      break;
    }
  }

}

CSteamLinkVideo::CSteamLinkVideo() :
  m_context(nullptr),
  m_stream(nullptr),
  m_convert_bitstream(false)
{
  SLVideo_SetLogLevel(g_advancedSettings.CanLogComponent(LOGVIDEO) ? k_ESLVideoLogDebug : k_ESLVideoLogError);
  SLVideo_SetLogFunction(LogFunction, nullptr);
}

CSteamLinkVideo::~CSteamLinkVideo()
{
  Dispose();
  SLVideo_SetLogFunction(nullptr, nullptr);
}

bool CSteamLinkVideo::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  if (hints.software)
    return false;

  Dispose();

  CSLVideoContext* context = SLVideo_CreateContext();
  if (context)
  {
    CSLVideoStream* stream = nullptr;

    switch (hints.codec)
    {
      case AV_CODEC_ID_H264:
        if (hints.extrasize < 7 || hints.extradata == NULL)
        {
          CLog::Log(LOGNOTICE,
            "%s: avcC data too small or missing", GetName());
          return false;
        }
        // valid avcC data (bitstream) always starts with the value 1 (version)
        if (*(char*)hints.extradata == 1)
          m_convert_bitstream = bitstream_convert_init(hints.extradata, hints.extrasize);

        stream = SLVideo_CreateStream(context, k_ESLVideoFormatH264, false);

        if (stream && hints.fpsscale > 0)
          SLVideo_SetStreamTargetFramerate(stream, hints.fpsrate, hints.fpsscale);
        break;
      default:
        if (g_advancedSettings.CanLogComponent(LOGVIDEO))
          CLog::Log(LOGDEBUG, "%s: Codec not supported", GetName());
        break;
    }

    if (stream)
    {
      // Success
      m_context = context;
      m_stream = stream;

      if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      {
        int width = 0;
        int height = 0;
        SLVideo_GetDisplayResolution(m_context, &width, &height);

        CLog::Log(LOGDEBUG, "%s: Display resolution = %d x %d", GetName(), width, height);
      }
    }
    else
    {
      CLog::Log(LOGERROR, "%s: Failed to create stream", GetName());
      SLVideo_FreeContext(context);
    }
  }
  else
  {
    CLog::Log(LOGERROR, "%s: Failed to create context", GetName());
  }

  return m_context != nullptr;
}

void CSteamLinkVideo::Dispose()
{
  if (m_stream)
  {
    SLVideo_FreeStream(m_stream);
    m_stream = nullptr;
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
  g_unQueuedVideoMS = 0;
}

int CSteamLinkVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
  if (!pData || iSize == 0)
    return VC_BUFFER;

  int demuxer_bytes = iSize;
  uint8_t *demuxer_content = pData;
  bool bitstream_convered  = false;

  if (m_convert_bitstream)
  {
    // convert demuxer packet from bitstream to bytestream (AnnexB)
    int bytestream_size = 0;
    uint8_t *bytestream_buff = NULL;

    bitstream_convert(demuxer_content, demuxer_bytes, &bytestream_buff, &bytestream_size);
    if (bytestream_buff && (bytestream_size > 0))
    {
      bitstream_convered = true;
      demuxer_bytes = bytestream_size;
      demuxer_content = bytestream_buff;
    }
  }

  if (SLVideo_BeginFrame(m_stream, demuxer_bytes) != 0)
  {
    CLog::Log(LOGERROR, "%s: Failed to begin frame", GetName());
    return VC_ERROR;
  }

  if (SLVideo_WriteFrameData(m_stream, demuxer_content, demuxer_bytes) != 0)
  {
    CLog::Log(LOGERROR, "%s: Error writing data", GetName());
    return VC_ERROR;
  }

  if (SLVideo_SubmitFrame(m_stream) != 0)
  {
    CLog::Log(LOGERROR, "%s: Error submitting frame", GetName());
    return VC_ERROR;
  }

  if (bitstream_convered)
    free(demuxer_content);

  g_unQueuedVideoMS = SLVideo_GetQueuedVideoMS(m_stream);

  return VC_PICTURE;
}

void CSteamLinkVideo::Reset(void)
{
  // TODO
}

bool CSteamLinkVideo::GetPicture(DVDVideoPicture *pDvdVideoPicture)
{
  int width = 0;
  int height = 0;
  SLVideo_GetDisplayResolution(m_context, &width, &height);

  memset(pDvdVideoPicture, 0, sizeof(*pDvdVideoPicture));

  pDvdVideoPicture->dts = DVD_NOPTS_VALUE;
  pDvdVideoPicture->pts = DVD_NOPTS_VALUE;
  pDvdVideoPicture->iWidth = width;
  pDvdVideoPicture->iHeight= height;
  pDvdVideoPicture->iDisplayWidth = width;
  pDvdVideoPicture->iDisplayHeight= height;
  pDvdVideoPicture->format = RENDER_FMT_STEAMLINK;

  return true;
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
  *poutbuf = (uint8_t*)realloc(*poutbuf, *poutbuf_size);
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
