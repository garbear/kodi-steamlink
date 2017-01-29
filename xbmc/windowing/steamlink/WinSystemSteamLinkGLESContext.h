/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
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
  CRenderSystemBase *GetRenderSystem() override { return this; }
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
