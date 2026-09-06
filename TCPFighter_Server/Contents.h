#pragma once

#include "Network.h"

void CreateCharacter(DWORD dwSessionID, st_SESSION *pSession);
void DeleteCharacter(DWORD dwSessionID);
st_CHARACTER *FindCharacter(DWORD dwSessionID);
void Update();
bool CharacterMoveCheck(short shX, short shY);
bool Sector_UpdateCharacter(st_CHARACTER *pChar);
void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND *pAround);
void GetUpdateSectorAround(st_CHARACTER *pChar, st_SECTOR_AROUND *pRemove, st_SECTOR_AROUND *pAdd);
void CharacterSectorUpdatePacket(st_CHARACTER *pChar);
bool Contents_ProcessAttack1(st_SESSION *pSession, BYTE byDirection, short shX, short shY);
bool Contents_ProcessAttack2(st_SESSION *pSession, BYTE byDirection, short shX, short shY);
bool Contents_ProcessAttack3(st_SESSION *pSession, BYTE byDirection, short shX, short shY);
void ServerControl();
void Monitor();
