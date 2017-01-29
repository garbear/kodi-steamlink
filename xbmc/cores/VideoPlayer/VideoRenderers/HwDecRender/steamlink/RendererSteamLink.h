/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  Copyright (C) 2016-2018 Valve Corporation
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/VideoPlayer/VideoRenderers/BaseRenderer.h"

namespace KODI
{
namespace STEAMLINK
{

class CRendererSteamLink : public CBaseRenderer
{
public:
  CRendererSteamLink() = default;
  ~CRendererSteamLink() override;

  // Registration
  static CBaseRenderer* Create(CVideoBuffer* buffer);
  static void Register();

  // Implementation of CBaseRenderer
  bool Configure(const VideoPicture& picture, float fps, unsigned int orientation) override;
  bool IsConfigured() override { return m_bConfigured; };
  void AddVideoPicture(const VideoPicture& picture, int index) override;
  void UnInit() override { }
  bool Flush(bool saveBuffers) override;
  void ReleaseBuffer(int idx) override;
  bool NeedBuffer(int idx) override;
  bool IsGuiLayer() override { return false; }
  bool CanFFRW() override { return false; }
  CRenderInfo GetRenderInfo() override;
  void Update() override;
  void RenderUpdate(int index, int index2, bool clear, unsigned int flags, unsigned int alpha) override;
  bool RenderCapture(CRenderCapture* capture) override { return false; }
  bool ConfigChanged(const VideoPicture& picture) override { return false; }
  bool SupportsMultiPassRendering() override { return false; }
  bool Supports(ERENDERFEATURE feature) override { return false; }
  bool Supports(ESCALINGMETHOD method) override { return false; }

protected:
  // Implementation of CBaseRenderer
  void ManageRenderArea() override;

private:
  bool m_bConfigured = false;

  struct BUFFER
  {
    CVideoBuffer* videoBuffer = nullptr;
  };

  BUFFER m_buffers[NUM_BUFFERS];
};

}
}
