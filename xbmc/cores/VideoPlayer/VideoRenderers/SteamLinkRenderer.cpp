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

#include "SteamLinkRenderer.h"
#include "utils/log.h"

#include <GLES2/gl2.h>

using namespace STEAMLINK;

bool CSteamLinkRenderer::LoadShadersHook()
{
  CLog::Log(LOGNOTICE, "CSteamLinkRenderer: Using STEAMLINK render method");
  m_textureTarget = GL_TEXTURE_2D;
  m_renderMethod = RENDER_BYPASS;
  return false;
}

int CSteamLinkRenderer::GetImageHook(YV12Image *image, int source /* = AUTOSOURCE */, bool readonly /* = false */)
{
  return source;
}
