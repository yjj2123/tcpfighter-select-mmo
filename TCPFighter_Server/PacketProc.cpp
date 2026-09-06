#include "PacketProc.h"

#include <cstdlib>

#include "PacketMake.h"
#include "Protocol.h"

static bool IsPositionError(st_CHARACTER *pChar, short shX, short shY)
{
    return std::abs(pChar->shX - shX) > dfERROR_RANGE || std::abs(pChar->shY - shY) > dfERROR_RANGE;
}

bool PacketProc(st_SESSION *pSession, BYTE byType, CPacket *pPacket)
{
    switch (byType)
    {
    case dfPACKET_CS_MOVE_START:
        return netPacketProc_MoveStart(pSession, pPacket);
    case dfPACKET_CS_MOVE_STOP:
        return netPacketProc_MoveStop(pSession, pPacket);
    case dfPACKET_CS_ATTACK1:
        return netPacketProc_Attack1(pSession, pPacket);
    case dfPACKET_CS_ATTACK2:
        return netPacketProc_Attack2(pSession, pPacket);
    case dfPACKET_CS_ATTACK3:
        return netPacketProc_Attack3(pSession, pPacket);
    case dfPACKET_CS_ECHO:
        return netPacketProc_Echo(pSession, pPacket);
    default:
        break;
    }
    return true;
}

bool netPacketProc_MoveStart(st_SESSION *pSession, CPacket *pPacket)
{
    BYTE byDirection;
    short shX;
    short shY;
    *pPacket >> byDirection >> shX >> shY;

    st_CHARACTER *pChar = FindCharacter(pSession->dwSessionID);
    if (pChar == nullptr)
    {
        return false;
    }

    if (IsPositionError(pChar, shX, shY))
    {
        CPacket syncPacket;
        mpSync(&syncPacket, pChar->dwSessionID, pChar->shX, pChar->shY);
        SendPacket_Around(pSession, &syncPacket, true);
        shX = pChar->shX;
        shY = pChar->shY;
    }

    pChar->shX = shX;
    pChar->shY = shY;
    pChar->byMoveDirection = byDirection;
    pChar->dwAction = byDirection;

    switch (byDirection)
    {
    case dfPACKET_MOVE_DIR_RR:
    case dfPACKET_MOVE_DIR_RU:
    case dfPACKET_MOVE_DIR_RD:
        pChar->byDirection = dfPACKET_MOVE_DIR_RR;
        break;
    case dfPACKET_MOVE_DIR_LL:
    case dfPACKET_MOVE_DIR_LU:
    case dfPACKET_MOVE_DIR_LD:
        pChar->byDirection = dfPACKET_MOVE_DIR_LL;
        break;
    default:
        break;
    }

    if (Sector_UpdateCharacter(pChar))
    {
        CharacterSectorUpdatePacket(pChar);
    }
    pChar->OldSector = pChar->CurSector;

    CPacket packet;
    mpMoveStart(&packet, pChar->dwSessionID, byDirection, shX, shY);
    SendPacket_Around(pSession, &packet, true);
    return true;
}

bool netPacketProc_MoveStop(st_SESSION *pSession, CPacket *pPacket)
{
    BYTE byDirection;
    short shX;
    short shY;
    *pPacket >> byDirection >> shX >> shY;

    st_CHARACTER *pChar = FindCharacter(pSession->dwSessionID);
    if (pChar == nullptr)
    {
        return false;
    }

    if (IsPositionError(pChar, shX, shY))
    {
        CPacket syncPacket;
        mpSync(&syncPacket, pChar->dwSessionID, pChar->shX, pChar->shY);
        SendPacket_Around(pSession, &syncPacket, true);
        shX = pChar->shX;
        shY = pChar->shY;
    }

    pChar->shX = shX;
    pChar->shY = shY;
    pChar->byDirection = byDirection;
    pChar->dwAction = dfACTION_STAND;

    if (Sector_UpdateCharacter(pChar))
    {
        CharacterSectorUpdatePacket(pChar);
    }
    pChar->OldSector = pChar->CurSector;

    CPacket packet;
    mpMoveStop(&packet, pChar->dwSessionID, byDirection, shX, shY);
    SendPacket_Around(pSession, &packet, true);
    return true;
}

bool netPacketProc_Attack1(st_SESSION *pSession, CPacket *pPacket)
{
    BYTE byDirection;
    short shX;
    short shY;
    *pPacket >> byDirection >> shX >> shY;
    return Contents_ProcessAttack1(pSession, byDirection, shX, shY);
}

bool netPacketProc_Attack2(st_SESSION *pSession, CPacket *pPacket)
{
    BYTE byDirection;
    short shX;
    short shY;
    *pPacket >> byDirection >> shX >> shY;
    return Contents_ProcessAttack2(pSession, byDirection, shX, shY);
}

bool netPacketProc_Attack3(st_SESSION *pSession, CPacket *pPacket)
{
    BYTE byDirection;
    short shX;
    short shY;
    *pPacket >> byDirection >> shX >> shY;
    return Contents_ProcessAttack3(pSession, byDirection, shX, shY);
}

bool netPacketProc_Echo(st_SESSION *pSession, CPacket *pPacket)
{
    DWORD dwTime;
    *pPacket >> dwTime;

    CPacket packet;
    mpEcho(&packet, dwTime);
    SendPacket_Unicast(pSession, &packet);
    return true;
}
