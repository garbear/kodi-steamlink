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

#include "DVDVideoCodec.h"
#include "filesystem/File.h"

#include <string>

#define STEAMLINK_VIDEO_CODEC_NAME  "SteamLinkVideo"

namespace STEAMLINK
{

class CSteamLinkVideo : public CDVDVideoCodec
{
public:
  CSteamLinkVideo(CProcessInfo &processInfo);
  virtual ~CSteamLinkVideo();

  // implementation of CDVDVideoCodec
  virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options) override;
  virtual void Dispose() override;
  virtual int Decode(uint8_t* pData, int iSize, double dts, double pts) override;
  virtual void Reset() override;
  virtual bool GetPicture(DVDVideoPicture* pDvdVideoPicture) override;
  virtual void SetDropState(bool bDrop) override { }
  virtual const char* GetName() override { return STEAMLINK_VIDEO_CODEC_NAME; }

private:
  // Steam Link functions
  void GetDisplayResolution(int &iWidth, int &iHeight);
  bool AddPacket(uint8_t* pData, int iSize);
  bool BeginFrame(int nFrameSize);
  bool WriteFrameData(void *pData, int nDataSize);
  bool SubmitFrame();

  // VideoPlayer data
  double m_currentPts;

  // Steam Link data
  void* m_context;
  void* m_stream;
  std::string m_directory;
  XFILE::CFile m_file;
  unsigned int m_packetCount;
};

}
