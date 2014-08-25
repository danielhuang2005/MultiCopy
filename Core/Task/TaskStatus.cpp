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

#include "TaskStatus.hpp"

#include <QThread>

#include "Core/Task/GlobalStatistics.hpp"
#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! Вычисление процента завершения операции.

float percent(qint64 Processed, qint64 Total)
{
    if (Total < 0)
        return -1;

    if (Total == 0)
        return 0;

    return static_cast<float>(Processed) / static_cast<float>(Total) * 100.0;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//             T T a s k S t a t u s : : T C o u n t e r s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.
/*!
   Конструирует экземпляр с обнулёнными счётчиками.
 */

TTaskStatus::TCounters::TCounters()
{
    clear();
}

//------------------------------------------------------------------------------
//! Очистка (обнуление) счётчиков.

void TTaskStatus::TCounters::clear()
{
    m_FilesCompleted        = 0;  m_FilesSkipped        = 0;
    m_CurrentProcessedBytes = 0;  m_TotalProcessedBytes = 0;
    m_CurrentSkippedBytes   = 0;  m_TotalSkippedBytes   = 0;
}

//------------------------------------------------------------------------------
//! Финализация счётчиков.
/*!
   Метод вызывается, когда обработка одного файла завершена, но обработка
   следующего ещё не начата.
 */

void TTaskStatus::TCounters::end()
{
    ++m_FilesCompleted;
}

//------------------------------------------------------------------------------
//! Переход к следующему файлу (нормальная обработка).

void TTaskStatus::TCounters::nextNormal()
{
    m_CurrentProcessedBytes = 0;
    m_CurrentSkippedBytes   = 0;
}

//------------------------------------------------------------------------------
//! Обработка байт.

void TTaskStatus::TCounters::addBytes(qint64 Bytes)
{
    m_CurrentProcessedBytes += Bytes;
    m_TotalProcessedBytes   += Bytes;
}

//------------------------------------------------------------------------------
//! Пропуск байт.

void TTaskStatus::TCounters::skipBytes(qint64 Bytes)
{
    m_CurrentSkippedBytes += Bytes;
    m_TotalSkippedBytes   += Bytes;
}

//------------------------------------------------------------------------------
//! Переход к следующему файлу (пропускаемому).

void TTaskStatus::TCounters::nextSkipped(qint64 Bytes)
{
    m_CurrentProcessedBytes = 0;
    m_CurrentSkippedBytes   = 0;
    ++m_FilesSkipped;
    skipBytes(Bytes);
}

//------------------------------------------------------------------------------
//! Число обработанных файлов (включая пропущенные).

int TTaskStatus::TCounters::files() const
{
    return m_FilesCompleted + m_FilesSkipped;
}

//------------------------------------------------------------------------------
//! Число обработанных байтов в текущем файле (включая пропущенные).

qint64 TTaskStatus::TCounters::currentBytes() const
{
    return m_CurrentProcessedBytes + m_CurrentSkippedBytes;
}

//------------------------------------------------------------------------------
//! Число обработанных байт (включая пропущенные).

qint64 TTaskStatus::TCounters::totalBytes() const
{
    return m_TotalProcessedBytes + m_TotalSkippedBytes;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//           T T a s k S t a t u s : : T R e a d e r S t a t
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.
/*!
   Конструирует экземпляр с обнулёнными счётчиками.
 */

TTaskStatus::TReaderStat::TReaderStat()
    : m_CurrentSize(-1)
{
}

//------------------------------------------------------------------------------
//! Очистка и обнуление счётчиков.

void TTaskStatus::TReaderStat::clear()
{
    m_Counters.clear();  m_RelName.clear();
    m_DirName.clear();   m_CurrentSize = -1;
}

//------------------------------------------------------------------------------
//! Финализация счётчиков.

void TTaskStatus::TReaderStat::end()
{
    m_Counters.end();
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//           T T a s k S t a t u s : : T W r i t e r S t a t
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TTaskStatus::TWriterStat::TWriterStat()
    : m_Size(-1)
{
}

//------------------------------------------------------------------------------
//! Проверка регистрации обработчика.

bool TTaskStatus::TWriterStat::isHandlerRegistered(const void* pHandler) const
{
    return m_CountersMap.contains(pHandler);
}

//------------------------------------------------------------------------------
//! Число зарегистрированных обработчиков.

int TTaskStatus::TWriterStat::handlersCount() const
{
    return m_CountersMap.count();
}

//------------------------------------------------------------------------------
//! Указатель на счётчики для указанного обработчика.
/*!
   \remarks Если обработчик не зарегистрирован, возвращает нулевой указатель.
 */

TTaskStatus::TCounters* TTaskStatus::TWriterStat::counters(const void* pHandler)
{
    TCountersMap::iterator It = m_CountersMap.find(pHandler);
    if (It != m_CountersMap.end())
        return &*It;

    return NULL;
}

//------------------------------------------------------------------------------
//! Указатель на счётчики для указанного обработчика.
/*!
   \remarks Если обработчик не зарегистрирован, возвращает нулевой указатель.

   \overload
 */

const TTaskStatus::TCounters* TTaskStatus::TWriterStat::counters(const void* pHandler) const
{
    TCountersMap::const_iterator It = m_CountersMap.constFind(pHandler);
    if (It != m_CountersMap.constEnd())
        return &*It;

    return NULL;
}

//------------------------------------------------------------------------------
//! Регистрация обработчика.
/*!
   Метод регистрирует обработчик и возвращает указатель на счётчики, связанные
   с этим обработчиком. Если обработчик уже зарегистрирован, новая группа
   счётчиков не создаётся.
 */

TTaskStatus::TCounters* TTaskStatus::TWriterStat::registerHandler(const void* pHandler)
{
    return &m_CountersMap[pHandler];
}

//------------------------------------------------------------------------------
//! Отмена регистрации обработчика.
/*!
   \return true, если обработчик успешно разрегистрирован и false, если
     обработчик не был зарегистрирован.
 */

bool TTaskStatus::TWriterStat::unregisterHandler(const void* pHandler)
{
    return m_CountersMap.remove(pHandler) > 0;
}

//------------------------------------------------------------------------------
//! Поиск самого медленного обработчика.
/*!
   Метод ищет самый медленный обработчик. Возвращает счётчики, связанные с этим
   обработчиком и в переменную *ppHandler заносит указатель на обработчик.
 */

const TTaskStatus::TCounters* TTaskStatus::TWriterStat::slowest(const void** ppHandler) const
{
    TCountersMap::const_iterator It = m_CountersMap.constBegin();
    if (It != m_CountersMap.end()) {
        int    Files = It->files();
        qint64 Bytes = It->currentBytes();
        TCountersMap::const_iterator ItSlowest = It;

        while (++It != m_CountersMap.constEnd()) {
            int Files2 = It->files();
            qint64 Bytes2 = It->currentBytes();
            if ((Files2 < Files) || ((Files2 == Files) && (Bytes2 < Bytes))) {
                Files = Files2;
                Bytes = Bytes2;
                ItSlowest = It;
            }
        }

        if (ppHandler != NULL)
            *ppHandler = ItSlowest.key();
        return &*ItSlowest;
    }

    return NULL;
}


//------------------------------------------------------------------------------
//! Очистка и обнуление счётчиков.

void TTaskStatus::TWriterStat::clear()
{
    m_RelName.clear();   m_Size = -1;
}

//------------------------------------------------------------------------------
//! Финализация счётчиков.

void TTaskStatus::TWriterStat::end()
{
    for (TCountersMap::iterator I = m_CountersMap.begin();
         I != m_CountersMap.end(); ++I)
    {
        I->end();
    }
    m_RelName.clear();   m_Size = -1;
}

//------------------------------------------------------------------------------
//! Начало обработки нового файла.
/*!
   \arg RelName Относительное (!) имя файла.
   \arg Size    Размер файла.
 */

void TTaskStatus::TWriterStat::newFile(const QString& RelName, qint64 Size)
{
    m_RelName = RelName;
    m_Size = Size;

    for (TCountersMap::iterator I = m_CountersMap.begin();
         I != m_CountersMap.end(); ++I)
    {
        I->nextNormal();
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//          T T a s k S t a t u s : : T T o t a l S t a t
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Конструктор.

TTaskStatus::TTotalStat::TTotalStat()
{
    clear();
}

//------------------------------------------------------------------------------
//! Добавление записанных байт.

void TTaskStatus::TTotalStat::addWritedBytes(qint64 WritedBytes)
{
    m_TotalWritedBytes += WritedBytes;
}

//------------------------------------------------------------------------------
//! Добавление записанных файлов.

void TTaskStatus::TTotalStat::addWritedFiles(int Count)
{
    m_TotalWritedFiles += Count;
}

//------------------------------------------------------------------------------
//! Очистка всех счётчиков.

void TTaskStatus::TTotalStat::clear()
{
    m_TotalWritedFiles = 0;
    m_TotalWritedBytes = 0;
}





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                      T T a s k S t a t u s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Статистика потока записи.
/*!
   \remarks Если статистика не найдена, возвращает нулевой указатель.
 */

TTaskStatus::TWriterStat* TTaskStatus::writerStat(const void* pWriter)
{
    TWriterStat* pStat;
    counters(pWriter, NULL, &pStat, NULL);
    return pStat;
}

//------------------------------------------------------------------------------
/*!
   \overload
 */

const TTaskStatus::TWriterStat* TTaskStatus::writerStat(const void *pWriter) const
{
    const TWriterStat* pStat;
    counters(pWriter, NULL, &pStat, NULL);
    return pStat;
}

//------------------------------------------------------------------------------
//! Поиск счётчиков.
/*!
   Метод возвращает указатель на группу счётчиков для комбинации потока записи
   pWriter и обработчика pHandler.

   \remarks Если счётчики не найдены, возвращает нулевой указатель.
*/

TTaskStatus::TCounters* TTaskStatus::counters(const void* pWriter,
                                              const void* pHandler)
{
    TCounters* pCounters;
    counters(pWriter, pHandler, NULL, &pCounters);
    return pCounters;
}

//------------------------------------------------------------------------------
/*!
   \overload
 */

const TTaskStatus::TCounters* TTaskStatus::counters(const void* pWriter,
                                                    const void* pHandler) const
{
    const TCounters* pCounters;
    counters(pWriter, pHandler, NULL, &pCounters);
    return pCounters;
}

//------------------------------------------------------------------------------
//! Поиск структур статистики.
/*!
   Основной метод поиска структур статистики. По возможности все остальные
   методы не должны искать счётчики самостоятельно, а должны использовать этот
   метод либо его "обёртки". Метод производит поиск структур статистики для
   потока записи pWriter с обработчиком pHandler. В переменную *ppWriterStat
   заносится указатель на статистику потока записи, а в переменную
   *ppCounters - указатель на счётчики обработчика. Метод возвращает true, если
   требуемые структуры найдены и false в противном случае.

   \remarks Параметры ppWriterStat и ppCounters могут быть нулевыми.

   \remarks Если параметр ppCounters нулевой, то поиск структуры счётчиков
     не производится и параметр pHandler игнорируется. Метод вернёт true, если
     ему удалось найти структуру статистики, связанную с потоком записи pWriter.
 */

bool TTaskStatus::counters(const void* pWriter, const void* pHandler,
                           TWriterStat** ppWriterStat,
                           TCounters**   ppCounters)
{
    Q_ASSERT(!m_WritersMutex.tryLock());

    TWriterStat* pWriterStat = NULL;
    TCounters*   pCounters   = NULL;
    bool         Result      = false;

    TWritersStat::iterator ItStat = m_WritersStat.find(pWriter);
    if (ItStat != m_WritersStat.end()) {
        pWriterStat = &*ItStat;

        if (ppCounters != NULL) {
            pCounters = ItStat->counters(pHandler);
            Result = pCounters != NULL;
        }
        else
            Result = true;
    }

    if (ppWriterStat != NULL)
        *ppWriterStat = pWriterStat;
    if (ppCounters != NULL)
        *ppCounters = pCounters;

    return Result;
}

//------------------------------------------------------------------------------
/*!
   \overload
 */

bool TTaskStatus::counters(const void* pWriter, const void *pHandler,
                           const TWriterStat **ppWriterStat,
                           const TCounters**   ppCounters) const
{
    Q_ASSERT(!m_WritersMutex.tryLock());

    const TWriterStat* pWriterStat = NULL;
    const TCounters*   pCounters   = NULL;
    bool               Result      = false;

    TWritersStat::const_iterator ItStat = m_WritersStat.constFind(pWriter);
    if (ItStat != m_WritersStat.constEnd()) {
        pWriterStat = &*ItStat;

        if (ppCounters != NULL) {
            pCounters = ItStat->counters(pHandler);
            Result = pCounters != NULL;
        }
        else
            Result = true;
    }

    if (ppWriterStat != NULL)
        *ppWriterStat = pWriterStat;
    if (ppCounters != NULL)
        *ppCounters = pCounters;

    return Result;
}

//------------------------------------------------------------------------------
//! Процент завершения чтения текущего файла.
/*!
   \remarks Если размер файла неизвестен, возвращает -1.
 */

float TTaskStatus::currentReadedPercent_Private() const
{
    Q_ASSERT(!m_ReaderMutex.tryLock());

    return percent(m_ReaderStat.counters()->currentBytes(),
                   m_ReaderStat.currentSize());
}

//------------------------------------------------------------------------------
//! Число полностью обработанных потоком чтения файлов.
/*!
   \remarks Пропущенные файлы также учитываются.
 */

int TTaskStatus::readedFiles_Private() const
{
    Q_ASSERT(!m_ReaderMutex.tryLock());

    return m_ReaderStat.counters()->files();
}

//------------------------------------------------------------------------------
//! Общее число обработчиков.
/*!
   Метод возвращает общее число зарегистрированных обработчиков.
 */

int TTaskStatus::handlersCount_Private() const
{
    Q_ASSERT(!m_WritersMutex.tryLock());

    int Result = 0;
    for (TWritersStat::const_iterator I = m_WritersStat.constBegin();
         I != m_WritersStat.constEnd(); ++I)
    {
        Result += I->handlersCount();
    }
    return Result;
}

//------------------------------------------------------------------------------
//! Поиск самого медленного процесса записи.
/*!
   Метод ищет самый медленный процесс и его обработчик. В переменные
   *ppWriter и *ppCounters помещаются указатели на структуру статистики и
   структуру счётчиков этого процесса и обработчика. Возвращает число
   обработанных этим процессом байт в текущем файле (в том числе пропущенных).

   \remarks Параметры ppWriter и ppCounter могут быть равны NULL.
 */

qint64 TTaskStatus::slowestWriter(const TWriterStat** ppWriterStat,
                                  const TCounters** ppCounters) const
{
    Q_ASSERT(!m_WritersMutex.tryLock());

    const TWriterStat* pWriterStat = NULL;
    const TCounters*   pCounters   = NULL;
    qint64             Bytes       = 0;

    TWritersStat::const_iterator It = m_WritersStat.constBegin();
    if (It != m_WritersStat.constEnd()) {
        pWriterStat = &*It;
        pCounters = It->slowest();
        int Files = pCounters->files();
        Bytes = pCounters->currentBytes();

        while (++It != m_WritersStat.constEnd()) {
            const TCounters* p = It->slowest();
            int Files2 = p->files();
            qint64 Bytes2 = p->currentBytes();
            if ((Files2 < Files) || ((Files2 == Files) && (Bytes2 < Bytes))) {
                Files = Files2;
                Bytes = Bytes2;
                pCounters = p;
                pWriterStat = &*It;
            }
        }
    }

    if (ppWriterStat != NULL)
        *ppWriterStat = pWriterStat;
    if (ppCounters != NULL)
        *ppCounters = pCounters;

    return Bytes;
}

//------------------------------------------------------------------------------
//! Внутренний метод отмены регистрации потока записи.

void TTaskStatus::unregisterWriter_Private(const void* pWriter, const TCounters* pCounters)
{
    Q_ASSERT(!m_WritersMutex.tryLock());
    Q_ASSERT(m_WritersStat.contains(pWriter));

    if (m_WritersStat.count() == 1)
    {
        // Отмена регистрации последнего потока записи.
        const TCounters* pLastCounters;
        if (pCounters != NULL)
            pLastCounters = pCounters;
        else
            slowestWriter(NULL, &pLastCounters);

        if (pLastCounters != NULL) {
            m_FinalStat.Files = pLastCounters->files();
            m_FinalStat.Bytes = pLastCounters->totalBytes();
        }
        else {
            qWarning("TTaskStatus::unregisterWriter_Private. "
                     "Unregistering last writer without last counters.");
        }
    }
    m_WritersStat.remove(pWriter);
}

//------------------------------------------------------------------------------
//! Конструктор.

TTaskStatus::TTaskStatus()
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TTaskStatus::~TTaskStatus()
{
}

//------------------------------------------------------------------------------
//! Регистрация потока записи pWriter с обработчиком pHandler.
/*!
   Метод возвращает true, если регистрация выполнено успешно и false, если
   данная комбинация потока и обработчика уже зарегистрирована.
 */

bool TTaskStatus::registerWriter(const void* pWriter, const void* pHandler)
{
    // Регистрация после запуска собьёт работу счётчиков.
    Q_ASSERT(m_TimeCounter.isStopped());

    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TWriterStat* pStat = &m_WritersStat[pWriter];
    if (!pStat->isHandlerRegistered(pHandler)) {
        pStat->registerHandler(pHandler);
        return true;
    }
    else {
        qWarning("TTaskStatus::registerWriter. Attempt to register writer "
                 "%p with handler %p, which is already registered.",
                 pWriter, pHandler);
        return false;
    }
}

//------------------------------------------------------------------------------
//! Регистрация потока записи pWriter со списком обработчиков Handlers.
/*!
   Метод возвращает число успешно зарегистрированных обработчиков. (Если
   некоторая комбинация потока и обработчика уже зарегистрирована, её повторная
   регистрация не производится.)
 */

int TTaskStatus::registerWriter(const void* pWriter,
                                QList<const void*> Handlers)
{
    // Регистрация после запуска собьёт работу счётчиков.
    Q_ASSERT(m_TimeCounter.isStopped());

    if (Handlers.count() == 0) {
        qWarning("TTaskStatus::registerWriter. Attempt to register writer "
                 "%p with empty handlers list.", pWriter);
        return 0;
    }

    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TWriterStat* pStat = &m_WritersStat[pWriter];
    int Count = 0;
    for (int i = 0; i < Handlers.count(); ++i) {
        if (!pStat->isHandlerRegistered(Handlers[i])) {
            pStat->registerHandler(Handlers[i]);
            ++Count;
        }
        else {
            qWarning("TTaskStatus::registerWriter. Attempt to register writer "
                     "%p with handler %p, which is already registered.",
                     pWriter, Handlers[i]);
        }
    }

    return Count;
}

//------------------------------------------------------------------------------
//! Отмена регистрации потока записи pWriter с обработчиком pHandler.
/*!
   Метод возвращает true, если отмена регистрации выполнено успешно и false,
   если данная комбинация потока и обработчика зарегистрирована не была.
 */

bool TTaskStatus::unregisterWriter(const void* pWriter, const void* pHandler)
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TWriterStat* pStat = writerStat(pWriter);
    if (pStat != NULL) {
        if (pStat->unregisterHandler(pHandler)) {
            if (pStat->handlersCount() == 0)
                m_WritersStat.remove(pWriter);
            return true;
        }
        else {
            qWarning("TTaskStatus::unregisterWriter. Attempt to unregister "
                     "writer %p with handler %p, which is not registered.",
                     pWriter, pHandler);
        }
    }
    else {
        qWarning("TTaskStatus::unregisterWriter. Attempt to unregister "
                 "writer %p, which is not registered.", pWriter);
    }

    return false;
}

//------------------------------------------------------------------------------
//! Отмена регистрации потока записи pWriter со списком обработчиков Handlers.
/*!
   Метод возвращает число успешно разрегистрированных обработчиков. (Если
   некоторая комбинация потока и обработчика уже разрегистрирована, её повторная
   разрегистрация не производится.)
 */

int TTaskStatus::unregisterWriter(const void* pWriter,
                                  QList<const void*> Handlers)
{
    if (Handlers.count() == 0) {
        qWarning("TTaskStatus::unregisterWriter. Attempt to unregister writer "
                 "%p with empty handlers list.", pWriter);
        return 0;
    }

    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    int Removed = 0;
    TWriterStat* pStat = writerStat(pWriter);

    // Если поток записи последний, сохраняем его наименьший счётчик.
    TCounters LastCounter;
    if (m_WritersStat.count() == 1) {
        const TCounters* pCounters = NULL;
        slowestWriter(NULL, &pCounters);
        LastCounter = *pCounters;
    }

    if (pStat != NULL)
    {
        for (int i = 0; i < Handlers.count(); ++i) {
            if (pStat->unregisterHandler(Handlers[i]))
                ++Removed;
            else
                qWarning("TTaskStatus::unregisterWriter. Attempt to unregister "
                         "writer %p with handler %p, which is not registered.",
                         pWriter, Handlers[i]);
        }

        if (pStat->handlersCount() == 0) {
            unregisterWriter_Private(pWriter, &LastCounter);
            m_WritersStat.remove(pWriter);
        }
    }
    else {
        qWarning("TTaskStatus::unregisterWriter. Attempt to unregister "
                 "writer %p, which is not registered.", pWriter);
    }
    return Removed;
}

//------------------------------------------------------------------------------
//! Отмена регистрации потока записи pWriter со всеми его обработчиками.
/*!
   Метод возвращает true, если отмена регистрации выполнено успешно и false,
   если данный поток записи зарегистрирован не был.
 */

bool TTaskStatus::unregisterWriter(const void* pWriter)
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    if (m_WritersStat.contains(pWriter)) {
        unregisterWriter_Private(pWriter);
        return true;
    }
    else {
        qWarning("TTaskStatus::unregisterWriter. Attempt to unregister writer "
                 "%p, which was not registered.", pWriter);
        return false;
    }
}

//------------------------------------------------------------------------------
//! Проверка регистрации потока записи.
/*!
   Метод возвращает true, если поток записи pWriter зарегистрирован хотя бы с
   одним обработчиком.
 */

bool TTaskStatus::isWriterRegistered(const void* pWriter) const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    return m_WritersStat.contains(pWriter);
}

//------------------------------------------------------------------------------
//! Проверка регистрации комбинации потока записи pWriter с обработчиком pHandler.

bool TTaskStatus::isWriterRegistered(const void* pWriter,
                                     const void* pHandler) const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    const TWriterStat* pStat = writerStat(pWriter);
    if (pStat != NULL)
        return pStat->isHandlerRegistered(pHandler);

    return false;
}

//------------------------------------------------------------------------------
//! Отмена регистрации всех потоков записи.

void TTaskStatus::unregisterAllWriters()
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    m_WritersStat.clear();
}

//------------------------------------------------------------------------------
//! Число зарегистрированных потоков записи.

int TTaskStatus::writersCount() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    return m_WritersStat.count();
}

//------------------------------------------------------------------------------
//! Число зарегистрированных обработчиков потока записи pWriter.

int TTaskStatus::handlersCount(const void* pWriter) const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    const TWriterStat* pStat = writerStat(pWriter);
    if (pStat != NULL)
        return pStat->handlersCount();
    else
        return 0;
}

//------------------------------------------------------------------------------
//! Общее число обработчиков.

int TTaskStatus::handlersCount() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    return handlersCount_Private();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Начало обработки задания.
/*!
   Поток чтения должен вызвать этот метод чтобы просигнализировать о начале
   обработки задания.
 */

void TTaskStatus::readerBeginTask()
{
}

//------------------------------------------------------------------------------
//! Начало чтения файла.
/*!
   Поток чтения должен вызвать этот метод после того, как он успешно откроет
   новый файл. Если файл открыть не удалось, должен быть вызван метод
   \c readerSkipFile. В параметре Size можно передать размер файла. Если размер
   файла неизвестен, нужно передать значение -1.
 */

void TTaskStatus::readerNewFile(const QString& DirName, const QString& RelName,
                                qint64 Size)
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    m_ReaderStat.counters()->nextNormal();
    m_ReaderStat.setDirName(DirName);
    m_ReaderStat.setRelName(RelName);
    m_ReaderStat.setCurrentSize(Size);
}

//------------------------------------------------------------------------------
//! Прогресс чтения.
/*!
   Поток чтения должен вызвать этот метод после того, как он успешно прочитает
   новую порцию данных. В параметре DeltaBytes передаётся число прочитанных
   байт. Если поток чтения не может прочитать файл до конца, он должен вызвать
   метод \c readerSkip.
 */

void TTaskStatus::readerProgress(qint64 DeltaBytes)
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    m_ReaderStat.counters()->addBytes(DeltaBytes);
}

//------------------------------------------------------------------------------
//! Пропуск файла потоком чтения.
/*!
   Поток чтения должен вызвать этот метод, если он не может прочитать очередной
   файл. В параметре Size поток может передать размер файла.
 */

void TTaskStatus::readerSkipFile(const QString& DirName, const QString& RelName,
                                 qint64 Size)
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    m_ReaderStat.counters()->nextSkipped(Size);
    m_ReaderStat.setDirName(DirName);
    m_ReaderStat.setRelName(RelName);
}

//------------------------------------------------------------------------------
//! Пропуск части файла потоком чтения.
/*!
   Поток чтения должен вызвать этот метод, если он не может прочитать очередной
   блок из файла. В параметре DeltaBytes передаётся размер пропускаемого блока.

   \remarks Если нужно пропустить весь файл, вызовите \c readerSkipFile.
 */

void TTaskStatus::readerSkip(qint64 DeltaBytes)
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    m_ReaderStat.counters()->skipBytes(DeltaBytes);
}

//------------------------------------------------------------------------------
//! Завершение чтения файла.
/*!
   Поток чтения может вызвать этот метод чтобы просигнализировать о завершении
   чтения файла.
 */

void TTaskStatus::readerEndFile()
{
    m_ReaderStat.end();
}

//------------------------------------------------------------------------------
//! Завершение работы потока чтения.
/*!
   Поток чтения должен вызвать этот метод чтобы просигнализировать о завершении
   своей работы.
 */

void TTaskStatus::readerEndTask()
{

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Начало обработки задания потоком записи.

void TTaskStatus::writerBeginTask(const void* pWriter)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerBeginTask. "
                 "Calling thread differs from method argument.");
    }
    if (!isWriterRegistered(pWriter)) {
        qWarning("TTaskStatus::writerBeginTask. Writer %p is not registered.",
                 pWriter);
    }

}

//------------------------------------------------------------------------------
//! Начало обработки нового файла потоком записи.
/*!
   Поток записи должен вызвать этот метод ПЕРЕД тем, как он перейдёт к
   обработке нового файла-источника и создаст хотя бы один файл-назначение.
   После открытия нового файла поток должен будет вызвать либо метод
   \c writerNewFile(const void*, const void*), либо метод
   \с writerSkipFile(const void*, const void*).

   \arg pWriter  Указатель на поток записи.
   \arg FileName Имя файла (путь относительно каталога-назначения).
   \arg Size     Размер файла.

   \remarks Если размер файла неизвестен, его можно не указывать.
 */

void TTaskStatus::writerNewFile(const void* pWriter, const QString& RelName,
                                qint64 Size)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerNewFile. "
                 "Calling thread differs from method argument.");
    }


    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TWriterStat* pStat = writerStat(pWriter);
    if (pStat != NULL)
    {
        pStat->newFile(RelName, Size);
    }
    else {
        qWarning("TTaskStatus::writerNewFile. Writer %p is not registered.",
                 pWriter);
    }
}

//------------------------------------------------------------------------------
//! Начало обработки нового файла.
/*!
   Поток записи должен вызвать этот метод ПОСЛЕ того, как он успешно откроет
   новый файл-назначение. ДО этого метода должен быть вызван метод
   \c writerNewFile(const void*, const QString&, qint64).

   \sa writerSkipFile
 */

void TTaskStatus::writerNewFile(const void* pWriter, const void* pHandler)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerNewFile. "
                 "Calling thread differs from method argument.");
    }


    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TCounters* pCounters = counters(pWriter, pHandler);
    if (pCounters != NULL) {
        pCounters->nextNormal();
    }
    else {
        qWarning("TTaskStatus::writerNewFile. Writer %p with handler %p "
                 "is not registered.", pWriter, pHandler);
    }
}

//------------------------------------------------------------------------------
//! Прогресс записи.
/*!
   Поток записи должен вызвать этот метод после того, как он успешно запишет
   новую порцию данных. В параметре DeltaBytes передаётся число записанных
   байт. Если поток не может записать файл до конца, он должен вызвать
   метод \c writerSkip.
 */

void TTaskStatus::writerProgress(const void* pWriter, const void* pHandler,
                                 qint64 DeltaBytes)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerProgress. "
                 "Calling thread differs from method argument.");
    }


    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TCounters* pCounters = counters(pWriter, pHandler);
    if (pCounters) {
        pCounters->addBytes(DeltaBytes);
        m_TotalStat.addWritedBytes(DeltaBytes);
    }
    else {
        qWarning("TTaskStatus::writerProgress. Writer %p with handler %p "
                 "is not registered.", pWriter, pHandler);
    }
}

//------------------------------------------------------------------------------
//! Пропуск файла одним из обработчиков потока записи.
/*!
   Поток записи должен вызвать этот метод, если один из его обработчиков не
   может записать очередной файл.

   \remarks Метод следует вызывать только в том случае, когда обработчик ещё до
     записи хотя бы одной порции данных отменил запись в файл (файл не удалось
     создать, недостаточно свободного места, команда пользователя и т.п.). Если
     в файл уже было что-то записано, нужно вызвать метод
     \c writerSkip(const void*, const void*, qint64).
 */

void TTaskStatus::writerSkipFile(const void *pWriter, const void *pHandler)
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TCounters* pCounters = counters(pWriter, pHandler);
    if (pCounters != NULL) {
        pCounters->nextSkipped(m_WritersStat[pWriter].size());
    }
    else {
        qWarning("TTaskStatus::writerSkipFile. Writer %p with handler %p "
                 "is not registered.", pWriter, pHandler);
    }
}

//------------------------------------------------------------------------------
//! Пропуск части файла одним из обработчиков потока записи.
/*!
   Поток записи должен вызвать этот метод, если один из его обработчиков не
   может записать очередной блок в файл. В параметре DeltaBytes передаётся
   размер пропускаемого блока.

   \remarks Если нужно пропустить весь файл, вызовите \c writerSkipFile.
 */

void TTaskStatus::writerSkip(const void *pWriter, const void *pHandler,
                             qint64 DeltaBytes)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerSkip. "
                 "Calling thread differs from method argument.");
    }


    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TCounters* pCounters = counters(pWriter, pHandler);
    if (pCounters) {
        pCounters->skipBytes(DeltaBytes);
    }
    else {
        qWarning("TTaskStatus::writerSkip. Writer %p with handler %p "
                 "is not registered.", pWriter, pHandler);
    }
}

//------------------------------------------------------------------------------
//! Завершение записи файла одним из обработчиков потока записи.
/*!
   Поток записи может вызвать этот метод чтобы просигнализировать о завершении
   записи файла одним из своих обработчиков.
 */

void TTaskStatus::writerEndFile(const void* pWriter, const void* pHandler)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerEndFile. "
                 "Calling thread differs from method argument.");
    }


    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TCounters* pCounters = counters(pWriter, pHandler);
    if (pCounters != NULL)
    {
        pCounters->end();
        if (pCounters->currentSkippedBytes() <= 0)
            m_TotalStat.addWritedFiles();
    }
    else {
        qWarning("TTaskStatus::writerEndFile. "
                 "Writer %p with handler %p is not registered.",
                 pWriter, pHandler);
    }
}

//------------------------------------------------------------------------------
//! Завершение работы потока записи.
/*!
   Поток записи может вызвать этот метод чтобы просигнализировать о завершении
   своей работы.
 */

void TTaskStatus::writerEndTask(const void* pWriter)
{
    // Проверка некритичных условий.
    if (pWriter != QThread::currentThread()) {
        qWarning("TTaskStatus::writerEndTask. "
                 "Calling thread differs from method argument.");
    }


    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    //writerEndTask_Private(pWriter);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Начало задания.
/*!
   Метод должен быть вызван перед началом обработки задания.
 */

void TTaskStatus::begin()
{
    QMutexLocker Locker1(&m_ReaderMutex);
    Q_UNUSED(Locker1);
    QMutexLocker Locker2(&m_WritersMutex);
    Q_UNUSED(Locker2);

    m_ReaderStat.clear();
    for (TWritersStat::iterator I = m_WritersStat.begin();
         I != m_WritersStat.end(); ++I)
    {
        I->clear();
    }
    m_TotalStat.clear();
    m_TimeCounter.start();
}

//------------------------------------------------------------------------------
//! Завершение задания.
/*!
   Этот метод должен быть вызван после того, как задание копирования будет
   полностью завершено. Метод заносит данные о завершённом задании в глобальную
   статистику.

   \remarks После вызова этого метода статистика не очищается. Для очистки
     статистики перед началом нового задания вызовите метод \c begin.
 */

void TTaskStatus::end()
{
    QMutexLocker Locker1(&m_ReaderMutex);
    Q_UNUSED(Locker1);
    QMutexLocker Locker2(&m_WritersMutex);
    Q_UNUSED(Locker2);

    m_TimeCounter.stop();

    const TCounters* pCounters;
    slowestWriter(NULL, &pCounters);
    if (pCounters != NULL) {
        m_FinalStat.Files = pCounters->files();
        m_FinalStat.Bytes = pCounters->totalBytes();
    }

    TGlobalStatistics* pGS = TGlobalStatistics::instance();
    if (pGS != NULL) {
        TGlobalStatistics::TStat Stat;
        Stat.BytesReaded    = m_ReaderStat.counters()->totalProcessedBytes();
        Stat.FilesReaded    = m_ReaderStat.counters()->filesCompleted();
        Stat.BytesWrited    = m_TotalStat.writedBytes();
        Stat.FilesWrited    = m_TotalStat.writedFiles();
        Stat.TasksCompleted = 1;
        pGS->append(Stat);
    }
    else {
        qWarning("TTaskStatus::end. Can't get global statistics.");
    }
}

//------------------------------------------------------------------------------
//! Очистка.
/*!
   Метод приводит экземпляр класса в исходное состояние, отменяя регистрацию
   всех потоков записи и обнуляя счётчики.
 */

void TTaskStatus::clear()
{
    QMutexLocker Locker1(&m_ReaderMutex);
    Q_UNUSED(Locker1);
    QMutexLocker Locker2(&m_WritersMutex);
    Q_UNUSED(Locker2);

    m_WritersStat.clear();
    m_ReaderStat.clear();
    m_TimeCounter.clear();
    clearTaskSize();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Приостановка таймеров.

void TTaskStatus::pause()
{
    m_TimeCounter.pause();
}

//------------------------------------------------------------------------------
//! Возобновление работы таймеров.

void TTaskStatus::resume()
{
    m_TimeCounter.resume();
}

//------------------------------------------------------------------------------
//! Флаг запуска таймеров.

bool  TTaskStatus::isStarted() const
{
    return m_TimeCounter.isStarted();
}

//------------------------------------------------------------------------------
//! Флаг приостановки работы таймеров.

bool TTaskStatus::isPaused() const
{
    return m_TimeCounter.isPaused();
}

//------------------------------------------------------------------------------
//! Скорость и время обработки.
/*!
   Метод заполняет поля структуры \c TSpeedAndTime, переданной в метод
   по указателю.
 */

void TTaskStatus::speedAndTime(TSpeedAndTime* pSpeedAndTime) const
{
    if (pSpeedAndTime == NULL) {
        qWarning("TTaskStatus::speedAndTime. Pointer to TSpeedAndTime is NULL.");
        return;
    }

    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    const TCounters* pCounters;
    slowestWriter(NULL, &pCounters);
    if (pCounters) {
        pSpeedAndTime->ElapsedTime = m_TimeCounter.msec();

        qint64 speed = 0;
        if (pSpeedAndTime->ElapsedTime > 0) {
            speed = pCounters->totalProcessedBytes() / pSpeedAndTime->ElapsedTime;
            pSpeedAndTime->Speed = 1000 * speed;
        }
        else
            pSpeedAndTime->Speed = 0;

        if (speed > 0)
            pSpeedAndTime->RemainingTime = (m_TaskSize.TotalSize - pCounters->totalBytes()) / speed;
        else
            pSpeedAndTime->RemainingTime = -1;
    }
    else {
        pSpeedAndTime->ElapsedTime   = -1;
        pSpeedAndTime->RemainingTime = -1;
        pSpeedAndTime->Speed         = 0;
    }
}

//------------------------------------------------------------------------------
//! Скорость и время обработки.

TTaskStatus::TSpeedAndTime TTaskStatus::speedAndTime() const
{
    TSpeedAndTime Result;
    speedAndTime(&Result);
    return Result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Имя текущего файла-источника.
/*!
   \sa readedFiles, currentReadedBytes, currentReadedPercent, readingStatus
 */

QString TTaskStatus::readedFileName() const
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    return AddWithSeparator(m_ReaderStat.dirName(), m_ReaderStat.relName());
}

//------------------------------------------------------------------------------
//! Относительное имя текущего файла-источника.

QString TTaskStatus::readedFileRelName() const
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    return m_ReaderStat.relName();
}

//------------------------------------------------------------------------------
//! Имя каталога текущего файла-источника.

QString TTaskStatus::readedFileDirName() const
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    return m_ReaderStat.dirName();
}

//------------------------------------------------------------------------------
//! Число полностью прочитанных файлов.
/*!
   \sa readedFileName, currentReadedBytes, currentReadedPercent, readingStatus
 */

int TTaskStatus::readedFiles() const
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    return readedFiles_Private();
}

//------------------------------------------------------------------------------
//! Процент прочитанных из текущего файла-источника байт.
/*!
   \remarks Если размер файла неизвестен, метод вернёт -1.

   \sa readedFileName, readedFiles, currentReadedBytes, readingStatus
 */

float TTaskStatus::currentReadedPercent() const
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    return currentReadedPercent_Private();
}

//------------------------------------------------------------------------------
//! Статус операции чтения.
/*!
   \sa readedFileName, readedFiles, currentReadedBytes, currentReadedPercent
 */

TTaskStatus::TStatus TTaskStatus::readingStatus() const
{
    QMutexLocker Locker(&m_ReaderMutex);
    Q_UNUSED(Locker);

    TStatus Status;
    Status.DirName  = m_ReaderStat.dirName();
    Status.RelName  = m_ReaderStat.relName();
    Status.Files    = readedFiles_Private();
    Status.Bytes    = m_ReaderStat.counters()->currentBytes();
    Status.Percent  = currentReadedPercent_Private();
    Status.TotalBytes = m_ReaderStat.counters()->totalBytes();
    if (m_TaskSize.TotalSize > 0)
        Status.TotalPercent = percent(Status.TotalBytes, m_TaskSize.TotalSize);
    else
        Status.TotalPercent = -1;

    return Status;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Имя файла, обрабатываемого самым медленным потоком записи.
/*!
   \sa slowestWritedFiles, slowestCurrentWritedBytes,
       slowestCurrentWritedPercent, writingStatus
 */

QString TTaskStatus::slowestWritedFileName() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    const TWriterStat* pWriterStat;
    slowestWriter(&pWriterStat);
    if (pWriterStat)
        return pWriterStat->relName();
    else
        return QString();
}

//------------------------------------------------------------------------------
//! Число файлов, обработанных самым медленным потоком записи.
/*!
   \sa slowestWritedFileName, slowestCurrentWritedBytes,
       slowestCurrentWritedPercent, writingStatus
 */

int TTaskStatus::slowestWritedFiles() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    const TCounters* pCounters;
    slowestWriter(NULL, &pCounters);
    if (pCounters)
        return pCounters->filesCompleted();
    else
        return -1;
}

//------------------------------------------------------------------------------
//! Число байт, обработанных самым медленным потоком записи.
/*!
   \sa slowestWritedFileName, slowestWritedFiles,
       slowestCurrentWritedPercent, writingStatus
 */

qint64 TTaskStatus::slowestCurrentWritedBytes() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    return slowestWriter();
}

//------------------------------------------------------------------------------
//! Процент завершения обработки файла самым медленным процессом записи.
/*!
   \remarks Если размер файла неизвестен, возвращает -1.

   \sa slowestWritedFileName, slowestWritedFiles,
       slowestCurrentWritedBytes, writingStatus
 */

float TTaskStatus::slowestCurrentWritedPercent() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    const TWriterStat* pWriterStat;
    qint64 Bytes = slowestWriter(&pWriterStat);
    if (pWriterStat)
        return percent(Bytes, pWriterStat->size());
    else
        return -1;
}

//------------------------------------------------------------------------------
//! Статус операции записи.
/*!
   \sa slowestWritedFileName, slowestWritedFiles,
       slowestCurrentWritedBytes, slowestCurrentWritedPercent
 */

TTaskStatus::TStatus TTaskStatus::writingStatus() const
{
    QMutexLocker Locker(&m_WritersMutex);
    Q_UNUSED(Locker);

    TStatus Result;
    const TWriterStat* pWriterStat;
    const TCounters*   pCounters;
    Result.Bytes    = slowestWriter(&pWriterStat, &pCounters);
    if (pWriterStat) {
        Result.RelName = pWriterStat->relName();
        Result.Percent = percent(Result.Bytes, pWriterStat->size());
    }
    if (pCounters) {
        Result.Files    = pCounters->files();
        Result.TotalBytes = pCounters->totalBytes();
        if (m_TaskSize.TotalSize > 0)
            Result.TotalPercent = percent(Result.TotalBytes, m_TaskSize.TotalSize);
        else
            Result.TotalPercent = -1;
    }
    else {
        Result.Files = m_FinalStat.Files;
        Result.Percent = 100;
        Result.TotalBytes = m_FinalStat.Bytes;
        Result.TotalPercent = percent(Result.TotalBytes, m_TaskSize.TotalSize);
    }
    return Result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Размер задания.

TTaskSize TTaskStatus::taskSize() const
{
    return m_TaskSize;
}

//------------------------------------------------------------------------------
//! Указатель на размер задания.
/*!
   Указатель остаётся валидным на протяжении всего времени существования
   экземпляра класса.
 */

const TTaskSize* TTaskStatus::taskSizePtr() const
{
    return &m_TaskSize;
}

//------------------------------------------------------------------------------
//! Установка размера задания.
/*!
   \remarks Если указатель на размер задания нулевой, метод очистит размер
     задания.
 */
void TTaskStatus::setTaskSize(const TTaskSize* pTaskSize)
{
    if (pTaskSize == NULL)
        m_TaskSize.clear();
    else
        m_TaskSize = *pTaskSize;
}

//------------------------------------------------------------------------------
//! Установка размера задания.
/*!
   \overload
 */

void TTaskStatus::setTaskSize(const TTaskSize& TaskSize)
{
    setTaskSize(&TaskSize);
}

//------------------------------------------------------------------------------
//! Очистка размера задания.

void TTaskStatus::clearTaskSize()
{
    m_TaskSize.clear();
}

//------------------------------------------------------------------------------
