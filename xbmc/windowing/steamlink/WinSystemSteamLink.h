/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "GLContextEGL.h"
#include "threads/CriticalSection.h"
#include "windowing/WinSystem.h"

#include <EGL/egl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <vector>

class CLibInputHandler;
class IDispResource;

namespace KODI
{
namespace STEAMLINK
{

class CWinSystemSteamLink : public CWinSystemBase
{
public:
  CWinSystemSteamLink();
  ~CWinSystemSteamLink() override;

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
  bool MessagePump() override;

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

  // Input properties
  std::unique_ptr<CLibInputHandler> m_libinput;
};

}
}
