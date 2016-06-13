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
#include "cores/VideoPlayer/DVDClock.h"
#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "filesystem/Directory.h"
#include "settings/AdvancedSettings.h"
#include "threads/SystemClock.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

// Steam Link video API
//#include "SLVideo.h"

#include <cstring>

using namespace STEAMLINK;
using namespace XbmcThreads;
using namespace XFILE;

#define VIDEO_DIRECTORY  "special://profile/video"

namespace
{
  /*
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
  */
}

CSteamLinkVideo::CSteamLinkVideo(CProcessInfo &processInfo) :
  CDVDVideoCodec(processInfo),
  m_currentPts(DVD_NOPTS_VALUE),
  m_context(nullptr),
  m_stream(nullptr),
  m_packetCount(0)
{
  // TODO: Refcount to allow logging with multiple instances
  //SLVideo_SetLogLevel(g_advancedSettings.CanLogComponent(LOGVIDEO) ? k_ESLVideoLogDebug : k_ESLVideoLogError);
  //SLVideo_SetLogFunction(LogFunction, nullptr);
}

CSteamLinkVideo::~CSteamLinkVideo()
{
  Dispose();
  //SLVideo_SetLogFunction(nullptr, nullptr);
}

bool CSteamLinkVideo::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  if (hints.software)
    return false;

  Dispose();

  unsigned int i = 0;

  do
  {
    m_directory = StringUtils::Format("%s/%d", VIDEO_DIRECTORY, i++);
  } while (CDirectory::Exists(m_directory));

  CLog::Log(LOGDEBUG, "%s: Using video directory %s", GetName(), m_directory.c_str());

  if (!CDirectory::Create(m_directory))
  {
    CLog::Log(LOGERROR, "%s: Failed to create directory", GetName());
    return false;
  }

  /*
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
  */

  return true;
}

void CSteamLinkVideo::Dispose()
{
  /*
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
  */
}

int CSteamLinkVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
  if (!pData || iSize == 0)
    return VC_BUFFER;

  int ret = VC_ERROR;

  if (!AddPacket(pData, iSize))
    return ret;

  // Guess if a frame was shown
  bool bFrameShown = false;
  if (pts == DVD_NOPTS_VALUE)
  {
    // Assume a frame was shown until we encounter a valid pts
    if (m_currentPts == DVD_NOPTS_VALUE)
      bFrameShown = true;
  }
  else
  {
    // Assume a frame was shown when pts changes
    if (m_currentPts != pts)
    {
      bFrameShown = true;
      m_currentPts = pts;
    }
  }

  if (bFrameShown)
    ret = VC_PICTURE;
  else
    ret = VC_BUFFER;

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

  std::memset(pDvdVideoPicture, 0, sizeof(*pDvdVideoPicture));

  pDvdVideoPicture->dts = DVD_NOPTS_VALUE;
  pDvdVideoPicture->pts = DVD_NOPTS_VALUE;//m_currentPts;
  pDvdVideoPicture->iWidth = width;
  pDvdVideoPicture->iHeight= height;
  pDvdVideoPicture->iDisplayWidth = width;
  pDvdVideoPicture->iDisplayHeight= height;
  pDvdVideoPicture->format = RENDER_FMT_STEAMLINK;

  return true;
}

void CSteamLinkVideo::GetDisplayResolution(int &iWidth, int &iHeight)
{
  //SLVideo_GetDisplayResolution(static_cast<CSLVideoContext*>(m_context), &iWidth, &iHeight);
  iWidth = 640;
  iHeight = 360;
}

bool CSteamLinkVideo::AddPacket(uint8_t* pData, int iSize)
{
  bool ret = true;

  if (!BeginFrame(iSize))
  {
    CLog::Log(LOGERROR, "%s: Failed to begin frame", GetName());
    ret = false;
  }
  else
  {
    if (!WriteFrameData(pData, iSize))
    {
      CLog::Log(LOGERROR, "%s: Tried to write more data than expected", GetName());
      ret = false;
    }

    if (!SubmitFrame())
    {
      CLog::Log(LOGERROR, "%s: Error submitting frame", GetName());
      ret = false;
    }
  }

  return ret;
}

bool CSteamLinkVideo::BeginFrame(int nFrameSize)
{  //return SLVideo_BeginFrame(static_cast<CSLVideoStream*>(m_stream), nFrameSize) == 0;
  unsigned int time = SystemClockMillis();
  std::string strFileName = StringUtils::Format("%s/%d_%d.03%d.dat", m_directory.c_str(), m_packetCount, time / 1000, time % 1000);

  return m_file.OpenForWrite(strFileName);
}

bool CSteamLinkVideo::WriteFrameData(void *pData, int nDataSize)
{
  //return SLVideo_WriteFrameData(static_cast<CSLVideoStream*>(m_stream), pData, nDataSize) == 0;
  return m_file.Write(pData, nDataSize) == nDataSize;
}

bool CSteamLinkVideo::SubmitFrame()
{
  //return SLVideo_SubmitFrame(static_cast<CSLVideoStream*>(m_stream)) == 0;
  m_file.Close();
  m_packetCount++;
  return true;
}
