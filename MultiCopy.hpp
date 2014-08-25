#ifndef __MULTICOPY__HPP__7135A604_B77B_41ED_B50F_CAE51C0D80CB__
#define __MULTICOPY__HPP__7135A604_B77B_41ED_B50F_CAE51C0D80CB__

#include <QObject>
#include <QHash>
#include <QSemaphore>

class TSemaphoreEx
{
    private :
        typedef QHash<QObject*, QSemaphore*> TSemaphoresMap;
        TSemaphoresMap m_pSemaphoresMap;
        QSemaphore m_pProducerSemaphore;
    public :
        TSemaphoreEx();
        ~TSemaphoreEx();

        bool registerConsumer(QObject* pConsumer);
        bool unregisterConsumer(QObject* pConsumer);
        void unregisterAll();

        void acquire(QObject* pConsumer, int n = 1);
        void release(QObject* pConsumer, int n = 1);
};

#endif // __MULTICOPY__HPP__7135A604_B77B_41ED_B50F_CAE51C0D80CB__
