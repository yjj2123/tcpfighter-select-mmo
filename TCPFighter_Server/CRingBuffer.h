#pragma once

#include "Define.h"

class CRingBuffer
{
public:
    CRingBuffer(int iBufferSize = dfRINGBUFFER_SIZE);
    ~CRingBuffer();

    int GetUseSize() const;
    int GetFreeSize() const;
    int Enqueue(const char *pData, int iSize);
    int Dequeue(char *pDest, int iSize);
    int Peek(char *pDest, int iSize) const;
    void MoveRear(int iSize);
    void MoveFront(int iSize);
    void ClearBuffer();
    char *GetFrontBufferPtr();
    char *GetRearBufferPtr();
    int DirectEnqueueSize() const;
    int DirectDequeueSize() const;

private:
    char *m_pBuffer;
    int m_iBufferSize;
    int m_iFront;
    int m_iRear;
};
