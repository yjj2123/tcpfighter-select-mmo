#include "PacketMake.h"

#include "Define.h"
#include "Protocol.h"

static void PutHeader(CPacket *pPacket, BYTE byType, BYTE bySize)
{
    pPacket->Clear();
    *pPacket << static_cast<BYTE>(dfPACKET_CODE);
    *pPacket << bySize;
    *pPacket << byType;
}

void mpCreateMyCharacter(CPacket *pPacket, DWORD id, BYTE dir, short x, short y, char hp)
{
    PutHeader(pPacket, dfPACKET_SC_CREATE_MY_CHARACTER, 10);
    *pPacket << id << dir << x << y << hp;
}

void mpCreateOtherCharacter(CPacket *pPacket, DWORD id, BYTE dir, short x, short y, char hp)
{
    PutHeader(pPacket, dfPACKET_SC_CREATE_OTHER_CHARACTER, 10);
    *pPacket << id << dir << x << y << hp;
}

void mpDeleteCharacter(CPacket *pPacket, DWORD id)
{
    PutHeader(pPacket, dfPACKET_SC_DELETE_CHARACTER, 4);
    *pPacket << id;
}

void mpMoveStart(CPacket *pPacket, DWORD id, BYTE dir, short x, short y)
{
    PutHeader(pPacket, dfPACKET_SC_MOVE_START, 9);
    *pPacket << id << dir << x << y;
}

void mpMoveStop(CPacket *pPacket, DWORD id, BYTE dir, short x, short y)
{
    PutHeader(pPacket, dfPACKET_SC_MOVE_STOP, 9);
    *pPacket << id << dir << x << y;
}

void mpAttack1(CPacket *pPacket, DWORD id, BYTE dir, short x, short y)
{
    PutHeader(pPacket, dfPACKET_SC_ATTACK1, 9);
    *pPacket << id << dir << x << y;
}

void mpAttack2(CPacket *pPacket, DWORD id, BYTE dir, short x, short y)
{
    PutHeader(pPacket, dfPACKET_SC_ATTACK2, 9);
    *pPacket << id << dir << x << y;
}

void mpAttack3(CPacket *pPacket, DWORD id, BYTE dir, short x, short y)
{
    PutHeader(pPacket, dfPACKET_SC_ATTACK3, 9);
    *pPacket << id << dir << x << y;
}

void mpDamage(CPacket *pPacket, DWORD attackID, DWORD damageID, char hp)
{
    PutHeader(pPacket, dfPACKET_SC_DAMAGE, 9);
    *pPacket << attackID << damageID << hp;
}

void mpSync(CPacket *pPacket, DWORD id, short x, short y)
{
    PutHeader(pPacket, dfPACKET_SC_SYNC, 8);
    *pPacket << id << x << y;
}

void mpEcho(CPacket *pPacket, DWORD time)
{
    PutHeader(pPacket, dfPACKET_SC_ECHO, 4);
    *pPacket << time;
}
