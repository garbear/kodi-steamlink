/*
 *  Copyright (C) 2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SteamLinkPerformance.h"
#include "guilib/guiinfo/GUIInfoProviders.h"
#include "guilib/guiinfo/SystemGUIInfo.h"
#include "guilib/GUIComponent.h"
#include "platform/linux/LinuxResourceCounter.h"
#include "platform/linux/XMemUtils.h"
#include "utils/CPUInfo.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "CompileInfo.h"
#include "GUIInfoManager.h"
#include "ServiceBroker.h"

using namespace KODI;
using namespace STEAMLINK;

CSteamLinkPerformance::CSteamLinkPerformance() :
  CThread("SteamLinkPerformance"),
  m_resourceCounter(new CLinuxResourceCounter)
{
  Create();
}

CSteamLinkPerformance::~CSteamLinkPerformance()
{
  StopThread(true);
}

void CSteamLinkPerformance::Process()
{
  while (!m_bStop)
  {
    g_cpuInfo.getUsedPercentage(); // must call it to recalculate pct values

    MEMORYSTATUSEX stat;
    stat.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&stat);

    CGUIComponent* gui = CServiceBroker::GetGUI();
    if (gui != nullptr)
    {
      float fps = gui->GetInfoManager().GetInfoProviders().GetSystemInfoProvider().GetFPS();
      CLog::Log(LOGDEBUG, "MEM: %" PRIu64"/%" PRIu64" KB - FPS: %2.1f fps",
          stat.ullAvailPhys/1024,
          stat.ullTotalPhys/1024,
          fps);
    }
    else
    {
      CLog::Log(LOGDEBUG, "MEM: %" PRIu64"/%" PRIu64" KB",
          stat.ullAvailPhys/1024,
          stat.ullTotalPhys/1024);
    }

    std::string strCores = g_cpuInfo.GetCoresUsageString();
    std::string ucAppName = CCompileInfo::GetAppName();
    StringUtils::ToUpper(ucAppName);
    double dCPU = m_resourceCounter->GetCPUUsage();

    CLog::Log(LOGDEBUG, "CPU: %s (%s: %4.2f%%)",
        strCores.c_str(),
        ucAppName.c_str(),
        dCPU);

    Sleep(1000);
  }
}
