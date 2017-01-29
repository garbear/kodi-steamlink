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

#include "RendererSteamLink.h"
#include "cores/VideoPlayer/DVDCodecs/Video/SteamLinkVideoBuffer.h"
#include "cores/VideoPlayer/DVDCodecs/Video/SteamLinkVideoStream.h"
#include "utils/log.h"

// Steam Link video API
#include "SLVideo.h"

#include <GLES2/gl2.h>

#include <utility>

using namespace STEAMLINK;

void CRendererSteamLink::AddVideoPictureHW(DVDVideoPicture &picture, int index)
{
  if (picture.steamlink)
  {
    YUVBUFFER &buf = m_buffers[index];
    buf.hwDec = new CSteamLinkBuffer(std::move(*picture.steamlink));
  }
}

void CRendererSteamLink::PreInit()
{
  CLinuxRendererGLES::PreInit();

  m_formats.clear();
  m_formats.push_back(RENDER_FMT_STEAMLINK);
}

void CRendererSteamLink::ReleaseBuffer(int idx)
{
  YUVBUFFER &buf = m_buffers[idx];

  delete static_cast<CSteamLinkBuffer*>(buf.hwDec);
  buf.hwDec = nullptr;
}

bool CRendererSteamLink::LoadShadersHook()
{
  CLog::Log(LOGNOTICE, "CRendererSteamLink: Using STEAMLINK render method");
  m_textureTarget = GL_TEXTURE_2D;
  m_renderMethod = RENDER_BYPASS;
  return true;
}

bool CRendererSteamLink::RenderUpdateVideoHook(bool clear, DWORD flags /* = 0 */, DWORD alpha /* = 255 */)
{
  ManageRenderArea();

  CSteamLinkBuffer* pBuffer = static_cast<CSteamLinkBuffer*>(m_buffers[m_iYV12RenderBuffer].hwDec);

  if (pBuffer && pBuffer->buffer)
  {
    CSteamLinkBuffer buffer(std::move(*pBuffer));

    buffer.stream->WriteData(buffer.buffer.get(), buffer.size);
  }

  return true;
}

int CRendererSteamLink::GetImageHook(YV12Image *image, int source /* = AUTOSOURCE */, bool readonly /* = false */)
{
  return source;
}
