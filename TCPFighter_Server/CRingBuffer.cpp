#include "CRingBuffer.h"

#include <cstring>

CRingBuffer::CRingBuffer(int iBufferSize)
    : m_pBuffer(nullptr),
      m_iBufferSize(iBufferSize > 2 ? iBufferSize : dfRINGBUFFER_SIZE),
      m_iFront(0),
      m_iRear(0)
{
    m_pBuffer = new char[m_iBufferSize];
}

CRingBuffer::~CRingBuffer()
{
    delete[] m_pBuffer;
    m_pBuffer = nullptr;
}

int CRingBuffer::GetUseSize() const
{
    if (m_iRear >= m_iFront)
    {
        return m_iRear - m_iFront;
    }

    return (m_iBufferSize - m_iFront) + m_iRear;
}

int CRingBuffer::GetFreeSize() const
{
    return m_iBufferSize - GetUseSize() - 1;
}

int CRingBuffer::Enqueue(const char *pData, int iSize)
{
    if (pData == nullptr || iSize <= 0)
    {
        return 0;
    }

    int iEnqueueSize = iSize;
    int iFreeSize = GetFreeSize();

    if (iEnqueueSize > iFreeSize)
    {
        iEnqueueSize = iFreeSize;
    }

    int iDirectSize = DirectEnqueueSize();
    if (iEnqueueSize <= iDirectSize)
    {
        std::memcpy(m_pBuffer + m_iRear, pData, iEnqueueSize);
        MoveRear(iEnqueueSize);
        return iEnqueueSize;
    }

    std::memcpy(m_pBuffer + m_iRear, pData, iDirectSize);
    std::memcpy(m_pBuffer, pData + iDirectSize, iEnqueueSize - iDirectSize);
    MoveRear(iEnqueueSize);
    return iEnqueueSize;
}

int CRingBuffer::Dequeue(char *pDest, int iSize)
{
    if (pDest == nullptr || iSize <= 0)
    {
        return 0;
    }

    int iDequeueSize = iSize;
    int iUseSize = GetUseSize();

    if (iDequeueSize > iUseSize)
    {
        iDequeueSize = iUseSize;
    }

    int iDirectSize = DirectDequeueSize();
    if (iDequeueSize <= iDirectSize)
    {
        std::memcpy(pDest, m_pBuffer + m_iFront, iDequeueSize);
        MoveFront(iDequeueSize);
        return iDequeueSize;
    }

    std::memcpy(pDest, m_pBuffer + m_iFront, iDirectSize);
    std::memcpy(pDest + iDirectSize, m_pBuffer, iDequeueSize - iDirectSize);
    MoveFront(iDequeueSize);
    return iDequeueSize;
}

int CRingBuffer::Peek(char *pDest, int iSize) const
{
    if (pDest == nullptr || iSize <= 0)
    {
        return 0;
    }

    int iPeekSize = iSize;
    int iUseSize = GetUseSize();

    if (iPeekSize > iUseSize)
    {
        iPeekSize = iUseSize;
    }

    int iDirectSize = DirectDequeueSize();
    if (iPeekSize <= iDirectSize)
    {
        std::memcpy(pDest, m_pBuffer + m_iFront, iPeekSize);
        return iPeekSize;
    }

    std::memcpy(pDest, m_pBuffer + m_iFront, iDirectSize);
    std::memcpy(pDest + iDirectSize, m_pBuffer, iPeekSize - iDirectSize);
    return iPeekSize;
}

void CRingBuffer::MoveRear(int iSize)
{
    if (iSize <= 0)
    {
        return;
    }

    m_iRear = (m_iRear + iSize) % m_iBufferSize;
}

void CRingBuffer::MoveFront(int iSize)
{
    if (iSize <= 0)
    {
        return;
    }

    m_iFront = (m_iFront + iSize) % m_iBufferSize;
}

void CRingBuffer::ClearBuffer()
{
    m_iFront = 0;
    m_iRear = 0;
}

char *CRingBuffer::GetFrontBufferPtr()
{
    return m_pBuffer + m_iFront;
}

char *CRingBuffer::GetRearBufferPtr()
{
    return m_pBuffer + m_iRear;
}

int CRingBuffer::DirectEnqueueSize() const
{
    if (m_iRear >= m_iFront)
    {
        if (m_iFront == 0)
        {
            return m_iBufferSize - m_iRear - 1;
        }

        return m_iBufferSize - m_iRear;
    }

    return m_iFront - m_iRear - 1;
}

int CRingBuffer::DirectDequeueSize() const
{
    if (m_iRear >= m_iFront)
    {
        return m_iRear - m_iFront;
    }

    return m_iBufferSize - m_iFront;
}
