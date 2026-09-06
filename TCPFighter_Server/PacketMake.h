#pragma once

#include "CPacket.h"

void mpCreateMyCharacter(CPacket *pPacket, DWORD id, BYTE dir, short x, short y, char hp);
void mpCreateOtherCharacter(CPacket *pPacket, DWORD id, BYTE dir, short x, short y, char hp);
void mpDeleteCharacter(CPacket *pPacket, DWORD id);
void mpMoveStart(CPacket *pPacket, DWORD id, BYTE dir, short x, short y);
void mpMoveStop(CPacket *pPacket, DWORD id, BYTE dir, short x, short y);
void mpAttack1(CPacket *pPacket, DWORD id, BYTE dir, short x, short y);
void mpAttack2(CPacket *pPacket, DWORD id, BYTE dir, short x, short y);
void mpAttack3(CPacket *pPacket, DWORD id, BYTE dir, short x, short y);
void mpDamage(CPacket *pPacket, DWORD attackID, DWORD damageID, char hp);
void mpSync(CPacket *pPacket, DWORD id, short x, short y);
void mpEcho(CPacket *pPacket, DWORD time);
