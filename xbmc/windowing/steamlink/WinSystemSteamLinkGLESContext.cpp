/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinSystemSteamLinkGLESContext.h"
#include "cores/RetroPlayer/process/steamlink/RPProcessInfoSteamLink.h"
#include "cores/RetroPlayer/rendering/VideoRenderers/RPRendererOpenGLES.h"
#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGLES.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "utils/log.h"

using namespace KODI;
using namespace STEAMLINK;

std::unique_ptr<CWinSystemBase> CWinSystemBase::CreateWinSystem()
{
  std::unique_ptr<CWinSystemBase> winSystem(new CWinSystemSteamLinkGLESContext);
  return winSystem;
}

bool CWinSystemSteamLinkGLESContext::InitWindowSystem()
{
  // Initialize VideoPlayer
  CLinuxRendererGLES::Register();

  // Initialize RetroPlayer
  CRPProcessInfoSteamLink::Register();
  CRPProcessInfoSteamLink::RegisterRendererFactory(new RETRO::CRendererFactoryOpenGLES);

  if (!CWinSystemSteamLink::InitWindowSystem())
    return false;

  if (!m_pGLContext.CreateDisplay(m_nativeDisplay,
                                  EGL_OPENGL_ES2_BIT,
                                  EGL_OPENGL_ES_API))
  {
    return false;
  }

  return true;
}

bool CWinSystemSteamLinkGLESContext::DestroyWindowSystem()
{
  //! @todo Deinitialize RetroPlayer

  // Deinitialize VideoPlayer
  VIDEOPLAYER::CRendererFactory::ClearRenderer();

  m_pGLContext.Destroy();

  return CWinSystemSteamLink::DestroyWindowSystem();
}

bool CWinSystemSteamLinkGLESContext::CreateNewWindow(const std::string& name,
                                                     bool fullScreen,
                                                     RESOLUTION_INFO& res)
{
  m_pGLContext.Detach();

  if (!CWinSystemSteamLink::DestroyWindow())
    return false;

  if (!CWinSystemSteamLink::CreateNewWindow(name, fullScreen, res))
    return false;

  if (!m_pGLContext.CreateSurface(m_nativeWindow))
    return false;

  if (!m_pGLContext.CreateContext())
    return false;

  if (!m_pGLContext.BindContext())
    return false;

  if (!m_pGLContext.SurfaceAttrib())
    return false;

  return true;
}

bool CWinSystemSteamLinkGLESContext::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  CWinSystemSteamLink::SetFullScreen(fullScreen, res, blankOtherDisplays);
  CRenderSystemGLES::ResetRenderSystem(res.iWidth, res.iHeight);

  return true;
}

void CWinSystemSteamLinkGLESContext::PresentRenderImpl(bool rendered)
{
  if (rendered)
    m_pGLContext.SwapBuffers();
}

EGLDisplay CWinSystemSteamLinkGLESContext::GetEGLDisplay() const
{
  return m_pGLContext.m_eglDisplay;
}

EGLSurface CWinSystemSteamLinkGLESContext::GetEGLSurface() const
{
  return m_pGLContext.m_eglSurface;
}

EGLContext CWinSystemSteamLinkGLESContext::GetEGLContext() const
{
  return m_pGLContext.m_eglContext;
}

EGLConfig  CWinSystemSteamLinkGLESContext::GetEGLConfig() const
{
  return m_pGLContext.m_eglConfig;
}
