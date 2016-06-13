/*
 *      Copyright (C) 2016 Team Kodi
 *      Copyright (C) 2016 Valve Corporation
 *      http://kodi.tv
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

#include "system.h" // for HAS_GLES, needed for LinuxRendererGLES.h

#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGLES.h"

namespace STEAMLINK
{

class CRendererSteamLink : public CLinuxRendererGLES
{
public:
  CRendererSteamLink() { }
  virtual ~CRendererSteamLink() { }

  // implementation of CBaseRenderer via CLinuxRendererGLES
  virtual bool IsGuiLayer() override { return false; }
  virtual EINTERLACEMETHOD AutoInterlaceMethod() override { return VS_INTERLACEMETHOD_NONE; }
  virtual bool SupportsMultiPassRendering() override { return false; };
  virtual bool Supports(ERENDERFEATURE feature) override { return false; };
  virtual bool Supports(EDEINTERLACEMODE mode) override { return false; };
  virtual bool Supports(EINTERLACEMETHOD method) override { return false; };
  virtual bool Supports(ESCALINGMETHOD method) override { return false; };
  
protected:
  // implementation of CLinuxRendererGLES
  virtual bool LoadShadersHook() override;
  virtual bool RenderHook(int index) override { return true; }
  virtual bool RenderUpdateVideoHook(bool clear, DWORD flags = 0, DWORD alpha = 255) override;
  virtual int  GetImageHook(YV12Image *image, int source = AUTOSOURCE, bool readonly = false) override;
};

}
