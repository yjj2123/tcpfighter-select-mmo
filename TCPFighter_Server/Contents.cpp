#include "Contents.h"

#include <algorithm>
#include <conio.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "PacketMake.h"
#include "Protocol.h"

static int ClampSectorIndexX(int value)
{
    if (value < 0)
    {
        return 0;
    }

    if (value >= dfSECTOR_MAX_X)
    {
        return dfSECTOR_MAX_X - 1;
    }

    return value;
}

static int ClampSectorIndexY(int value)
{
    if (value < 0)
    {
        return 0;
    }

    if (value >= dfSECTOR_MAX_Y)
    {
        return dfSECTOR_MAX_Y - 1;
    }

    return value;
}

static st_SECTOR_POS MakeSectorPos(short shX, short shY)
{
    st_SECTOR_POS pos;
    pos.iX = ClampSectorIndexX(shX / dfSECTOR_SIZE_X);
    pos.iY = ClampSectorIndexY(shY / dfSECTOR_SIZE_Y);
    return pos;
}

static bool IsSameSector(const st_SECTOR_POS &lhs, const st_SECTOR_POS &rhs)
{
    return lhs.iX == rhs.iX && lhs.iY == rhs.iY;
}

static bool SectorListContains(const st_SECTOR_AROUND *pAround, int iX, int iY)
{
    for (int i = 0; i < pAround->iCount; ++i)
    {
        if (pAround->Around[i].iX == iX && pAround->Around[i].iY == iY)
        {
            return true;
        }
    }

    return false;
}

static void RemoveFromSector(st_CHARACTER *pChar, const st_SECTOR_POS &pos)
{
    g_Sector[pos.iY][pos.iX].erase(pChar->SectorIter);
}

static void AddToSector(st_CHARACTER *pChar, const st_SECTOR_POS &pos)
{
    std::list<st_CHARACTER *> &sector = g_Sector[pos.iY][pos.iX];
    sector.push_back(pChar);
    auto it = sector.end();
    --it;
    pChar->SectorIter = it;
}

static void SendCreateCharacterPackets(st_SESSION *pSession, st_CHARACTER *pChar, bool bSendMove)
{
    CPacket packet;
    mpCreateOtherCharacter(&packet, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
    SendPacket_Unicast(pSession, &packet);

    if (bSendMove && pChar->dwAction != dfACTION_STAND)
    {
        packet.Clear();
        mpMoveStart(&packet, pChar->dwSessionID, pChar->byMoveDirection, pChar->shX, pChar->shY);
        SendPacket_Unicast(pSession, &packet);
    }
}

static void ApplyMoveByDirection(st_CHARACTER *pChar)
{
    short shNewX = pChar->shX;
    short shNewY = pChar->shY;

    switch (pChar->byMoveDirection)
    {
    case dfPACKET_MOVE_DIR_LL:
        shNewX -= dfSPEED_PLAYER_X;
        break;
    case dfPACKET_MOVE_DIR_LU:
        shNewX -= dfSPEED_PLAYER_X;
        shNewY -= dfSPEED_PLAYER_Y;
        break;
    case dfPACKET_MOVE_DIR_UU:
        shNewY -= dfSPEED_PLAYER_Y;
        break;
    case dfPACKET_MOVE_DIR_RU:
        shNewX += dfSPEED_PLAYER_X;
        shNewY -= dfSPEED_PLAYER_Y;
        break;
    case dfPACKET_MOVE_DIR_RR:
        shNewX += dfSPEED_PLAYER_X;
        break;
    case dfPACKET_MOVE_DIR_RD:
        shNewX += dfSPEED_PLAYER_X;
        shNewY += dfSPEED_PLAYER_Y;
        break;
    case dfPACKET_MOVE_DIR_DD:
        shNewY += dfSPEED_PLAYER_Y;
        break;
    case dfPACKET_MOVE_DIR_LD:
        shNewX -= dfSPEED_PLAYER_X;
        shNewY += dfSPEED_PLAYER_Y;
        break;
    default:
        return;
    }

    if (shNewX < dfRANGE_MOVE_LEFT)
    {
        shNewX = dfRANGE_MOVE_LEFT;
    }

    if (shNewX > dfRANGE_MOVE_RIGHT)
    {
        shNewX = dfRANGE_MOVE_RIGHT;
    }

    if (shNewY < dfRANGE_MOVE_TOP)
    {
        shNewY = dfRANGE_MOVE_TOP;
    }

    if (shNewY > dfRANGE_MOVE_BOTTOM)
    {
        shNewY = dfRANGE_MOVE_BOTTOM;
    }

    pChar->shX = shNewX;
    pChar->shY = shNewY;
}

static bool IsTargetInRange(st_CHARACTER *pAttacker, st_CHARACTER *pTarget, int iRangeX, int iRangeY)
{
    int iDiffX = static_cast<int>(pTarget->shX) - static_cast<int>(pAttacker->shX);
    int iDiffY = static_cast<int>(pTarget->shY) - static_cast<int>(pAttacker->shY);

    if (std::abs(iDiffY) > iRangeY)
    {
        return false;
    }

    switch (pAttacker->byDirection)
    {
    case dfPACKET_MOVE_DIR_LL:
        return (iDiffX < 0) && (-iDiffX <= iRangeX);
    case dfPACKET_MOVE_DIR_RR:
        return (iDiffX > 0) && (iDiffX <= iRangeX);
    default:
        break;
    }

    return false;
}

static bool ProcessAttack(st_SESSION *pSession, BYTE byDirection, short shX, short shY, int iRangeX, int iRangeY, int iDamage, BYTE byPacketType)
{
    st_CHARACTER *pChar = FindCharacter(pSession->dwSessionID);
    if (pChar == nullptr)
    {
        return false;
    }

    if (std::abs(pChar->shX - shX) > dfERROR_RANGE || std::abs(pChar->shY - shY) > dfERROR_RANGE)
    {
        CPacket syncPacket;
        mpSync(&syncPacket, pChar->dwSessionID, pChar->shX, pChar->shY);
        SendPacket_Around(pSession, &syncPacket, true);
        return true;
    }

    pChar->shX = shX;
    pChar->shY = shY;
    pChar->byDirection = byDirection;

    if (Sector_UpdateCharacter(pChar))
    {
        CharacterSectorUpdatePacket(pChar);
        pChar->OldSector = pChar->CurSector;
    }

    CPacket attackPacket;
    if (byPacketType == dfPACKET_SC_ATTACK1)
    {
        mpAttack1(&attackPacket, pChar->dwSessionID, byDirection, shX, shY);
    }
    else if (byPacketType == dfPACKET_SC_ATTACK2)
    {
        mpAttack2(&attackPacket, pChar->dwSessionID, byDirection, shX, shY);
    }
    else
    {
        mpAttack3(&attackPacket, pChar->dwSessionID, byDirection, shX, shY);
    }

    SendPacket_Around(pSession, &attackPacket, true);

    st_SECTOR_AROUND around;
    GetSectorAround(pChar->CurSector.iX, pChar->CurSector.iY, &around);

    std::vector<st_CHARACTER *> hitTargets;

    for (int i = 0; i < around.iCount; ++i)
    {
        const std::list<st_CHARACTER *> &sector = g_Sector[around.Around[i].iY][around.Around[i].iX];
        for (st_CHARACTER *pTarget : sector)
        {
            if (pTarget == pChar || pTarget->chHP <= 0)
            {
                continue;
            }

            if (IsTargetInRange(pChar, pTarget, iRangeX, iRangeY))
            {
                hitTargets.push_back(pTarget);
            }
        }
    }

    for (st_CHARACTER *pTarget : hitTargets)
    {
        pTarget->chHP -= static_cast<char>(iDamage);
        if (pTarget->chHP < 0)
        {
            pTarget->chHP = 0;
        }

        CPacket damagePacket;
        mpDamage(&damagePacket, pChar->dwSessionID, pTarget->dwSessionID, pTarget->chHP);
        SendPacket_Around(pTarget->pSession, &damagePacket, true);
    }
    return true;
}

void CreateCharacter(DWORD dwSessionID, st_SESSION *pSession)
{
    st_CHARACTER *pChar = g_CharacterPool.Alloc();
    if (pChar == nullptr)
    {
        return;
    }

    pChar->pSession = pSession;
    pChar->dwSessionID = dwSessionID;
    pChar->dwAction = dfACTION_STAND;
    pChar->byDirection = dfPACKET_MOVE_DIR_LL;
    pChar->byMoveDirection = dfPACKET_MOVE_DIR_LL;
    pChar->shX = static_cast<short>((std::rand() % (dfRANGE_MOVE_RIGHT - 200)) + 100);
    pChar->shY = static_cast<short>((std::rand() % (dfRANGE_MOVE_BOTTOM - 200)) + 100);
    pChar->chHP = dfINIT_HP;
    pChar->CurSector = MakeSectorPos(pChar->shX, pChar->shY);
    pChar->OldSector = pChar->CurSector;

    g_CharacterMap.insert(std::make_pair(dwSessionID, pChar));
    AddToSector(pChar, pChar->CurSector);

    CPacket packet;
    mpCreateMyCharacter(&packet, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
    SendPacket_Unicast(pSession, &packet);

    st_SECTOR_AROUND around;
    GetSectorAround(pChar->CurSector.iX, pChar->CurSector.iY, &around);

    for (int i = 0; i < around.iCount; ++i)
    {
        const std::list<st_CHARACTER *> &sector = g_Sector[around.Around[i].iY][around.Around[i].iX];
        for (st_CHARACTER *pOther : sector)
        {
            if (pOther == pChar)
            {
                continue;
            }

            SendCreateCharacterPackets(pSession, pOther, true);

            packet.Clear();
            mpCreateOtherCharacter(&packet, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
            SendPacket_Unicast(pOther->pSession, &packet);
        }
    }
}

void DeleteCharacter(DWORD dwSessionID)
{
    auto iter = g_CharacterMap.find(dwSessionID);
    if (iter == g_CharacterMap.end())
    {
        return;
    }

    st_CHARACTER *pChar = iter->second;

    CPacket packet;
    mpDeleteCharacter(&packet, pChar->dwSessionID);
    SendPacket_Around(pChar->pSession, &packet, false);

    RemoveFromSector(pChar, pChar->CurSector);
    g_CharacterMap.erase(iter);
    g_CharacterPool.Free(pChar);
}

st_CHARACTER *FindCharacter(DWORD dwSessionID)
{
    auto iter = g_CharacterMap.find(dwSessionID);
    if (iter == g_CharacterMap.end())
    {
        return nullptr;
    }

    return iter->second;
}

void Update()
{
    static DWORD dwLastUpdateTick = timeGetTime();
    DWORD dwNow = timeGetTime();
    DWORD dwElapsed = dwNow - dwLastUpdateTick;

    if (dwElapsed < dfFRAME_INTERVAL)
    {
        return;
    }

    int iFrameCount = dwElapsed / dfFRAME_INTERVAL;
    dwLastUpdateTick += iFrameCount * dfFRAME_INTERVAL;

    std::vector<st_CHARACTER*> characters;
    characters.reserve(g_CharacterMap.size());

    for (auto iter = g_CharacterMap.begin(); iter != g_CharacterMap.end(); ++iter)
    {
        characters.push_back(iter->second);
    }

    for (st_CHARACTER* pChar : characters)
    {
        if (pChar == nullptr || pChar->pSession == nullptr)
        {
            continue;
        }

        if (pChar->chHP <= 0)
        {
            pChar->chHP = dfINIT_HP;
            pChar->shX = static_cast<short>((std::rand() % (dfRANGE_MOVE_RIGHT - 200)) + 100);
            pChar->shY = static_cast<short>((std::rand() % (dfRANGE_MOVE_BOTTOM - 200)) + 100);
            pChar->dwAction = dfACTION_STAND;

            if (Sector_UpdateCharacter(pChar))
            {
                CharacterSectorUpdatePacket(pChar);
            }
            pChar->OldSector = pChar->CurSector;

            CPacket packet;
            mpSync(&packet, pChar->dwSessionID, pChar->shX, pChar->shY);
            SendPacket_Around(pChar->pSession, &packet, true);
            continue;
        }

        if (dwNow - pChar->pSession->dwLastRecvTime > dfNETWORK_PACKET_RECV_TIMEOUT)
        {
            DisconnectSession(pChar->pSession->Socket);
            continue;
        }

        if (pChar->dwAction != dfACTION_STAND)
        {
            for (int f = 0; f < iFrameCount; ++f)
            {
                ApplyMoveByDirection(pChar);
            }

            if (Sector_UpdateCharacter(pChar))
            {
                CharacterSectorUpdatePacket(pChar);
            }
            pChar->OldSector = pChar->CurSector;
        }
    }
}

bool CharacterMoveCheck(short shX, short shY)
{
    if (shX < dfRANGE_MOVE_LEFT || shX > dfRANGE_MOVE_RIGHT)
    {
        return false;
    }

    if (shY < dfRANGE_MOVE_TOP || shY > dfRANGE_MOVE_BOTTOM)
    {
        return false;
    }

    return true;
}

bool Sector_UpdateCharacter(st_CHARACTER *pChar)
{
    if (pChar == nullptr)
    {
        return false;
    }

    pChar->CurSector = MakeSectorPos(pChar->shX, pChar->shY);

    if (IsSameSector(pChar->OldSector, pChar->CurSector))
    {
        return false;
    }

    RemoveFromSector(pChar, pChar->OldSector);
    AddToSector(pChar, pChar->CurSector);
    return true;
}

void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND *pAround)
{
    pAround->iCount = 0;

    for (int y = iSectorY - 1; y <= iSectorY + 1; ++y)
    {
        if (y < 0 || y >= dfSECTOR_MAX_Y)
        {
            continue;
        }

        for (int x = iSectorX - 1; x <= iSectorX + 1; ++x)
        {
            if (x < 0 || x >= dfSECTOR_MAX_X)
            {
                continue;
            }

            pAround->Around[pAround->iCount].iX = x;
            pAround->Around[pAround->iCount].iY = y;
            ++pAround->iCount;
        }
    }
}

void GetUpdateSectorAround(st_CHARACTER *pChar, st_SECTOR_AROUND *pRemove, st_SECTOR_AROUND *pAdd)
{
    pRemove->iCount = 0;
    pAdd->iCount = 0;

    st_SECTOR_AROUND oldAround;
    st_SECTOR_AROUND curAround;
    GetSectorAround(pChar->OldSector.iX, pChar->OldSector.iY, &oldAround);
    GetSectorAround(pChar->CurSector.iX, pChar->CurSector.iY, &curAround);

    for (int i = 0; i < oldAround.iCount; ++i)
    {
        if (!SectorListContains(&curAround, oldAround.Around[i].iX, oldAround.Around[i].iY))
        {
            pRemove->Around[pRemove->iCount++] = oldAround.Around[i];
        }
    }

    for (int i = 0; i < curAround.iCount; ++i)
    {
        if (!SectorListContains(&oldAround, curAround.Around[i].iX, curAround.Around[i].iY))
        {
            pAdd->Around[pAdd->iCount++] = curAround.Around[i];
        }
    }
}

void CharacterSectorUpdatePacket(st_CHARACTER *pChar)
{
    st_SECTOR_AROUND removeAround;
    st_SECTOR_AROUND addAround;
    GetUpdateSectorAround(pChar, &removeAround, &addAround);

    CPacket packet;

    for (int i = 0; i < removeAround.iCount; ++i)
    {
        const std::list<st_CHARACTER *> &sector = g_Sector[removeAround.Around[i].iY][removeAround.Around[i].iX];
        for (st_CHARACTER *pOther : sector)
        {
            if (pOther == pChar)
            {
                continue;
            }

            packet.Clear();
            mpDeleteCharacter(&packet, pChar->dwSessionID);
            SendPacket_Unicast(pOther->pSession, &packet);

            packet.Clear();
            mpDeleteCharacter(&packet, pOther->dwSessionID);
            SendPacket_Unicast(pChar->pSession, &packet);
        }
    }

    for (int i = 0; i < addAround.iCount; ++i)
    {
        const std::list<st_CHARACTER *> &sector = g_Sector[addAround.Around[i].iY][addAround.Around[i].iX];
        for (st_CHARACTER *pOther : sector)
        {
            if (pOther == pChar)
            {
                continue;
            }

            packet.Clear();
            mpCreateOtherCharacter(&packet, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
            SendPacket_Unicast(pOther->pSession, &packet);

            if (pChar->dwAction != dfACTION_STAND)
            {
                packet.Clear();
                mpMoveStart(&packet, pChar->dwSessionID, pChar->byMoveDirection, pChar->shX, pChar->shY);
                SendPacket_Unicast(pOther->pSession, &packet);
            }

            SendCreateCharacterPackets(pChar->pSession, pOther, true);
        }
    }
}

bool Contents_ProcessAttack1(st_SESSION *pSession, BYTE byDirection, short shX, short shY)
{
    return ProcessAttack(pSession, byDirection, shX, shY, dfATTACK1_RANGE_X, dfATTACK1_RANGE_Y, dfATTACK1_DAMAGE, dfPACKET_SC_ATTACK1);
}

bool Contents_ProcessAttack2(st_SESSION *pSession, BYTE byDirection, short shX, short shY)
{
    return ProcessAttack(pSession, byDirection, shX, shY, dfATTACK2_RANGE_X, dfATTACK2_RANGE_Y, dfATTACK2_DAMAGE, dfPACKET_SC_ATTACK2);
}

bool Contents_ProcessAttack3(st_SESSION *pSession, BYTE byDirection, short shX, short shY)
{
    return ProcessAttack(pSession, byDirection, shX, shY, dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y, dfATTACK3_DAMAGE, dfPACKET_SC_ATTACK3);
}

void ServerControl()
{
    static bool bControlMode = false;

    if (!_kbhit())
    {
        return;
    }

    int ch = _getch();

    if (ch == 'u' || ch == 'U')
    {
        bControlMode = true;
        std::printf("Control Mode : Press Q - Quit\n");
        std::printf("Control Mode : Press L - Key Lock\n");
        return;
    }

    if ((ch == 'l' || ch == 'L') && bControlMode)
    {
        std::printf("Control Lock..! Press U - Control Unlock\n");
        bControlMode = false;
        return;
    }

    if ((ch == 'q' || ch == 'Q') && bControlMode)
    {
        g_bShutdown = true;
    }
}

void Monitor()
{
    static DWORD dwLastMonitorTick = timeGetTime();
    DWORD dwNow = timeGetTime();
    if (dwNow - dwLastMonitorTick < 1000)
    {
        return;
    }

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(hConsole, coord);

    std::printf("========================================\n");
    std::printf(" TCPFighter Server Monitor              \n");
    std::printf("========================================\n");
    std::printf(" Session Count    : %d / %d\n",
        static_cast<int>(g_SessionMap.size()), dfSESSION_MAX);
    std::printf(" Character Count  : %d / %d\n",
        static_cast<int>(g_CharacterMap.size()), dfSESSION_MAX);
    std::printf(" Loop/sec         : %lu                  \n", static_cast<unsigned long>(g_dwMonitorLoopCount));
    std::printf(" Accept/sec       : %lu                  \n", static_cast<unsigned long>(g_dwMonitorAcceptCount));
    std::printf(" Disconnect/sec   : %lu                  \n", static_cast<unsigned long>(g_dwMonitorDisconnectCount));
    std::printf(" RecvPacket/sec   : %lu                  \n", static_cast<unsigned long>(g_dwMonitorRecvPacketCount));
    std::printf(" SendPacket/sec   : %lu                  \n", static_cast<unsigned long>(g_dwMonitorSendPacketCount));
    std::printf(" Session Pool     : %d / %d              \n", g_SessionPool.GetAllocCount(), g_SessionPool.GetCapacity());
    std::printf(" Character Pool   : %d / %d              \n", g_CharacterPool.GetAllocCount(), g_CharacterPool.GetCapacity());
    std::printf("========================================\n");

    g_dwMonitorLoopCount = 0;
    g_dwMonitorAcceptCount = 0;
    g_dwMonitorDisconnectCount = 0;
    g_dwMonitorRecvPacketCount = 0;
    g_dwMonitorSendPacketCount = 0;
    dwLastMonitorTick = dwNow;
}
