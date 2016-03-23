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
#include "DVDVideoCodec.h"
#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

// Steam Link video API
#include "SLVideo.h"

using namespace STEAMLINK;

namespace
{
  void LogFunction(void *pContext, ESLVideoLog eLogLevel, const char *pszMessage)
  {
    // TODO: Problem that log messages end with a newline?
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

CSteamLinkVideo::CSteamLinkVideo(CProcessInfo &processInfo) :
  CDVDVideoCodec(processInfo),
  m_context(nullptr),
  m_stream(nullptr)
{
  // TODO: Refcount to allow logging with multiple instances
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
        stream = SLVideo_CreateStream(context, k_ESLVideoFormatH264);
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
        GetDisplayResolution(width, height);

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
    SLVideo_FreeStream(static_cast<CSLVideoStream*>(m_stream));
    m_stream = nullptr;
  }
  if (m_context)
  {
    SLVideo_FreeContext(static_cast<CSLVideoContext*>(m_context));
    m_context = nullptr;
  }
}

int CSteamLinkVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
  int ret = VC_ERROR;

  if (BeginFrame(iSize))
  {
    if (WriteFrameData(pData, iSize))
    {
      if (SubmitFrame())
        ret = VC_PICTURE;
      else
        CLog::Log(LOGERROR, "%s: Error submitting frame", GetName());
    }
    else
    {
      CLog::Log(LOGERROR, "%s: Tried to write more data than expected", GetName()); // TODO
    }
  }
  else
  {
    CLog::Log(LOGERROR, "%s: Failed to begin frame", GetName());
  }

  return ret;
}

void CSteamLinkVideo::Reset(void)
{
  // TODO
}

bool CSteamLinkVideo::GetPicture(DVDVideoPicture *pDvdVideoPicture)
{
  int width = 0;
  int height = 0;
  GetDisplayResolution(width, height);

  memset(pDvdVideoPicture, 0, sizeof(*pDvdVideoPicture));

  pDvdVideoPicture->iWidth = width;
  pDvdVideoPicture->iHeight= height;
  pDvdVideoPicture->iDisplayWidth = width;
  pDvdVideoPicture->iDisplayHeight= height;
  pDvdVideoPicture->format = RENDER_FMT_BYPASS;

  return true;
}

void CSteamLinkVideo::GetDisplayResolution(int &iWidth, int &iHeight)
{
  SLVideo_GetDisplayResolution(static_cast<CSLVideoContext*>(m_context), &iWidth, &iHeight);
}

bool CSteamLinkVideo::BeginFrame(int nFrameSize)
{
  return SLVideo_BeginFrame(static_cast<CSLVideoStream*>(m_stream), nFrameSize) == 0;
}

bool CSteamLinkVideo::WriteFrameData(void *pData, int nDataSize)
{
  return SLVideo_WriteFrameData(static_cast<CSLVideoStream*>(m_stream), pData, nDataSize) == 0;
}

bool CSteamLinkVideo::SubmitFrame()
{
  return SLVideo_SubmitFrame(static_cast<CSLVideoStream*>(m_stream)) == 0;
}
