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

#include "Synchronizer.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                      T S y n c h r o n i z e r
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Конструктор.
/*!
   \arg BlocksCount Число блоков в кольцевом буфере.

   \remarks Изменять число блоков в кольцевом буфере после начала работы с ним
     нельзя!
 */

TSynchronizer::TSynchronizer(int BlocksCount)
    : m_ProducerSemaphore(BlocksCount),
      m_BlockedConsumersCount(0),
      m_BlocksCount(BlocksCount),
      m_ReadIndex(0),
      m_WriteIndex(0),
      m_Unlocked(false)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TSynchronizer::~TSynchronizer()
{
    unregisterAllConsumers();
}

//------------------------------------------------------------------------------
//! Пересчёт индексов чтения и освобождение семафора свободных блоков.

void TSynchronizer::updateReadIndex()
{
    // Все операции со счётчиками должны проводиться монопольно!
    // (Метод должен быть вызван только при заблокированном мьютексе.)
    Q_ASSERT(!m_Locker.tryLockForWrite());

    if (m_ConsumersData.count() <= 0)
        return;

    // Вычисляем минимальную дельту.
    TConsumersData::const_iterator I = m_ConsumersData.constBegin();
    int minDelta = I.value()->Delta;
    while (++I != m_ConsumersData.constEnd())
        if (I.value()->Delta < minDelta)
            minDelta = I.value()->Delta;

    // Если минимальная дельта отлична от нуля.
    if (minDelta > 0)
    {
        // Сдвигаем индекс чтения.
        m_ReadIndex = (m_ReadIndex + minDelta) % m_BlocksCount;

        // Уменьшаем дельты.
        // --->>>   ВНИМАНИЕ! Повторное использование I.   <<<---
        for (I = m_ConsumersData.constBegin();
             I != m_ConsumersData.constEnd(); ++I)
        {
            I.value()->Delta -= minDelta;
            Q_ASSERT(I.value()->Delta >= 0);
        }

        // Освобождаем семафор свободных блоков.
        m_ProducerSemaphore.release(minDelta);
    }
}

//------------------------------------------------------------------------------
//! Захват блоков производителем.
/*!
   Метод производит захват указанного числа свободных блоков кольцевого
   буфера. Если данное число блоков захватить невозможно, вызывающий процесс
   останавливается до момента их освобождения.

   \arg n Требуемое число блоков.

   \remarks Если число блоков неположительно, метод ничего не делает;
     если это число больше числа блоков буфера, производится захват всех блоков.

   \sa releaseProducerSemaphore, acquireConsumerSemaphore,
       releaseConsumerSemaphore
 */

void TSynchronizer::acquireProducerSemaphore(int n)
{
    // Коррекция числа блоков.
    if (n <= 0) {
        qWarning("TSynchronizer::acquireProducerSemaphore. "
                 "Non-positive argument (%i). Ignored.", n);
        return;
    }
    if (n > m_BlocksCount) {
        qWarning("TSynchronizer::acquireProducerSemaphore. "
                 "Argument (%i) is more than blocks count (%i). Reduced.",
                 n, m_BlocksCount);
        n = m_BlocksCount;
    }


    // Ждём завершения блокирующих потребителей...
    m_ProducerLocker.wait();
    // .. и только после этого пытаемся захватить блок.
    m_ProducerSemaphore.acquire(n);
}

//------------------------------------------------------------------------------
//! Освобождение блоков.
/*!
   Метод производит освобождение указанного числа занятых блоков кольцевого
   буфера. Блокировки вызывающего процесса не происходит.

   \arg n Требуемое число блоков.

   \remarks Если число освобождаемых блоков неположительно, метод ничего не
     делает; если это число больше числа блоков буфера, освобождаются все блоки.

   \sa acquireProducerSemaphore, releaseProducerSemaphore,
       acquireConsumerSemaphore
 */

void TSynchronizer::releaseConsumerSemaphore(int n)
{
    // Коррекция числа блоков.
    if (n <= 0) {
        qWarning("TSynchronizer::releaseConsumerSemaphore. "
                 "Non-positive argument (%i). Ignored.", n);
        return;
    }
    if (n > m_BlocksCount) {
        qWarning("TSynchronizer::releaseConsumerSemaphore. "
                 "Argument (%i) is more than blocks count (%i). Reduced.",
                 n, m_BlocksCount);
        n = m_BlocksCount;
    }


    // Все операции со счётчиками должны проводиться монопольно!
    {
        QWriteLocker WriteLocker(&m_Locker);
        Q_UNUSED(WriteLocker);

        // Перемещаем индекс записи.
        m_WriteIndex = (m_WriteIndex + n) % m_BlocksCount;

        // Инициализируем блокировщик производителя...
        m_ProducerLocker.init(m_BlockedConsumersCount);

        // ... и только после этого освобождаем семафоры потребителей.
        for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
             I != m_ConsumersData.constEnd(); ++I)
        {
            I.value()->Semaphore.release(n);
        }
    }
}

//------------------------------------------------------------------------------
//! Захват блоков потребителем.
/*!
   Метод производит захват указанного числа занятых блоков кольцевого
   буфера. Если данное число блоков захватить невозможно, вызывающий процесс
   останавливается до момента их освобождения.

   \arg pConsumer Указатель, используемый в качестве идентификатора
     потребителя. Должен совпадать с указателем, переданным ранее в метод
     регистрации потребителя.
   \arg n Требуемое число блоков.

   \remarks Если число блоков неположительно, метод ничего не делает;
     если это число больше числа блоков буфера, производится захват всех блоков.

   \remarks Если потребитель не был ранее зарегистрирован методом
     или его регистрация отменена, захват блоков и остановка вызывающего
     процесса не происходят.

   \remarks Метод независим от типа потребителя (блокирующий или
     неблокирующий).

   \sa acquireProducerSemaphore, releaseProducerSemaphore,
       releaseConsumerSemaphore
 */

void TSynchronizer::acquireConsumerSemaphore(void* pConsumer, int n)
{
    // В этой функции не должно быть изменения состояния блокировщика
    // производителя.

    // Коррекция числа блоков.
    if (n < 0) {
        qWarning("TSynchronizer::acquireConsumerSemaphore. "
                 "Non-positive argument (%i). Ignored.", n);
        return;
    }
    if (n > m_BlocksCount) {
        qWarning("TSynchronizer::acquireConsumerSemaphore. "
                 "Argument (%i) is more than blocks count (%i). "
                 "Reduced.", n, m_BlocksCount);
        n = m_BlocksCount;
    }


    TConsumerData* pCD = m_ConsumersData.value(pConsumer, NULL);
    if (pCD != NULL)
        pCD->Semaphore.acquire(n);
}

//------------------------------------------------------------------------------
//! Освобождение свободных блоков.
/*!
   Метод производит освобождение указанного числа занятых блоков кольцевого
   буфера. Блокировки вызывающего процесса не происходит.

   \arg pConsumer Указатель, используемый в качестве идентификатора
     "потребителя". Должен совпадать с указателем, переданным ранее в метод
     registerConsumer.
   \arg n Число освобождаемых блоков.

   \remarks Если потребитель не был ранее зарегистрирован методом
     registerConsumer или его регистрация отменена методом unregisterConsumer
     освобождения свободных блоков не происходит.

   \remarks Если число освобождаемых блоков неположительно, метод ничего не
     делает; если это число больше числа блоков буфера, освобождаются все блоки.

   \remarks Метод независим от типа потребителя (блокирующий или
     неблокирующий).

   \sa acquireProducerSemaphore, acquireConsumerSemaphore,
       releaseConsumerSemaphore
 */

void TSynchronizer::releaseProducerSemaphore(void* pConsumer, int n)
{
    // Коррекция числа блоков.
    if (n < 0) {
        qWarning("TSynchronizer::releaseProducerSemaphore called with "
                 "non-positive argument (%i). Ignored.", n);
        return;
    }
    if (n > m_BlocksCount) {
        qWarning("TSynchronizer::releaseProducerSemaphore is called with "
                 "argument (%i) is more than blocks count (%i). Reduced.",
                 n, m_BlocksCount);
        // n = m_BlocksCount;
        // Коррекция будет выполнена далее.
    }


    TConsumerData* pCD = m_ConsumersData.value(pConsumer, NULL);
    if (pCD != NULL)
    {
        // Коррекция числа блоков.
        int n1 = m_BlocksCount - pCD->Delta;
        if (n > n1) {
            qWarning("TSynchronizer::releaseProducerSemaphore. "
                     "Argument (%i) is more than available blocks "
                     "count (%i). Reduced.", n, n1);
            n = n1;
        }

        // Все операции со счётчиками должны проводиться монопольно!
        QWriteLocker WriteLocker(&m_Locker);
        Q_UNUSED(WriteLocker);

        // Увеличиваем дельту.
        pCD->Delta += n;
        Q_ASSERT(pCD->Delta <= m_BlocksCount);
        updateReadIndex();

        // Освобождаем блокировщик производителя только после обновления
        // индексов.
        m_ProducerLocker.release(pConsumer);
    }
}

//------------------------------------------------------------------------------
//! Регистрация блокирующего потребителя.
/*!
   \sa registerConsumer
 */

bool TSynchronizer::registerBlockedConsumer(void* pConsumer)
{
    return registerConsumer(pConsumer, true);
}

//------------------------------------------------------------------------------
//! Регистрация неблокирующего потребителя.
/*!
   \sa registerConsumer
 */

bool TSynchronizer::registerNonblockedConsumer(void* pConsumer)
{
    return registerConsumer(pConsumer, false);
}

//------------------------------------------------------------------------------
//! Регистрация неблокирующего потребителя.
/*!
   Метод производит регистрацию потребителя, создавая для него
   внутреннюю структуру данных.

   \arg pConsumer Указатель, используемый в качестве идентификатора
     потребителя. Это может быть, например, дескриптор потока или указатель
     на объект типа QThread.
   \arg Blocked true, если нужно зарегистрировать блокирующего потребителя
     и false, если неблокирующего.

   \return true, если регистрация прошла успешно и false, если потребитель
     уже зарегистрирован.

   \remarks Метод не позволяет зарегистрировать одного и того же потребителя
     одновременно как блокирующего и неблокирующего.

   \sa registerBlockedConsumer, registerNonblockedConsumer
 */

bool TSynchronizer::registerConsumer(void* pConsumer, bool Blocked)
{
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    if (!m_ConsumersData.contains(pConsumer))
    {
        m_ConsumersData.insert(pConsumer, new TConsumerData(Blocked));
        if (Blocked) {
            ++m_BlockedConsumersCount;
            Q_ASSERT(m_BlockedConsumersCount <= m_ConsumersData.count());
        }
        return true;
    }
    else {
        qWarning("Attempt to register consumer (%p), "
                 "which is already registered.", pConsumer);
    }
    return false;
}

//------------------------------------------------------------------------------
//! Отмена регистрации потребителя.
/*!
   \return true, если регистрация успешно отменена и false, если потребитель
     не был зарегистрирован.

   \sa unregisterBlockedConsumer, unregisterNonblockedConsumer
 */

bool TSynchronizer::unregisterConsumer(void* pConsumer)
{
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    TConsumerData* pCD = m_ConsumersData.value(pConsumer, NULL);
    if (pCD != NULL)
    {
        if (pCD->Blocked) {
            --m_BlockedConsumersCount;
            Q_ASSERT(m_BlockedConsumersCount >= 0);
            m_ProducerLocker.release(pConsumer);
        }
        delete pCD;
        m_ConsumersData.remove(pConsumer);
        updateReadIndex();
        return true;
    }
    else {
        qWarning("TSynchronizer::unregisterConsumer. Attempt to unregister "
                 "consumer (%p), which is not registered.", pConsumer);
    }
    return false;
}

//------------------------------------------------------------------------------
//! Проверка регистрации потребителя.
/*!
   Если потребитель зарегистрирован в экземпляре класса синхронизатора (как
   блокирующий или как неблокирующий), метод возвращает true, в противном
   случае - false.
 */

bool TSynchronizer::isConsumerRegistered(void* pConsumer) const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_ConsumersData.contains(pConsumer);
}

//------------------------------------------------------------------------------
//! Проверка блокирующего потребителя.
/*!
   Метод проверяет, является ли указанный потребитель блокирующим и возвращает
   true, если да. Если потребитель неблокирующий или не зарегистрирован,
   возвращает false.

   \sa isConsumerRegistered, isNonblockedConsumer
 */

bool TSynchronizer::isBlockedConsumer(void *pConsumer) const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    TConsumerData* pCD = m_ConsumersData.value(pConsumer, NULL);
    return (pCD != NULL) && pCD->Blocked;
}

//------------------------------------------------------------------------------
//! Проверка неблокирующего потребителя.
/*!
   Метод проверяет, является ли указанный потребитель неблокирующим и возвращает
   true, если да. Если потребитель блокирующий или не зарегистрирован,
   возвращает false.

   \sa isConsumerRegistered, isBlockedConsumer
 */

bool TSynchronizer::isNonblockedConsumer(void *pConsumer) const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    TConsumerData* pCD = m_ConsumersData.value(pConsumer, NULL);
    return (pCD != NULL) && !pCD->Blocked;
}

//------------------------------------------------------------------------------
//! Отмена регистрации всех блокирующих потребителей.
/*!
   \sa unregisterAllNonblockedConsumers, unregisterAllConsumers
 */

void TSynchronizer::unregisterAllBlockedConsumers()
{
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        if (I.value()->Blocked)
        {
            delete I.value();
            m_ConsumersData.remove(I.key());
        }
    }
    m_BlockedConsumersCount = 0;
    m_ProducerLocker.unlock();
}

//------------------------------------------------------------------------------
//! Отмена регистрации всех "неблокирующих потребителей".
/*!
   \sa unregisterAllBlockedConsumers, unregisterAllConsumers
 */

void TSynchronizer::unregisterAllNonblockedConsumers()
{
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        if (!I.value()->Blocked)
        {
            delete I.value();
            m_ConsumersData.remove(I.key());
        }
    }
}

//------------------------------------------------------------------------------
//! Отмена регистрации всех потребителей.

void TSynchronizer::unregisterAllConsumers()
{
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        delete I.value();
    }
    m_ConsumersData.clear();
    m_BlockedConsumersCount = 0;
    m_ProducerLocker.unlock();
}

//------------------------------------------------------------------------------
//! Список всех блокирующих потребителей.
/*!
   \sa nonblockedConsumers, consumers
 */

QList<void*> TSynchronizer::blockedConsumers() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    QList<void*> Result;
    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        if (I.value()->Blocked)
            Result.append(I.value());
    }
    return Result;
}

//------------------------------------------------------------------------------
//! Список всех неблокирующих потребителей.
/*!
   \sa blockedConsumers, consumers
 */

QList<void*> TSynchronizer::nonblockedConsumers() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    QList<void*> Result;
    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        if (!I.value()->Blocked)
            Result.append(I.value());
    }
    return Result;
}

//------------------------------------------------------------------------------
//! Список всех потребителей.

QList<void*> TSynchronizer::consumers() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_ConsumersData.keys();
}

//------------------------------------------------------------------------------
//! Количество блокирующих потребителей.
/*!
   \sa nonblockedConsumersCount, consumersCount
 */

int TSynchronizer::blockedConsumersCount() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_BlockedConsumersCount;
}

//------------------------------------------------------------------------------
//! Количество неблокирующих потребителей.
/*!
   \sa blockedConsumersCount, consumersCount
 */

int TSynchronizer::nonblockedConsumersCount() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_ConsumersData.count() - m_BlockedConsumersCount;
}

//------------------------------------------------------------------------------
//! Общее количество потребителей.

int TSynchronizer::consumersCount() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_ConsumersData.count();
}

//------------------------------------------------------------------------------
//! Число блоков в кольцевом буфере.

int TSynchronizer::blocksCount() const
{
    return m_BlocksCount;
}

//------------------------------------------------------------------------------
//! Индекс первого свободного блока.

int TSynchronizer::firstFreeIndex() const
{
    // Все операции со счётчиками должны проводиться монопольно!
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_WriteIndex;
}

//------------------------------------------------------------------------------
//! Число свободных блоков в кольцевом буфере.

int TSynchronizer::freeBlocksCount() const
{
    return m_ProducerSemaphore.available();
}

//------------------------------------------------------------------------------
//! Индекс первого занятого блока.
/*!
   Метод возвращает индекс первого ещё не освобождённого хотя бы одним
   потребителем блока.
 */

int TSynchronizer::firstUsedIndex() const
{
    // Все операции со счётчиками должны проводиться монопольно!
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_ReadIndex;
}

//------------------------------------------------------------------------------
//! Индекс первого ещё не освобождённого потребителем блока.
/*!
   Метод возвращает индекс первого ещё не освобождённого потребителем блока
   кольцевого буфера. Если потребитель не зарегистрирован, возвращает -1.

   \arg pConsumer Указатель, используемый в качестве идентификатора
     потребителя. Должен совпадать с указателем, переданным ранее в метод
     регистрации потребителя.
 */

int TSynchronizer::firstUsedIndex(void* pConsumer) const
{
    TConsumerData* pCD = m_ConsumersData.value(pConsumer, NULL);
    if (pCD != NULL)
    {
        // Все операции со счётчиками должны проводиться монопольно!
        QReadLocker ReadLocker(&m_Locker);
        Q_UNUSED(ReadLocker);

        return (m_ReadIndex + pCD->Delta) % m_BlocksCount;
    }
    return -1;
}

//------------------------------------------------------------------------------
//! Разблокировка всех семафоров.
/*!
   Метод производит разблокировку всех внутренних семафоров класса, позволяя
   пройти точку ожидания освобождения семафоров всем процессам. Следует иметь
   в виду, что после выполнения этого метода индексы и счётчики семафоров
   могут быть неверны. Для исправления этой ситуации после вызова unlock
   и завершения всех разблокированных процессов вызовите метод reset.

   \remarks Чтобы определить, был ли вызван метод unlock, используйте метод
     isUnlocked. Он будет возвращать true до вызова метода reset.

   \sa isUnlocked, reset
 */

void TSynchronizer::unlock()
{
    // Флаг разблокировки защищается тем же мьютексом, что и индекс m_ReadIndex.
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    m_Unlocked = true;

    m_ProducerSemaphore.init(m_BlocksCount);

    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        I.value()->Semaphore.init(m_BlocksCount);
    }

    m_ProducerLocker.unlock();
}

//------------------------------------------------------------------------------
//! Признак принудительной разблокировки.
/*!
   Метод возвращает true, если был вызван метод unlock. После вызова метода
   reset метод будет возвращать false.

   \sa unlock, reset
 */

bool TSynchronizer::isUnlocked() const
{
    QReadLocker ReadLocker(&m_Locker);
    Q_UNUSED(ReadLocker);

    return m_Unlocked;
}

//------------------------------------------------------------------------------
//! Сброс всех счётчиков и индексов.
/*!
   Метод сбрасывает в начальное состояние все индексы и счётчики, а также
   приводит в начальное состояние все семафоры, устанавливая число свободных
   блоков кольцевого буфера в максимум (равен общему числу блоков), а число
   блоков, доступных для процессов-потребителей, в ноль.

   \remarks Не вызывайте этот метод при работающих синхронизируемых потоках.
     Результаты могут быть непредсказуемы.

   \sa unlock, isUnlocked
 */

void TSynchronizer::reset()
{
    QWriteLocker WriteLocker(&m_Locker);
    Q_UNUSED(WriteLocker);

    m_ProducerSemaphore.init(m_BlocksCount);
    m_WriteIndex = 0;
    m_ReadIndex  = 0;
    m_ProducerLocker.unlock();

    for (TConsumersData::const_iterator I = m_ConsumersData.constBegin();
         I != m_ConsumersData.constEnd(); ++I)
    {
        I.value()->Semaphore.init(0);
        I.value()->Delta = 0;
    }

    m_Unlocked = false;
}

//------------------------------------------------------------------------------
