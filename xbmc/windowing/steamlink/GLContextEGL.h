/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "EGL/egl.h"

namespace KODI
{
namespace STEAMLINK
{

class CGLContextEGL
{
public:
  CGLContextEGL();
  virtual ~CGLContextEGL();

  bool CreateDisplay(EGLDisplay display,
                     EGLint renderable_type,
                     EGLint rendering_api);

  bool CreateSurface(EGLNativeWindowType surface);
  bool CreateContext();
  bool BindContext();
  bool SurfaceAttrib();
  void Destroy();
  void Detach();
  bool SetVSync(bool enable);
  void SwapBuffers();

  EGLDisplay m_eglDisplay;
  EGLSurface m_eglSurface;
  EGLContext m_eglContext;
  EGLConfig m_eglConfig;
};

}
}
