/*
 *      Copyright (C) 2016-2018 Team Kodi
 *      Copyright (C) 2016-2018 Valve Corporation
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

#include "GLContextEGL.h"
#include "threads/CriticalSection.h"
#include "windowing/WinSystem.h"

#include <EGL/egl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <vector>

class IDispResource;

namespace KODI
{
namespace STEAMLINK
{

class CWinSystemSteamLink : public CWinSystemBase
{
public:
  CWinSystemSteamLink();
  ~CWinSystemSteamLink() override = default;

  // implementation of CWinSystemBase
  bool InitWindowSystem() override;
  bool DestroyWindowSystem() override;
  bool CreateNewWindow(const std::string& name, bool fullScreen, RESOLUTION_INFO& res) override;
  bool DestroyWindow() override;
  bool ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop) override { return true; }
  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override { return true; }
  bool Hide() override { return true; }
  bool Show(bool raise = true) override { return true; }
  void UpdateResolutions() override;
  void Register(IDispResource *resource) override;
  void Unregister(IDispResource *resource) override;

protected:
  // EGL properties
  EGLDisplay m_nativeDisplay = nullptr;
  EGLNativeWindowType m_nativeWindow = nullptr;

  // SDL properties
  SDL_Window *m_window = nullptr;
  SDL_SysWMinfo m_windowInfo;

  // Display resources
  CCriticalSection m_resourceSection;
  std::vector<IDispResource*>  m_resources;
};

}
}
