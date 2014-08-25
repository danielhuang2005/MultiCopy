/*******************************************************************************
 *
 *            Copyright (С) 2011 Юрий Владимирович Круглов
 *
 * Эта программа является свободным программным обеспечением. Вы можете
 * распространять и/или  модифицировать её согласно условиям Стандартной
 * Общественной Лицензии GNU, опубликованной Организацией Свободного
 * Программного Обеспечения, версии 3, либо по Вашему желанию, любой более
 * поздней версии.
 *
 * Эта программа распространяется в надежде на то, что окажется полезной, но
 * БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, даже без подразумеваемых гарантий ОКУПАЕМОСТИ или
 * СООТВЕТСТВИЯ КОНКРЕТНЫМ ЦЕЛЯМ.
 * Подробнее - см. Стандартной Общественную Лицензию GNU.
 *
 * Вы должны были получить копию Основной Общественной Лицензии GNU вместе с
 * этой программой. При её отсутствии обратитесь на сайт
 * http://www.gnu.org/licenses/.
 *
 *******************************************************************************
 *
 *                 Copyright (C) 2011 Yuri V. Krugloff
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

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
 *
 * \arg BlocksCount Число блоков в кольцевом буфере.
 *
 * \remarks Изменять число блоков в кольцевом буфере после начала работы с ним
 *   нельзя!
 */

TSynchronizer::TSynchronizer(int BlocksCount)
    : m_ProducerSemaphore(BlocksCount),
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
    unregisterAll();
}

//------------------------------------------------------------------------------
//! Пересчёт индексов чтения и освобождение семафора свободных блоков.

void TSynchronizer::updateReadIndex()
{
    TDataMap::const_iterator I = m_DataMap.constBegin();
    if (I != m_DataMap.constEnd())
    {
        // Вычисляем минимальную дельту.
        int minDelta = I.value()->Delta;
        while (++I != m_DataMap.constEnd())
            if (I.value()->Delta < minDelta)
                minDelta = I.value()->Delta;

        // Если минимальная дельта отлична от нуля.
        if (minDelta > 0)
        {
            // Сдвигаем индекс чтения.
            m_ReadIndex = (m_ReadIndex + minDelta) % m_BlocksCount;

            // Уменьшаем дельты.
            for (TDataMap::const_iterator I = m_DataMap.constBegin();
                 I != m_DataMap.constEnd(); ++I)
            {
                I.value()->Delta -= minDelta;
                Q_ASSERT(I.value()->Delta >= 0);
            }

            // Освобождаем семафор свободных блоков.
            m_ProducerSemaphore.release(minDelta);
        }
    }
}

//------------------------------------------------------------------------------
//! Захват блоков "производителем".
/*!
 * Метод производит захват указанного числа свободных блоков кольцевого
 * буфера. Если данное число блоков захватить невозможно, вызывающий процесс
 * останавливается до момента их освобождения.
 *
 * \arg n Требуемое число блоков.
 *
 * \remarks Если число блоков неположительно, метод ничего не делает;
 *   если это число больше числа блоков буфера, производится захват всех блоков.
 */

void TSynchronizer::acquireProducerSemaphore(int n)
{
    // Коррекция числа блоков.
    if (n <= 0) return;
    if (n > m_BlocksCount) n = m_BlocksCount;

    m_ProducerSemaphore.acquire(n);
}

//------------------------------------------------------------------------------
//! Освобождение блоков.
/*!
 * Метод производит освобождение указанного числа занятых блоков кольцевого
 * буфера. Блокировки вызывающего процесса не происходит.
 *
 * \arg n Требуемое число блоков.
 *
 * \remarks Если число освобождаемых блоков неположительно, метод ничего не
 *   делает; если это число больше числа блоков буфера, освобождаются все блоки.
 */

void TSynchronizer::releaseConsumerSemaphore(int n)
{
    // Коррекция числа освобождаемых блоков.
    if (n <= 0) return;
    if (n > m_BlocksCount) n = m_BlocksCount;

    // Все операции со счётчиками должны проводиться монопольно!
    {
        QMutexLocker Locker(&m_Mutex);
        Q_UNUSED(Locker);
        m_WriteIndex = (m_WriteIndex + n) % m_BlocksCount;

        for (TDataMap::const_iterator I = m_DataMap.constBegin();
             I != m_DataMap.constEnd(); ++I)
        {
            I.value()->Semaphore.release(n);
        }
    }
}

//------------------------------------------------------------------------------
//! Захват блоков потоком-потребителем.
/*!
 * Метод производит захват указанного числа занятых блоков кольцевого
 * буфера. Если данное число блоков захватить невозможно, вызывающий процесс
 * останавливается до момента их освобождения.
 *
 * \arg pConsumer Указатель, используемый в качестве идентификатора
 *   "потребителя". Должен совпадать с указателем, переданным ранее в метод
 *   registerConsumer.
 * \arg n Требуемое число блоков.
 *
 * \remarks Если число блоков неположительно, метод ничего не делает;
 *   если это число больше числа блоков буфера, производится захват всех блоков.
 *
 * \remarks  Если "потребитель" не был ранее зарегистрирован методом
 *   registerConsumer или его регистрация отменена методом unregisterConsumer
 *   захват блоков и остановка вызывающего процесса не происходят.
 */

void TSynchronizer::acquireConsumerSemaphore(void* pConsumer, int n)
{
    TConsumerData* pCD;
    if ((n > 0) && ((pCD = m_DataMap.value(pConsumer, NULL)) != NULL))
    {
        // Коррекция числа блоков.
        if (n > m_BlocksCount) n = m_BlocksCount;

        pCD->Semaphore.acquire(n);
    }
}

//------------------------------------------------------------------------------
//! Освобождение свободных блоков.
/*!
 * Метод производит освобождение указанного числа занятых блоков кольцевого
 * буфера. Блокировки вызывающего процесса не происходит.
 *
 * \arg pConsumer Указатель, используемый в качестве идентификатора
 *   "потребителя". Должен совпадать с указателем, переданным ранее в метод
 *   registerConsumer.
 * \arg n Число освобождаемых блоков.
 *
 * \remarks  Если "потребитель" не был ранее зарегистрирован методом
 *   registerConsumer или его регистрация отменена методом unregisterConsumer
 *   освобождения свободных блоков не происходит.
 *
 * \remarks Если число освобождаемых блоков неположительно, метод ничего не
 *   делает; если это число больше числа блоков буфера, освобождаются все блоки.
 */

void TSynchronizer::releaseProducerSemaphore(void* pConsumer, int n)
{
    TConsumerData* pCD;
    if ((n > 0) && ((pCD = m_DataMap.value(pConsumer, NULL)) != NULL))
    {
        // Коррекция числа блоков.
        if (n > m_BlocksCount) n = m_BlocksCount;

        // Все операции со счётчиками должны проводиться монопольно!
        QMutexLocker Locker(&m_Mutex);
        Q_UNUSED(Locker);

        // Увеличиваем дельту.
        pCD->Delta += n;
        updateReadIndex();
    }
}

//------------------------------------------------------------------------------
//! Регистрация "потребителя".
/*!
 * Метод производит регистрацию "потребителя", создавая для него внутреннюю
 * структуру данных.
 *
 * \arg pConsumer Указатель, используемый в качестве идентификатора
 *   "потребителя". Это может быть, например, дескриптор потока или указатель
 *   на объект типа QThread.
 *
 * \return true, если регистрация прошла успешно и false если "потребитель"
 *   уже зарегистрирован.
 */

bool TSynchronizer::registerConsumer(void* pConsumer)
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    if (!m_DataMap.contains(pConsumer))
    {
        m_DataMap.insert(pConsumer, new TConsumerData());
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//! Отмена регистрации "потребителя".
/*!
 *
 * \return true, если регистрация успешно отменена и false если "потребитель"
 *   не был зарегистрирован.
 */

bool TSynchronizer::unregisterConsumer(void* pConsumer)
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    TConsumerData* pCD = m_DataMap.value(pConsumer, NULL);
    if (pCD != NULL)
    {
        delete pCD;
        m_DataMap.remove(pConsumer);
        updateReadIndex();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//! Отмена регистрации всех потребителей.

void TSynchronizer::unregisterAll()
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    for (TDataMap::const_iterator I = m_DataMap.constBegin();
         I != m_DataMap.constEnd(); ++I)
    {
        delete I.value();
    }
    m_DataMap.clear();
}

//------------------------------------------------------------------------------
//! Список всех потребителей.

QList<void*> TSynchronizer::consumers() const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_DataMap.keys();
}

//------------------------------------------------------------------------------
//! Количество потребителей.

int TSynchronizer::consumersCount() const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_DataMap.count();
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
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);
    return m_WriteIndex;
}

//------------------------------------------------------------------------------
//! Число свободных блоков в кольцевом буфере.

int TSynchronizer::freeBlocksCount() const
{
    return m_ProducerSemaphore.available();
}

//------------------------------------------------------------------------------
//! Проверка регистрации "потребителя".

bool TSynchronizer::isConsumerRegistered(void* pConsumer) const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_DataMap.contains(pConsumer);
}

//------------------------------------------------------------------------------
//! Индекс первого занятого блока.
/*!
 * Метод возвращает индекс первого ещё не освобождённого хотя бы одним
 * "потребителем" блока.
 */

int TSynchronizer::firstUsedIndex() const
{
    // Все операции со счётчиками должны проводиться монопольно!
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_ReadIndex;
}

//------------------------------------------------------------------------------
//! Индекс первого ещё не освобождённого "потребителем" блока.
/*!
 * Метод возвращает индекс первого ещё не освобождённого "потребителем" блока
 * кольцевого буфера. Если потребитель не зарегистрирован, возвращает -1.
 *
 * \arg pConsumer Указатель, используемый в качестве идентификатора
 *   "потребителя". Должен совпадать с указателем, переданным ранее в метод
 *   registerConsumer.
 */

int TSynchronizer::firstUsedIndex(void* pConsumer) const
{
    TConsumerData* pCD = m_DataMap.value(pConsumer, NULL);
    if (pCD != NULL)
    {
        // Все операции со счётчиками должны проводиться монопольно!
        QMutexLocker Locker(&m_Mutex);
        Q_UNUSED(Locker);

        return (m_ReadIndex + pCD->Delta) % m_BlocksCount;
    }
    return -1;
}

//------------------------------------------------------------------------------
//! Разблокировка всех семафоров.
/*!
 * Метод производит разблокировку всех внутренних семафоров класса, позволяя
 * пройти точку ожидания освобождения семафоров всем процессам. Следует иметь
 * в виду, что после выполнения этого метода индексы и счётчики семафоров
 * будут неверны. Для исправления этой ситуации после вызова unlock
 * и завершения всех разблокированных процессов вызовите метод reset.
 *
 * \remarks Чтобы определить, был ли вызван метод unlock, используйте метод
 *   isUnlocked. Он будет возвращать true до вызова метода reset.
 */

void TSynchronizer::unlock()
{
    // Флаг разблокировки защищается тем же мьютексом, что и индекс m_ReadIndex.
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_Unlocked = true;

    m_ProducerSemaphore.release(m_BlocksCount -
                                m_ProducerSemaphore.available());

    for (TDataMap::const_iterator I = m_DataMap.constBegin();
         I != m_DataMap.constEnd(); ++I)
    {
        I.value()->Semaphore.release(m_BlocksCount -
                                     I.value()->Semaphore.available());
    }
}

//------------------------------------------------------------------------------
//! Признак принудительной разблокировки.
/*!
 * Метод возвращает true, если был вызван метод unlock. После вызова метода
 * reset метод буде возвращать false.
 *
 * \sa unlock, reset
 */

bool TSynchronizer::isUnlocked() const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_Unlocked;
}

//------------------------------------------------------------------------------
//! Сброс всех счётчиков и индексов.
/*!
 * Метод сбрасывает в начальное состояние все индексы и счётчики, а также
 * приводит в начальное состояние все семафоры, устанавливая число свободных
 * блоков кольцевого буфера в максимум (равен общему числу блоков), а число
 * блоков, доступных для процессов-потребителей, в ноль.
 */

void TSynchronizer::reset()
{
    QMutexLocker Locker1(&m_Mutex);
    Q_UNUSED(Locker1);

    m_ProducerSemaphore.release(m_BlocksCount -
                                m_ProducerSemaphore.available());
    m_WriteIndex = 0;
    m_ReadIndex  = 0;

    for (TDataMap::const_iterator I = m_DataMap.constBegin();
         I != m_DataMap.constEnd(); ++I)
    {
        I.value()->Semaphore.acquire(I.value()->Semaphore.available());
        I.value()->Delta = 0;
    }

    m_Unlocked = false;
}

//------------------------------------------------------------------------------

