#include "Network.h"

#include <cstdio>
#include <WS2tcpip.h>
#include <set>
#include <vector>

#include "Contents.h"
#include "Log.h"
#include "PacketProc.h"
#include "Protocol.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

static SOCKET g_ListenSocket = INVALID_SOCKET;
static std::set<SOCKET> g_DisconnectReserve;

static bool SetNonBlocking(SOCKET sock)
{
    u_long on = 1;
    return ioctlsocket(sock, FIONBIO, &on) == 0;
}

static void SetNoDelay(SOCKET sock)
{
    BOOL bNoDelay = TRUE;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&bNoDelay), sizeof(bNoDelay));
}

static void ProcessDisconnects()
{
    if (g_DisconnectReserve.empty())
    {
        return;
    }

    std::vector<SOCKET> reserved(g_DisconnectReserve.begin(), g_DisconnectReserve.end());
    g_DisconnectReserve.clear();

    for (SOCKET sock : reserved)
    {
        auto iter = g_SessionMap.find(sock);
        if (iter == g_SessionMap.end())
        {
            continue;
        }

        st_SESSION *pSession = iter->second;
        DWORD dwSessionID = pSession->dwSessionID;

        int idx = pSession->iVectorIndex;
        if (idx >= 0 && idx < static_cast<int>(g_Sessions.size()))
        {
            int lastIdx = static_cast<int>(g_Sessions.size()) - 1;
            if (idx != lastIdx)
            {
                g_Sessions[idx] = g_Sessions[lastIdx];
                g_Sessions[idx]->iVectorIndex = idx;
            }
            g_Sessions.pop_back();
            pSession->iVectorIndex = -1;
        }

        DeleteCharacter(dwSessionID);
        closesocket(pSession->Socket);
        pSession->Socket = INVALID_SOCKET;
        pSession->RecvQ.ClearBuffer();
        pSession->SendQ.ClearBuffer();
        g_SessionPool.Free(pSession);
        g_SessionMap.erase(iter);
        ++g_dwMonitorDisconnectCount;
    }
}

static void netAcceptProc()
{
    while (true)
    {
        SOCKADDR_IN clientAddr;
        int iAddrLen = sizeof(clientAddr);
        SOCKET clientSock = accept(g_ListenSocket, reinterpret_cast<SOCKADDR *>(&clientAddr), &iAddrLen);
        if (clientSock == INVALID_SOCKET)
        {
            int iError = WSAGetLastError();
            if (iError == WSAEWOULDBLOCK)
            {
                break;
            }

            break;
        }

        SetNonBlocking(clientSock);
        SetNoDelay(clientSock);

        if (g_SessionMap.size() >= dfSESSION_MAX)
        {
            closesocket(clientSock);
            break;
        }

        st_SESSION *pSession = g_SessionPool.Alloc();
        if (pSession == nullptr)
        {
            closesocket(clientSock);
            continue;
        }

        pSession->Socket = clientSock;
        pSession->dwSessionID = g_dwSessionIDCounter++;
        pSession->dwLastRecvTime = timeGetTime();
        pSession->RecvQ.ClearBuffer();
        pSession->SendQ.ClearBuffer();

        g_SessionMap.insert(std::make_pair(clientSock, pSession));
        g_Sessions.push_back(pSession);
        pSession->iVectorIndex = static_cast<int>(g_Sessions.size()) - 1;
        CreateCharacter(pSession->dwSessionID, pSession);
        ++g_dwMonitorAcceptCount;
    }
}

static void netRecvProc(st_SESSION* pSession)
{
    int iDirectSize = pSession->RecvQ.DirectEnqueueSize();
    if (iDirectSize <= 0)
    {
        DisconnectSession(pSession->Socket);
        return;
    }

    int iRet = recv(pSession->Socket, pSession->RecvQ.GetRearBufferPtr(), iDirectSize, 0);
    if (iRet == 0)
    {
        DisconnectSession(pSession->Socket);
        return;
    }

    if (iRet == SOCKET_ERROR)
    {
        int iError = WSAGetLastError();
        if (iError == WSAEWOULDBLOCK)
        {
            goto PACKET_PROC;
        }

        DisconnectSession(pSession->Socket);
        return;
    }

    pSession->RecvQ.MoveRear(iRet);
    pSession->dwLastRecvTime = timeGetTime();

PACKET_PROC:
    int iProcessed = 0;
    while (true)
    {
        if (pSession->RecvQ.GetUseSize() < 3)
        {
            break;
        }

        st_PACKET_HEADER header;
        if (pSession->RecvQ.Peek(reinterpret_cast<char*>(&header), 3) != 3)
        {
            break;
        }

        if (header.byCode != dfPACKET_CODE)
        {
            _LOG(dfLOG_LEVEL_ERROR, "Invalid packet code 0x%X SessionID:%lu",
                header.byCode,
                static_cast<unsigned long>(pSession->dwSessionID));
            DisconnectSession(pSession->Socket);
            return;
        }

        int iTotalSize = 3 + header.bySize;
        if (pSession->RecvQ.GetUseSize() < iTotalSize)
        {
            break;
        }

        char szHeader[3];
        pSession->RecvQ.Dequeue(szHeader, 3);

        CPacket packet;
        if (header.bySize > 0)
        {
            char payload[512];
            pSession->RecvQ.Dequeue(payload, header.bySize);
            packet.PutData(payload, header.bySize);
        }

        if (!PacketProc(pSession, header.byType, &packet))
        {
            DisconnectSession(pSession->Socket);
            return;
        }
        ++g_dwMonitorRecvPacketCount;
        ++iProcessed;

        if (iProcessed >= dfPACKET_PROC_LIMIT)
        {
            _LOG(dfLOG_LEVEL_ERROR, "Packet flood detected. SessionID:%lu Processed:%d",
                static_cast<unsigned long>(pSession->dwSessionID), iProcessed);
            DisconnectSession(pSession->Socket);
            return;
        }
    }
}

static void netSendProc(st_SESSION *pSession)
{
    while (pSession->SendQ.GetUseSize() > 0)
    {
        int iSendSize = pSession->SendQ.DirectDequeueSize();
        if (iSendSize <= 0)
        {
            break;
        }

        int iRet = send(pSession->Socket, pSession->SendQ.GetFrontBufferPtr(), iSendSize, 0);
        if (iRet == SOCKET_ERROR)
        {
            int iError = WSAGetLastError();
            if (iError == WSAEWOULDBLOCK)
            {
                return;
            }

            DisconnectSession(pSession->Socket);
            return;
        }

        if (iRet <= 0)
        {
            DisconnectSession(pSession->Socket);
            return;
        }

        pSession->SendQ.MoveFront(iRet);
    }
}

bool netStartUp()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return false;
    }

    g_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_ListenSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return false;
    }

    BOOL bReuse = TRUE;
    setsockopt(g_ListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&bReuse), sizeof(bReuse));
    SetNoDelay(g_ListenSocket);
    SetNonBlocking(g_ListenSocket);

    SOCKADDR_IN listenAddr;
    ZeroMemory(&listenAddr, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(dfNETWORK_PORT);

    if (bind(g_ListenSocket, reinterpret_cast<SOCKADDR *>(&listenAddr), sizeof(listenAddr)) == SOCKET_ERROR)
    {
        closesocket(g_ListenSocket);
        g_ListenSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    if (listen(g_ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(g_ListenSocket);
        g_ListenSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    return true;
}

void netProcess()
{
    static std::vector<st_SESSION*> groupSessions;
    const std::vector<st_SESSION *> &sessions = g_Sessions;

    size_t index = 0;
    bool bFirstGroup = true;

    while (index < sessions.size() || bFirstGroup)
    {
        static fd_set readSet;
        static fd_set writeSet;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);

        groupSessions.clear();
        groupSessions.reserve(FD_SETSIZE);

        if (bFirstGroup)
        {
            FD_SET(g_ListenSocket, &readSet);
        }

        int iLimit = bFirstGroup ? FD_SETSIZE - 1 : FD_SETSIZE;
        while (index < sessions.size() && static_cast<int>(groupSessions.size()) < iLimit)
        {
            st_SESSION *pSession = sessions[index++];
            groupSessions.push_back(pSession);
            FD_SET(pSession->Socket, &readSet);
            if (pSession->SendQ.GetUseSize() > 0)
            {
                FD_SET(pSession->Socket, &writeSet);
            }
        }

        TIMEVAL timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        int iRet = select(0, &readSet, &writeSet, nullptr, &timeout);
        if (iRet != SOCKET_ERROR)
        {
            if (bFirstGroup && FD_ISSET(g_ListenSocket, &readSet))
            {
                netAcceptProc();
            }

            for (st_SESSION *pSession : groupSessions)
            {
                if (g_SessionMap.find(pSession->Socket) == g_SessionMap.end()
                    || g_DisconnectReserve.count(pSession->Socket) > 0)
                {
                    continue;
                }

                if (FD_ISSET(pSession->Socket, &readSet))
                {
                    netRecvProc(pSession);
                }

                if (g_SessionMap.find(pSession->Socket) == g_SessionMap.end()
                    || g_DisconnectReserve.count(pSession->Socket) > 0)
                {
                    continue;
                }

                if (FD_ISSET(pSession->Socket, &writeSet))
                {
                    netSendProc(pSession);
                }
            }
        }

        bFirstGroup = false;
    }

    ProcessDisconnects();
}

void netCleanUp()
{
    std::vector<SOCKET> sockets;
    sockets.reserve(g_SessionMap.size());

    for (auto iter = g_SessionMap.begin(); iter != g_SessionMap.end(); ++iter)
    {
        sockets.push_back(iter->first);
    }

    for (SOCKET sock : sockets)
    {
        DisconnectSession(sock);
    }

    ProcessDisconnects();

    if (g_ListenSocket != INVALID_SOCKET)
    {
        closesocket(g_ListenSocket);
        g_ListenSocket = INVALID_SOCKET;
    }

    WSACleanup();

    if (g_SessionPool.GetAllocCount() > 0 || g_CharacterPool.GetAllocCount() > 0)
    {
        std::printf("[WARN] Leaked: Session=%d, Character=%d\n",
            g_SessionPool.GetAllocCount(), g_CharacterPool.GetAllocCount());
    }
}

void DisconnectSession(SOCKET sock)
{
    if (g_SessionMap.find(sock) == g_SessionMap.end())
    {
        return;
    }

    g_DisconnectReserve.insert(sock);
}

void SendPacket_Unicast(st_SESSION *pSession, CPacket *pPacket)
{
    if (pSession == nullptr || pPacket == nullptr)
    {
        return;
    }

    int iDataSize = pPacket->GetDataSize();
    if (pSession->SendQ.Enqueue(pPacket->GetBufferPtr(), iDataSize) != iDataSize)
    {
        _LOG(dfLOG_LEVEL_ERROR, "SendQ Full! SessionID:%lu SendQ Used:%d Free:%d PacketSize:%d",
            static_cast<unsigned long>(pSession->dwSessionID),
            pSession->SendQ.GetUseSize(),
            pSession->SendQ.GetFreeSize(),
            iDataSize);
        DisconnectSession(pSession->Socket);
        return;
    }

    ++g_dwMonitorSendPacketCount;
}

void SendPacket_SectorOne(int iSectorX, int iSectorY, CPacket *pPacket, st_SESSION *pExceptSession)
{
    if (iSectorX < 0 || iSectorX >= dfSECTOR_MAX_X || iSectorY < 0 || iSectorY >= dfSECTOR_MAX_Y)
    {
        return;
    }

    const std::list<st_CHARACTER *> &sector = g_Sector[iSectorY][iSectorX];
    for (st_CHARACTER *pChar : sector)
    {
        if (pChar == nullptr || pChar->pSession == nullptr)
        {
            continue;
        }

        if (pExceptSession != nullptr && pChar->pSession == pExceptSession)
        {
            continue;
        }

        SendPacket_Unicast(pChar->pSession, pPacket);
    }
}

void SendPacket_Around(st_SESSION *pSession, CPacket *pPacket, bool bSendMe)
{
    if (pSession == nullptr || pPacket == nullptr)
    {
        return;
    }

    st_CHARACTER *pChar = FindCharacter(pSession->dwSessionID);
    if (pChar == nullptr)
    {
        return;
    }

    st_SECTOR_AROUND around;
    GetSectorAround(pChar->CurSector.iX, pChar->CurSector.iY, &around);

    for (int i = 0; i < around.iCount; ++i)
    {
        SendPacket_SectorOne(around.Around[i].iX, around.Around[i].iY, pPacket, bSendMe ? nullptr : pSession);
    }
}
