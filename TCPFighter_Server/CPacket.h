#pragma once

#include <Windows.h>

#include "Define.h"

class CPacket
{
public:
    CPacket();

    void Clear();
    int GetDataSize() const;
    char *GetBufferPtr();
    const char *GetBufferPtr() const;
    int PutData(const char *pSrc, int iSize);
    int GetData(char *pDest, int iSize);

    CPacket &operator<<(BYTE value);
    CPacket &operator<<(char value);
    CPacket &operator<<(short value);
    CPacket &operator<<(WORD value);
    CPacket &operator<<(int value);
    CPacket &operator<<(DWORD value);
    CPacket &operator<<(float value);
    CPacket &operator<<(double value);

    CPacket &operator>>(BYTE &value);
    CPacket &operator>>(char &value);
    CPacket &operator>>(short &value);
    CPacket &operator>>(WORD &value);
    CPacket &operator>>(int &value);
    CPacket &operator>>(DWORD &value);
    CPacket &operator>>(float &value);
    CPacket &operator>>(double &value);

private:
    template <typename T>
    int PutValue(const T &value)
    {
        return PutData(reinterpret_cast<const char *>(&value), static_cast<int>(sizeof(T)));
    }

    template <typename T>
    int GetValue(T &value)
    {
        return GetData(reinterpret_cast<char *>(&value), static_cast<int>(sizeof(T)));
    }

private:
    char m_pBuffer[dfPACKET_BUFFER_SIZE];
    int m_iDataSize;
    int m_iReadPos;
};
