/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinSystemSteamLink.h"
#include "WinEventsSteamLink.h"
#include "cores/AudioEngine/Sinks/steamlink/AESinkSteamLink.h"
#include "platform/linux/powermanagement/LinuxPowerSyscall.h"
#include "settings/DisplaySettings.h"
#include "utils/log.h"
#include "windowing/GraphicContext.h"

#include <algorithm>

using namespace KODI;
using namespace STEAMLINK;

CWinSystemSteamLink::CWinSystemSteamLink()
{
  // Initialize AudioEngine
  CAESinkSteamLink::Register();

  // Initialize power management
  CLinuxPowerSyscall::Register();
}

CWinSystemSteamLink::~CWinSystemSteamLink() = default;

bool CWinSystemSteamLink::InitWindowSystem()
{
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
  {
    CLog::Log(LOGERROR, "Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }

  SDL_DisplayMode mode;
  SDL_GetDesktopDisplayMode(0, &mode);

  m_window = SDL_CreateWindow("SDL",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              mode.w,
                              mode.h,
                              SDL_WINDOW_FULLSCREEN | SDL_WINDOW_HIDDEN);
  if (m_window == nullptr)
    return false;

  SDL_VERSION(&m_windowInfo.version);

  if (SDL_GetWindowWMInfo(m_window, &m_windowInfo) < 0 || m_windowInfo.subsystem != SDL_SYSWM_VIVANTE)
  {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }

  m_nativeDisplay = m_windowInfo.info.vivante.display;

  CLog::Log(LOGDEBUG, "STEAMLINK: Initialized SDL window");

  m_winEvents.reset(new CWinEventsSteamLink());

  return CWinSystemBase::InitWindowSystem();
}

bool CWinSystemSteamLink::DestroyWindowSystem()
{
  m_winEvents.reset();

  m_nativeWindow = nullptr;
  m_nativeDisplay = nullptr;

  if (m_window != nullptr)
  {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }

  SDL_QuitSubSystem(SDL_INIT_VIDEO);

  CLog::Log(LOGDEBUG, "STEAMLINK: Deinitialized SDL window");

  return true;
}

bool CWinSystemSteamLink::CreateNewWindow(const std::string& name, bool fullScreen, RESOLUTION_INFO& res)
{
  SDL_ShowWindow(m_window);

  m_nativeWindow = m_windowInfo.info.vivante.window;

  CLog::Log(LOGDEBUG, "STEAMLINK: Created SDL window");

  return true;
}

bool CWinSystemSteamLink::DestroyWindow()
{
  m_nativeWindow = nullptr;

  SDL_HideWindow(m_window);

  CLog::Log(LOGDEBUG, "STEAMLINK: Destroyed SDL window");

  return true;
}

void CWinSystemSteamLink::UpdateResolutions()
{
  CWinSystemBase::UpdateResolutions();

  int width = 0;
  int height = 0;
  SDL_GetWindowSize(m_window, &width, &height);

  RESOLUTION_INFO& desktopRes = CDisplaySettings::GetInstance().GetResolutionInfo(RES_DESKTOP);
  UpdateDesktopResolution(desktopRes, width, height, 59.94, D3DPRESENTFLAG_PROGRESSIVE);
}

void CWinSystemSteamLink::Register(IDispResource *resource)
{
  CSingleLock lock(m_resourceSection);

  m_resources.push_back(resource);
}

void CWinSystemSteamLink::Unregister(IDispResource *resource)
{
  CSingleLock lock(m_resourceSection);

  m_resources.erase(std::remove(m_resources.begin(), m_resources.end(), resource), m_resources.end());
}

bool CWinSystemSteamLink::MessagePump()
{
  return m_winEvents->MessagePump();
}
