/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GLContextEGL.h"
#include "guilib/IDirtyRegionSolver.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/log.h"
#include "ServiceBroker.h"

using namespace KODI;
using namespace STEAMLINK;

CGLContextEGL::CGLContextEGL() :
  m_eglDisplay(EGL_NO_DISPLAY),
  m_eglSurface(EGL_NO_SURFACE),
  m_eglContext(EGL_NO_CONTEXT),
  m_eglConfig(nullptr)
{
}

CGLContextEGL::~CGLContextEGL()
{
  Destroy();
}

bool CGLContextEGL::CreateDisplay(EGLDisplay display,
                                  EGLint renderable_type,
                                  EGLint rendering_api)
{
  EGLint neglconfigs = 0;
  int major, minor;

  EGLint surface_type = EGL_WINDOW_BIT;
  // for the non-trivial dirty region modes, we need the EGL buffer to be preserved across updates
  if (CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_guiAlgorithmDirtyRegions == DIRTYREGION_SOLVER_COST_REDUCTION ||
      CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_guiAlgorithmDirtyRegions == DIRTYREGION_SOLVER_UNION)
    surface_type |= EGL_SWAP_BEHAVIOR_PRESERVED_BIT;

  EGLint attribs[] =
  {
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      8,
    EGL_DEPTH_SIZE,     16,
    EGL_STENCIL_SIZE,    0,
    EGL_SAMPLE_BUFFERS,  0,
    EGL_SAMPLES,         0,
    EGL_SURFACE_TYPE,    surface_type,
    EGL_RENDERABLE_TYPE, renderable_type,
    EGL_NONE
  };

  if (m_eglDisplay == EGL_NO_DISPLAY)
  {
    m_eglDisplay = eglGetDisplay((EGLNativeDisplayType)display);
  }

  if (m_eglDisplay == EGL_NO_DISPLAY)
  {
    CLog::Log(LOGERROR, "failed to get EGL display");
    return false;
  }

  if (!eglInitialize(m_eglDisplay, &major, &minor))
  {
    CLog::Log(LOGERROR, "failed to initialize EGL display");
    return false;
  }

  eglBindAPI(rendering_api);

  if (!eglChooseConfig(m_eglDisplay, attribs,
                       &m_eglConfig, 1, &neglconfigs))
  {
    CLog::Log(LOGERROR, "Failed to query number of EGL configs");
    return false;
  }

  if (neglconfigs <= 0)
  {
    CLog::Log(LOGERROR, "No suitable EGL configs found");
    return false;
  }

  return true;
}

bool CGLContextEGL::CreateContext()
{
  int client_version = 2;

  const EGLint context_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, client_version, EGL_NONE
  };

  if (m_eglContext == EGL_NO_CONTEXT)
  {
    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig,
                                    EGL_NO_CONTEXT, context_attribs);
  }

  if (m_eglContext == EGL_NO_CONTEXT)
  {
    CLog::Log(LOGERROR, "failed to create EGL context");
    return false;
  }

  return true;
}

bool CGLContextEGL::BindContext()
{
  if (!eglMakeCurrent(m_eglDisplay, m_eglSurface,
                      m_eglSurface, m_eglContext))
  {
    CLog::Log(LOGERROR, "Failed to make context current %p %p %p",
                         m_eglDisplay, m_eglSurface, m_eglContext);
    return false;
  }

  return true;
}

bool CGLContextEGL::SurfaceAttrib()
{
  // for the non-trivial dirty region modes, we need the EGL buffer to be preserved across updates
  if (CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_guiAlgorithmDirtyRegions == DIRTYREGION_SOLVER_COST_REDUCTION ||
      CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_guiAlgorithmDirtyRegions == DIRTYREGION_SOLVER_UNION)
  {
    if ((m_eglDisplay == EGL_NO_DISPLAY) || (m_eglSurface == EGL_NO_SURFACE))
    {
      return false;
    }

    if (!eglSurfaceAttrib(m_eglDisplay, m_eglSurface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED))
    {
      CLog::Log(LOGDEBUG, "%s: Could not set EGL_SWAP_BEHAVIOR",__FUNCTION__);
    }
  }

  return true;
}

bool CGLContextEGL::CreateSurface(EGLNativeWindowType surface)
{
  m_eglSurface = eglCreateWindowSurface(m_eglDisplay,
                                        m_eglConfig,
                                        surface,
                                        nullptr);

  if (m_eglSurface == EGL_NO_SURFACE)
  {
    CLog::Log(LOGERROR, "failed to create EGL window surface %d", eglGetError());
    return false;
  }

  return true;
}

void CGLContextEGL::Destroy()
{
  if (m_eglContext != EGL_NO_CONTEXT)
  {
    eglDestroyContext(m_eglDisplay, m_eglContext);
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    m_eglContext = EGL_NO_CONTEXT;
  }

  if (m_eglSurface != EGL_NO_SURFACE)
  {
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    m_eglSurface = EGL_NO_SURFACE;
  }

  if (m_eglDisplay != EGL_NO_DISPLAY)
  {
    eglTerminate(m_eglDisplay);
    m_eglDisplay = EGL_NO_DISPLAY;
  }
}

void CGLContextEGL::Detach()
{
  if (m_eglContext != EGL_NO_CONTEXT)
  {
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  }

  if (m_eglSurface != EGL_NO_SURFACE)
  {
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    m_eglSurface = EGL_NO_SURFACE;
  }
}

bool CGLContextEGL::SetVSync(bool enable)
{
  if (!eglSwapInterval(m_eglDisplay, enable))
  {
    return false;
  }

  return true;
}

void CGLContextEGL::SwapBuffers()
{
  if (m_eglDisplay == EGL_NO_DISPLAY || m_eglSurface == EGL_NO_SURFACE)
  {
    return;
  }

  eglSwapBuffers(m_eglDisplay, m_eglSurface);
}
