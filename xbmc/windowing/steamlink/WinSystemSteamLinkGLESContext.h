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
#include "WinSystemSteamLink.h"
#include "rendering/gles/RenderSystemGLES.h"

#include <memory>

namespace KODI
{
namespace STEAMLINK
{

class CWinSystemSteamLinkGLESContext : public CWinSystemSteamLink,
                                       public CRenderSystemGLES
{
public:
  CWinSystemSteamLinkGLESContext() = default;
  virtual ~CWinSystemSteamLinkGLESContext() = default;

  // implementation of CWinSystemBase via CWinSystemSteamLink
  bool InitWindowSystem() override;
  bool DestroyWindowSystem() override;
  bool CreateNewWindow(const std::string& name, bool fullScreen, RESOLUTION_INFO& res) override;
  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override;

  EGLDisplay GetEGLDisplay() const;
  EGLSurface GetEGLSurface() const;
  EGLContext GetEGLContext() const;
  EGLConfig  GetEGLConfig() const;

protected:
  // implementation of CWinSystemBase via CWinSystemSteamLink
  void SetVSyncImpl(bool enable) override { }
  void PresentRenderImpl(bool rendered) override;

private:
  CGLContextEGL m_pGLContext;
};

}
}
