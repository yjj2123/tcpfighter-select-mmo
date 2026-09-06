#include "CPacket.h"

#include <cstring>

CPacket::CPacket()
    : m_iDataSize(0),
      m_iReadPos(0)
{
    std::memset(m_pBuffer, 0, sizeof(m_pBuffer));
}

void CPacket::Clear()
{
    m_iDataSize = 0;
    m_iReadPos = 0;
}

int CPacket::GetDataSize() const
{
    return m_iDataSize;
}

char *CPacket::GetBufferPtr()
{
    return m_pBuffer;
}

const char *CPacket::GetBufferPtr() const
{
    return m_pBuffer;
}

int CPacket::PutData(const char *pSrc, int iSize)
{
    if (pSrc == nullptr || iSize <= 0)
    {
        return 0;
    }

    int iWritable = static_cast<int>(sizeof(m_pBuffer)) - m_iDataSize;
    if (iSize > iWritable)
    {
        iSize = iWritable;
    }

    if (iSize <= 0)
    {
        return 0;
    }

    std::memcpy(m_pBuffer + m_iDataSize, pSrc, iSize);
    m_iDataSize += iSize;
    return iSize;
}

int CPacket::GetData(char *pDest, int iSize)
{
    if (pDest == nullptr || iSize <= 0)
    {
        return 0;
    }

    int iReadable = m_iDataSize - m_iReadPos;
    if (iSize > iReadable)
    {
        iSize = iReadable;
    }

    if (iSize <= 0)
    {
        return 0;
    }

    std::memcpy(pDest, m_pBuffer + m_iReadPos, iSize);
    m_iReadPos += iSize;
    return iSize;
}

CPacket &CPacket::operator<<(BYTE value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(char value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(short value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(WORD value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(int value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(DWORD value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(float value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator<<(double value)
{
    PutValue(value);
    return *this;
}

CPacket &CPacket::operator>>(BYTE &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(char &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(short &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(WORD &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(int &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(DWORD &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(float &value)
{
    GetValue(value);
    return *this;
}

CPacket &CPacket::operator>>(double &value)
{
    GetValue(value);
    return *this;
}
