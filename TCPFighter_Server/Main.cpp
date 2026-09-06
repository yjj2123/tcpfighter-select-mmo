#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include "CCrashDump.h"
#include "CFreeList.h"
#include "Contents.h"
#include "Log.h"
#include "Network.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

long procademy::CCrashDump::_DumpCount = 0;
int g_iLogLevel = dfLOG_LEVEL_ERROR;
bool g_bShutdown = false;
DWORD g_dwSessionIDCounter = 0;
std::unordered_map<SOCKET, st_SESSION *> g_SessionMap;
std::vector<st_SESSION *> g_Sessions;
std::unordered_map<DWORD, st_CHARACTER *> g_CharacterMap;
std::list<st_CHARACTER *> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];
CFreeList<st_SESSION> g_SessionPool(dfSESSION_MAX + 200);
CFreeList<st_CHARACTER> g_CharacterPool(dfSESSION_MAX + 200);
DWORD g_dwMonitorLoopCount = 0;
DWORD g_dwMonitorAcceptCount = 0;
DWORD g_dwMonitorDisconnectCount = 0;
DWORD g_dwMonitorRecvPacketCount = 0;
DWORD g_dwMonitorSendPacketCount = 0;
procademy::CCrashDump g_CrashDump;

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    timeBeginPeriod(1);

    if (!netStartUp())
    {
        timeEndPeriod(1);
        return 1;
    }

    while (!g_bShutdown)
    {
        netProcess();
        Update();
        ServerControl();
        ++g_dwMonitorLoopCount;
        Monitor();
    }

    netCleanUp();
    timeEndPeriod(1);
    return 0;
}
