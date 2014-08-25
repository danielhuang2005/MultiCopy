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

#include "RWCalculator.hpp"

//------------------------------------------------------------------------------

//! Конструктор.

TRWCalculator::TRWCalculator()
    : m_WritedTotal(0),
      m_WritedCurrent(0),
      m_ReadedCurrent(0)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TRWCalculator::~TRWCalculator()
{
}

//------------------------------------------------------------------------------
//! Пересчёт числа записанных байт.
/*!
 * \return true, если изменилось минимальное число записанных байт и false если
 *   не изменилось. Если метод вернул true приложение может обновить виджеты,
 *   визуализирующие общий прогресс операции.
 */

bool TRWCalculator::updateWritedBytes()
{
    bool AllStarted = true;

    for (TData::const_iterator I = m_Data.constBegin();
         I != m_Data.constEnd(); ++I)
    {
        if (!I.value().Started)
        {
            AllStarted = false;
            break;
        }
    }

    if (AllStarted)
    {
        TData::const_iterator I = m_Data.constBegin();
        if (I != m_Data.constEnd())
        {
            qint64 min = I.value().WritedBytes;
            while (++I != m_Data.constEnd())
            {
                register qint64 v = I.value().WritedBytes;
                if (v < min) min = v;
            }
            if (min > m_WritedCurrent)
            {
                m_WritedCurrent = min;
                return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//! Регистрация потока записи.
/*!
 * Метод регистрирует поток записи. Если поток уже зарегистрирован,
 * ничего не делает.
 *
 * \return true, если изменилось минимальное число записанных байт и false если
 *   не изменилось. Если метод вернул true приложение может обновить виджеты,
 *   визуализирующие общий прогресс операции.
 */

bool TRWCalculator::registerWriter(const void *const pFileWriter)
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    if (!m_Data.contains(pFileWriter))
    {
        m_Data.insert(pFileWriter, TWriterData());
        return updateWritedBytes();
    }

    return false;
}

//------------------------------------------------------------------------------
//! Отмена регистрации потока записи.
/*!
 * Метод отменяет регистрацию потока записи. Если поток не зарегистрирован,
 * ничего не делает.
 *
 * \return true, если изменилось минимальное число записанных байт и false если
 *   не изменилось. Если метод вернул true приложение может обновить виджеты,
 *   визуализирующие общий прогресс операции.
 */

bool TRWCalculator::unregisterWriter(const void *const pFileWriter)
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    if (m_Data.contains(pFileWriter))
    {
        m_Data.remove(pFileWriter);
        return updateWritedBytes();
    }
    return false;
}

//------------------------------------------------------------------------------
//! Проверка регистрации потока записи.

bool TRWCalculator::isWriterRegistered(const void *const pFileWriter) const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_Data.contains(pFileWriter);
}

//------------------------------------------------------------------------------
//! Отмена регистрации всех потоков.

void TRWCalculator::unregisterAll()
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_Data.clear();
    m_ReadedCurrent = 0;
    m_WritedCurrent = 0;
}

//------------------------------------------------------------------------------
//! Переход к новому файлу.
/*!
 * Метод сбрасывает все счётчики в начальное состояние. Должен быть вызван перед
 * обработкой следующего файла.
 */

void TRWCalculator::newFile()
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_ReadedCurrent = 0;
    m_WritedTotal += m_WritedCurrent;
    m_WritedCurrent = 0;
    for (TData::iterator I = m_Data.begin(); I != m_Data.end(); ++I)
    {
        I.value().Started = false;
        I.value().WritedBytes = 0;
    }
}

//------------------------------------------------------------------------------
//! Прогресс процесса записи.
/*!
 * Этот метод должен быть вызван потоком записи после того, как он обработал
 * некоторую часть файла.
 *
 * \param pFileWriter Указатель, однозначно идентифицирующий поток-обработчик
 *   файла.
 * \param WritedBytes Число записанных потоком байт (с момента предыдущего
 *   вызова).
 *
 * \return true, если изменилось минимальное число записанных байт и false если
 *   не изменилось. Если метод вернул true приложение может обновить виджеты,
 *   визуализирующие общий прогресс операции.
 */

bool TRWCalculator::writeProgress(const void *const pFileWriter,
                                  const qint64 WritedBytes)
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    if (m_Data.contains(pFileWriter))
    {
        m_Data[pFileWriter].Started = true;
        m_Data[pFileWriter].WritedBytes += WritedBytes;
        return updateWritedBytes();
    }

    return false;
}

//------------------------------------------------------------------------------
//! Прогресс процесса чтения.

bool TRWCalculator::readProgress(const qint64 ReadedBytes)
{
    // Поскольку процесс чтения один, то счётчики можно не защищать мьютексами.
    m_ReadedCurrent += ReadedBytes;
    return true;
}

//------------------------------------------------------------------------------
//! Начало отсчёта времени.

void TRWCalculator::begin()
{
    m_TimeCounter.start();
    m_WritedTotal = 0;
}

//------------------------------------------------------------------------------
//! Пауза при отсчёте времени.

void TRWCalculator::pause()
{
    m_TimeCounter.pause();
}

//------------------------------------------------------------------------------
//! Продолжение отсчёта времени.

void TRWCalculator::resume()
{
    m_TimeCounter.resume();
}

//------------------------------------------------------------------------------
//! Флаг приостановки подсчёта времени.

bool TRWCalculator::isPaused() const
{
    return m_TimeCounter.isPaused();
}

//------------------------------------------------------------------------------
//! Остановка отсчёта времени.

void TRWCalculator::end()
{
    m_TimeCounter.stop();
}

//------------------------------------------------------------------------------
//! Очистка (обнуление) счётчиков.

void TRWCalculator::clear()
{
    m_WritedTotal   = 0;
    m_WritedCurrent = 0;
    m_ReadedCurrent = 0;
    newFile();
    m_TimeCounter.clear();
}

//------------------------------------------------------------------------------
//! Время копирования.

qint64 TRWCalculator::time() const
{
    return m_TimeCounter.msec();
}

//------------------------------------------------------------------------------
//! Скорость копирования (байт в секунду).

qint64 TRWCalculator::speed() const
{
    register qint64 msec = time();
    if (msec > 0)
        return (1000 * (m_WritedTotal + m_WritedCurrent)) / msec;
    else return 0;
}

//------------------------------------------------------------------------------

qint64 TRWCalculator::remaining(qint64 Size) const
{
    qint64 t = time();
    qint64 w = m_WritedTotal + m_WritedCurrent;
    if ((w == 0) || (t == 0))
        return -1;

    return t * Size / w;
}

//------------------------------------------------------------------------------
//! Число процессов записи.

int TRWCalculator::writersCount() const
{
    return m_Data.count();
}

//------------------------------------------------------------------------------
//! Число обработанных процессом чтения байт.

qint64 TRWCalculator::readedBytes() const
{
    return m_ReadedCurrent;
}

//------------------------------------------------------------------------------
//! Число байт, обработанных самым медленным процессом записи.

qint64 TRWCalculator::writedBytes() const
{
    return m_WritedCurrent;
}

//------------------------------------------------------------------------------
//! Число байт, обработанных потоком записи.
/*!
 * Метод возвращает число байт, обработанных потоком записи, ассоциированным
 * с указателем pFileWriter. Если поток не найден, возвращает -1.
 */

qint64 TRWCalculator::writedBytes(const void *const pFileWriter) const
{
    if (m_Data.contains(pFileWriter))
        return m_Data[pFileWriter].WritedBytes;
    else return -1;
}

//------------------------------------------------------------------------------

qint64 TRWCalculator::writedTotal() const
{
    return m_WritedTotal;
}

//------------------------------------------------------------------------------
