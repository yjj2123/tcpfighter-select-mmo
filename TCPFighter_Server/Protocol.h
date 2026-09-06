#pragma once

#include <Windows.h>

#pragma pack(push, 1)
struct st_PACKET_HEADER
{
    BYTE byCode;
    BYTE bySize;
    BYTE byType;
};
#pragma pack(pop)

enum
{
    dfPACKET_SC_CREATE_MY_CHARACTER = 0,
    dfPACKET_SC_CREATE_OTHER_CHARACTER = 1,
    dfPACKET_SC_DELETE_CHARACTER = 2,
    dfPACKET_CS_MOVE_START = 10,
    dfPACKET_SC_MOVE_START = 11,
    dfPACKET_CS_MOVE_STOP = 12,
    dfPACKET_SC_MOVE_STOP = 13,
    dfPACKET_CS_ATTACK1 = 20,
    dfPACKET_SC_ATTACK1 = 21,
    dfPACKET_CS_ATTACK2 = 22,
    dfPACKET_SC_ATTACK2 = 23,
    dfPACKET_CS_ATTACK3 = 24,
    dfPACKET_SC_ATTACK3 = 25,
    dfPACKET_SC_DAMAGE = 30,
    dfPACKET_SC_SYNC = 251,
    dfPACKET_CS_ECHO = 252,
    dfPACKET_SC_ECHO = 253
};

enum
{
    dfPACKET_MOVE_DIR_LL = 0,
    dfPACKET_MOVE_DIR_LU = 1,
    dfPACKET_MOVE_DIR_UU = 2,
    dfPACKET_MOVE_DIR_RU = 3,
    dfPACKET_MOVE_DIR_RR = 4,
    dfPACKET_MOVE_DIR_RD = 5,
    dfPACKET_MOVE_DIR_DD = 6,
    dfPACKET_MOVE_DIR_LD = 7
};
