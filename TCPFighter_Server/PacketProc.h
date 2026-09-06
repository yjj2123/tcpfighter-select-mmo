#pragma once

#include "Contents.h"

bool PacketProc(st_SESSION *pSession, BYTE byType, CPacket *pPacket);
bool netPacketProc_MoveStart(st_SESSION *pSession, CPacket *pPacket);
bool netPacketProc_MoveStop(st_SESSION *pSession, CPacket *pPacket);
bool netPacketProc_Attack1(st_SESSION *pSession, CPacket *pPacket);
bool netPacketProc_Attack2(st_SESSION *pSession, CPacket *pPacket);
bool netPacketProc_Attack3(st_SESSION *pSession, CPacket *pPacket);
bool netPacketProc_Echo(st_SESSION *pSession, CPacket *pPacket);
