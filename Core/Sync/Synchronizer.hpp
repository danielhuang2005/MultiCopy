/*******************************************************************************

              Copyright (С) 2012 Юрий Владимирович Круглов

   Эта программа является свободным программным обеспечением. Вы можете
   распространять и/или  модифицировать её согласно условиям Стандартной
   Общественной Лицензии GNU, опубликованной Организацией Свободного
   Программного Обеспечения, версии 3, либо по Вашему желанию, любой более
   поздней версии.

   Эта программа распространяется в надежде на то, что окажется полезной, но
   БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, даже без подразумеваемых гарантий ОКУПАЕМОСТИ или
   СООТВЕТСТВИЯ КОНКРЕТНЫМ ЦЕЛЯМ.
   Подробнее - см. Стандартной Общественную Лицензию GNU.

   Вы должны были получить копию Основной Общественной Лицензии GNU вместе с
   этой программой. При её отсутствии обратитесь на сайт
   http://www.gnu.org/licenses/.

********************************************************************************

                   Copyright (C) 2012 Yuri V. Krugloff

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation, either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

#ifndef __SYNCHRONIZER__HPP__7135A604_B77B_41ED_B50F_CAE51C0D80CB__
#define __SYNCHRONIZER__HPP__7135A604_B77B_41ED_B50F_CAE51C0D80CB__

#include <QHash>
#include <QReadWriteLock>

#include "ProducerLocker.hpp"
#include "SemaphoreEx.hpp"

//------------------------------------------------------------------------------
//! Класс синхронизатора потоков.
/*!
   Все публичные методы класса реентерабельны (reenterable) и потокобезопасны
   (thread-safe). Для остальных это не гарантируется!
 */

class TSynchronizer
{
    private :
        //! Структура связанных с потребителем данных.
        struct TConsumerData {
            TSemaphoreEx Semaphore;  //!< Семафор непрочитанных блоков.
            int          Delta;      //!< Сдвиг индекса чтения.
            bool         Blocked;    //!< Флаг блокирующего потребителя.
            //! Конструктор.
            TConsumerData(bool blocked = false)
                : Delta(0), Blocked(blocked) {}
        };

        //! Массив структур данных, связанных с потребителями.
        typedef QHash<void*, TConsumerData*> TConsumersData;

        TConsumersData  m_ConsumersData;      //!< Данные, связанные с
                                              //!< потребителями.
        TSemaphoreEx    m_ProducerSemaphore;  //!< Семафор свободных блоков.
        TProducerLocker m_ProducerLocker;     //!< Блокировщик производителя.

        int m_BlockedConsumersCount;  //!< Число блокирующих потребителей.
        int m_BlocksCount;  //!< Число блоков кольцевого буфера.
        int m_ReadIndex;    //!< Индекс первого занятого блока.
        int m_WriteIndex;   //!< Индекс первого свободного блока.

        mutable QReadWriteLock m_Locker;  //!< Блокировщик доступа.
        bool m_Unlocked;         //!< Флаг принудительной разблокировки.

        void updateReadIndex();
    public:
        TSynchronizer(int BlocksCount);
        ~TSynchronizer();

        void acquireProducerSemaphore(int n = 1);
        void releaseProducerSemaphore(void* pConsumer, int n = 1);

        void acquireConsumerSemaphore(void* pConsumer, int n = 1);
        void releaseConsumerSemaphore(int n = 1);

        bool registerBlockedConsumer(void* pConsumer);
        bool registerNonblockedConsumer(void* pConsumer);
        bool registerConsumer(void* pConsumer, bool Blocked = false);
        //bool reRegisterConsumer(void* pConsumer, bool Blocked);
        bool unregisterConsumer(void* pConsumer);
        bool isConsumerRegistered(void* pConsumer) const;
        bool isBlockedConsumer(void* pConsumer) const;
        bool isNonblockedConsumer(void* pConsumer) const;

        void unregisterAllBlockedConsumers();
        void unregisterAllNonblockedConsumers();
        void unregisterAllConsumers();

        QList<void*> blockedConsumers() const;
        QList<void*> nonblockedConsumers() const;
        QList<void*> consumers() const;
        int blockedConsumersCount() const;
        int nonblockedConsumersCount() const;
        int consumersCount() const;

        int blocksCount() const;
        int firstFreeIndex() const;
        int freeBlocksCount() const;
        int firstUsedIndex() const;
        int firstUsedIndex(void* pConsumer) const;

        void unlock();
        bool isUnlocked() const;
        void reset();
};

//------------------------------------------------------------------------------

#endif // __SYNCHRONIZER__HPP__7135A604_B77B_41ED_B50F_CAE51C0D80CB__
