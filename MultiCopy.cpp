#include "MultiCopy.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TSemaphoreEx::TSemaphoreEx()
{

}

//------------------------------------------------------------------------------

TSemaphoreEx::~TSemaphoreEx()
{

}

//------------------------------------------------------------------------------

bool TSemaphoreEx::registerConsumer(QObject* pConsumer)
{
    if (m_pSemaphoresMap.contains(pConsumer))
        return false;

    m_pSemaphoresMap.insert(pConsumer, new QSemaphore());
    return true;
}

//------------------------------------------------------------------------------

bool TSemaphoreEx::unregisterConsumer(QObject* pConsumer)
{
    QSemaphore* pS = m_pSemaphoresMap.value(pConsumer, NULL);
    if (pS != NULL) {
        delete pS;
        m_pSemaphoresMap.remove(pConsumer);
    }
    return pS != NULL;
}

//------------------------------------------------------------------------------

void TSemaphoreEx::unregisterAll()
{
    for (TSemaphoresMap::const_iterator I = m_pSemaphoresMap.constBegin();
         I != m_pSemaphoresMap.constEnd(); ++I)
    {
        delete I.value();
    }
    m_pSemaphoresMap.clear();
}

//------------------------------------------------------------------------------

void TSemaphoreEx::acquire(QObject* pConsumer, int n)
{
    QSemaphore* pS = m_pSemaphoresMap.value(pConsumer, NULL);
    if (pS != NULL)
        pS->acquire(n);
}

//------------------------------------------------------------------------------

void TSemaphoreEx::release(QObject* pConsumer, int n)
{
    QSemaphore* pS = m_pSemaphoresMap.value(pConsumer, NULL);
    if (pS != NULL)
        pS->release(n);
}

//------------------------------------------------------------------------------
