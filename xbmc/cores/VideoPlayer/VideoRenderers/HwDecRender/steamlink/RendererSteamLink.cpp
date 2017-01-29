/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RendererSteamLink.h"
#include "cores/VideoPlayer/DVDCodecs/Video/steamlink/SteamLinkVideoBuffer.h"
#include "cores/VideoPlayer/DVDCodecs/Video/steamlink/SteamLinkVideoStream.h"
#include "cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "settings/DisplaySettings.h"
#include "windowing/GraphicContext.h"
#include "windowing/Resolution.h"
#include "windowing/WinSystem.h"
#include "utils/log.h"
#include "ServiceBroker.h"

// Steam Link video API
#include "SLVideo.h"

#include <utility>

using namespace KODI;
using namespace STEAMLINK;

CRendererSteamLink::~CRendererSteamLink()
{
  Flush(false);
}

CBaseRenderer* CRendererSteamLink::Create(CVideoBuffer* buffer)
{
  return new CRendererSteamLink();
}

void CRendererSteamLink::Register()
{
  VIDEOPLAYER::CRendererFactory::RegisterRenderer("steamlink", CRendererSteamLink::Create);
}

bool CRendererSteamLink::Configure(const VideoPicture& picture, float fps, unsigned int orientation)
{
  // Configure CBaseRenderer
  m_sourceWidth = picture.iWidth;
  m_sourceHeight = picture.iHeight;

  // Calculate the input frame aspect ratio.
  CalculateFrameAspectRatio(picture.iDisplayWidth, picture.iDisplayHeight);
  SetViewMode(m_videoSettings.m_ViewMode);
  ManageRenderArea();

  Flush(false);

  m_bConfigured = true;
  return true;
}

void CRendererSteamLink::AddVideoPicture(const VideoPicture& picture, int index)
{
  BUFFER& buf = m_buffers[index];
  if (buf.videoBuffer)
  {
    CLog::LogF(LOGERROR, "Unreleased video buffer");
    buf.videoBuffer->Release();
  }
  buf.videoBuffer = picture.videoBuffer;
  buf.videoBuffer->Acquire();
}

bool CRendererSteamLink::Flush(bool saveBuffers)
{
  if (!saveBuffers)
  {
    for (int i = 0; i < NUM_BUFFERS; i++)
      ReleaseBuffer(i);
  }

  return saveBuffers;
}

void CRendererSteamLink::ReleaseBuffer(int idx)
{
  BUFFER& buf = m_buffers[idx];
  if (buf.videoBuffer)
  {
    buf.videoBuffer->Release();
    buf.videoBuffer = nullptr;
  }
}

bool CRendererSteamLink::NeedBuffer(int index)
{
  BUFFER& buf = m_buffers[index];
  CSteamLinkVideoBuffer* buffer = dynamic_cast<CSteamLinkVideoBuffer*>(buf.videoBuffer);
  if (buffer && buffer->buffer)
    return true;

  return false;
}

CRenderInfo CRendererSteamLink::GetRenderInfo()
{
  CRenderInfo info;
  info.max_buffer_size = NUM_BUFFERS;
  return info;
}

void CRendererSteamLink::Update()
{
  if (!m_bConfigured)
    return;

  ManageRenderArea();
}

void CRendererSteamLink::RenderUpdate(int index, int index2, bool clear, unsigned int flags, unsigned int alpha)
{
  BUFFER& buf = m_buffers[index];
  if (buf.videoBuffer)
  {
    CSteamLinkVideoBuffer* buffer = dynamic_cast<CSteamLinkVideoBuffer*>(buf.videoBuffer);
    if (buffer != nullptr && buffer->buffer)
      buffer->stream->WriteData(buffer->buffer.get(), buffer->size);

    ReleaseBuffer(index);
  }
}

void CRendererSteamLink::ManageRenderArea()
{
  CBaseRenderer::ManageRenderArea();

  RESOLUTION_INFO info = CServiceBroker::GetWinSystem()->GetGfxContext().GetResInfo();
  if (info.iScreenWidth != info.iWidth)
  {
    CalcNormalRenderRect(0, 0, info.iScreenWidth, info.iScreenHeight,
                         GetAspectRatio() * CDisplaySettings::GetInstance().GetPixelRatio(),
                         CDisplaySettings::GetInstance().GetZoomAmount(),
                         CDisplaySettings::GetInstance().GetVerticalShift());
  }
}
