#pragma once
#define FD_SETSIZE 1024 
#include <WinSock2.h>
#include <Windows.h>
#include <mmsystem.h>
#include <list>
#include <unordered_map>

#include <vector>

#include "CFreeList.h"
#include "CRingBuffer.h"
#include "CPacket.h"
#include "Define.h"
#include "Protocol.h"

struct st_SESSION
{
    SOCKET Socket;
    DWORD dwSessionID;
    CRingBuffer RecvQ;
    CRingBuffer SendQ;
    DWORD dwLastRecvTime;
    int iVectorIndex;

    st_SESSION()
        : Socket(INVALID_SOCKET),
          dwSessionID(0),
          RecvQ(dfRECVBUFFER_SIZE),
          SendQ(dfSENDBUFFER_SIZE),
          dwLastRecvTime(0),
          iVectorIndex(-1)
    {
    }
};

struct st_SECTOR_POS
{
    int iX;
    int iY;
};

struct st_SECTOR_AROUND
{
    int iCount;
    st_SECTOR_POS Around[9];
};

struct st_CHARACTER
{
    st_SESSION *pSession;
    DWORD dwSessionID;
    DWORD dwAction;
    BYTE byDirection;
    BYTE byMoveDirection;
    short shX;
    short shY;
    st_SECTOR_POS CurSector;
    st_SECTOR_POS OldSector;
    std::list<st_CHARACTER *>::iterator SectorIter;
    char chHP;

    st_CHARACTER()
        : pSession(nullptr),
          dwSessionID(0),
          dwAction(dfACTION_STAND),
          byDirection(dfPACKET_MOVE_DIR_LL),
          byMoveDirection(dfPACKET_MOVE_DIR_LL),
          shX(0),
          shY(0),
          chHP(0)
    {
        CurSector.iX = 0;
        CurSector.iY = 0;
        OldSector.iX = 0;
        OldSector.iY = 0;
    }
};

extern bool g_bShutdown;
extern DWORD g_dwSessionIDCounter;
extern std::unordered_map<SOCKET, st_SESSION *> g_SessionMap;
extern std::vector<st_SESSION *> g_Sessions;
extern std::unordered_map<DWORD, st_CHARACTER *> g_CharacterMap;
extern std::list<st_CHARACTER *> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];
extern CFreeList<st_SESSION> g_SessionPool;
extern CFreeList<st_CHARACTER> g_CharacterPool;
extern DWORD g_dwMonitorLoopCount;
extern DWORD g_dwMonitorAcceptCount;
extern DWORD g_dwMonitorDisconnectCount;
extern DWORD g_dwMonitorRecvPacketCount;
extern DWORD g_dwMonitorSendPacketCount;

bool netStartUp();
void netProcess();
void netCleanUp();
void DisconnectSession(SOCKET sock);
void SendPacket_Unicast(st_SESSION *pSession, CPacket *pPacket);
void SendPacket_SectorOne(int iSectorX, int iSectorY, CPacket *pPacket, st_SESSION *pExceptSession);
void SendPacket_Around(st_SESSION *pSession, CPacket *pPacket, bool bSendMe = false);
