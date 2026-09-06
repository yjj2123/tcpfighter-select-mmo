#pragma once
#include <type_traits> 
#include <cstddef>


template <typename T>
class CFreeList
{
public:
    CFreeList(int iCapacity)
        : m_iCapacity(iCapacity),
          m_iAllocCount(0),
          m_pNodeArray(nullptr),
          m_pFreeHead(nullptr)
    {
        m_pNodeArray = new st_NODE[m_iCapacity];
        for (int i = m_iCapacity - 1; i >= 0; --i)
        {
            m_pNodeArray[i].pNext = m_pFreeHead;
            m_pFreeHead = &m_pNodeArray[i];
        }
    }

    ~CFreeList()
    {
        delete[] m_pNodeArray;
    }

    T *Alloc()
    {
        if (m_pFreeHead == nullptr)
        {
            return nullptr;
        }

        st_NODE *pNode = m_pFreeHead;
        m_pFreeHead = pNode->pNext;
        ++m_iAllocCount;
        return &pNode->Data;
    }

    void Free(T* pData)
    {
        if (pData == nullptr)
        {
            return;
        }

#pragma warning(push)
#pragma warning(disable: 4200 4075)
        st_NODE* pNode = reinterpret_cast<st_NODE*>(
            reinterpret_cast<char*>(pData) - offsetof(st_NODE, Data));
#pragma warning(pop)

        pNode->pNext = m_pFreeHead;
        m_pFreeHead = pNode;
        --m_iAllocCount;
    }

    int GetAllocCount() const
    {
        return m_iAllocCount;
    }

    int GetCapacity() const
    {
        return m_iCapacity;
    }

private:
    struct st_NODE
    {
        st_NODE *pNext;
        T Data;
    };

    static_assert(std::is_standard_layout<st_NODE>::value
        || sizeof(void*) == 8,
        "st_NODE layout may not be compatible with offsetof on this platform");


    int m_iCapacity;
    int m_iAllocCount;
    st_NODE *m_pNodeArray;
    st_NODE *m_pFreeHead;
};
