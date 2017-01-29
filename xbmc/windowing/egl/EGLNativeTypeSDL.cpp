/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "system.h"

#include "EGLNativeTypeSDL.h"
#include "guilib/gui3d.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

#if HAVE_SDL_VERSION == 2

CEGLNativeTypeSDL::CEGLNativeTypeSDL()
{
  m_window = NULL;

  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    return;
  }

  SDL_DisplayMode mode;
  SDL_GetDesktopDisplayMode(0, &mode);

  m_window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mode.w, mode.h, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_HIDDEN);
  if (!m_window) {
    return;
  }

  SDL_VERSION(&m_windowInfo.version);
  if (SDL_GetWindowWMInfo(m_window, &m_windowInfo) < 0 || m_windowInfo.subsystem != SDL_SYSWM_VIVANTE) {
    SDL_DestroyWindow(m_window);
    m_window = NULL;
  }

  // XBMC draws its own cursor
  SDL_ShowCursor(0);
}

CEGLNativeTypeSDL::~CEGLNativeTypeSDL()
{
  if (m_window) {
    SDL_DestroyWindow(m_window);
  }
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
} 

bool CEGLNativeTypeSDL::CheckCompatibility()
{
  return (m_window != NULL);
}

void CEGLNativeTypeSDL::Initialize()
{
}

void CEGLNativeTypeSDL::Destroy()
{
}

bool CEGLNativeTypeSDL::CreateNativeDisplay()
{
  return true;
}

bool CEGLNativeTypeSDL::CreateNativeWindow()
{
  SDL_ShowWindow(m_window);
  return true;
}

bool CEGLNativeTypeSDL::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  if (!nativeDisplay)
    return false;
  *nativeDisplay = (XBNativeDisplayType*) &m_windowInfo.info.vivante.display;
  return true;
}

bool CEGLNativeTypeSDL::GetNativeWindow(XBNativeDisplayType **nativeWindow) const
{
  if (!nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_windowInfo.info.vivante.window;
  return true;
}  

bool CEGLNativeTypeSDL::DestroyNativeDisplay()
{
  return true;
}

bool CEGLNativeTypeSDL::DestroyNativeWindow()
{
  SDL_HideWindow(m_window);
  return true;
}

bool CEGLNativeTypeSDL::GetNativeResolution(RESOLUTION_INFO *res) const
{
  SDL_GetWindowSize(m_window, &res->iWidth, &res->iHeight);
  res->fRefreshRate = 59.94f;
  res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
  res->iScreen = 0;
  res->bFullScreen = true;
  res->iSubtitles = static_cast<int>(0.965 * res->iHeight);
  res->fPixelRatio = 1.0f;
  res->iScreenWidth = res->iWidth;
  res->iScreenHeight = res->iHeight;
  res->strMode = StringUtils::Format("%dx%d @ %.2fp",
                     res->iScreenWidth,
                     res->iScreenHeight,
                     res->fRefreshRate);
  return true;
}

bool CEGLNativeTypeSDL::SetNativeResolution(const RESOLUTION_INFO &res)
{
  return true;
}

bool CEGLNativeTypeSDL::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  RESOLUTION_INFO res;
  if (!GetNativeResolution(&res))
    return false;

  resolutions.push_back(res);
  return true;
}

bool CEGLNativeTypeSDL::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  return GetNativeResolution(res);
}

bool CEGLNativeTypeSDL::ShowWindow(bool show)
{
  if (show)
    SDL_ShowWindow(m_window);
  else
    SDL_HideWindow(m_window);
  return true;
}

#endif // SDL 2.0
