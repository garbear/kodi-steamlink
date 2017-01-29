/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "threads/Thread.h"

#include <memory>

class CLinuxResourceCounter;

namespace KODI
{
namespace STEAMLINK
{

class CSteamLinkPerformance : protected CThread
{
public:
  CSteamLinkPerformance();
  ~CSteamLinkPerformance() override;

protected:
  // Implementation of CThread
  void Process() override;

private:
  std::unique_ptr<CLinuxResourceCounter> m_resourceCounter;
};

}
}
