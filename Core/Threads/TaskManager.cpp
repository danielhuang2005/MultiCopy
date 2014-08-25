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

#include "TaskManager.hpp"

#include <QSemaphore>
#include <QHash>

#include "Core/Common/CommonFn.hpp"
#include "Core/Buffer/CircularBuffer.hpp"
#include "Core/Threads/SizeCalculator.hpp"
#include "Core/Threads/Reader.hpp"
#include "Core/Threads/Writer.hpp"
#include "Core/Task/TaskStatus.hpp"
#include "Core/Errors/ErrorHandler.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//            T T a s k M a n a g e r : : T D e s t s C o u n t e r
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Конструктор.

TTaskManager::TDestsCounter::TDestsCounter()
    : m_Dests(0), m_Skipped(0)
{
}

//------------------------------------------------------------------------------
//! Инициализация экземпляра класса.
/*!
   \arg DestsCount Число назначений.
 */

void TTaskManager::TDestsCounter::init(int DestsCount)
{
    m_Dests   = DestsCount;
    m_Skipped = 0;
    m_FileRelName.clear();
}

//------------------------------------------------------------------------------
//! Пропуск файла.
/*!
   Пропуск файла с относительным именем FileRelName одним из потоков записи.
   Поскольку экземпляр объекта TReader может быть только один на задание и
   обработку он ведёт последовательно, можно обрабатывать пропуск только
   текущего файла. Действительно, если имя обрабатываемого потоком записи файла
   не совпадает с именем файла, обрабатываемого потоком чтения, то отменять
   чтение уже поздно, тот файл уже целиком прочитан.

   \return Число уже отменённых записей.
 */

int TTaskManager::TDestsCounter::skip(const QString& FileRelName)
{
    if (FileRelName == m_FileRelName)
        ++m_Skipped;
    else {
        m_Skipped = 1;
        m_FileRelName = FileRelName;
    }
    return m_Skipped;
}

//------------------------------------------------------------------------------
//! Очистка.

void TTaskManager::TDestsCounter::clear()
{
    m_Dests   = 0;
    m_Skipped = 0;
    m_FileRelName.clear();
}

//------------------------------------------------------------------------------

void TTaskManager::TDestsCounter::cancelDest()
{
    --m_Dests;
    if (m_Dests < 0) {
        qWarning("TTaskManager::TDestsCounter::cancelDest. "
                 "Destinations count is negative (%i).", m_Dests);
    }
}




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                        T T a s k M a n a g e r
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Автоматический выбор размера кольцевого буфера.

void TTaskManager::bufferSizeAutodetect(int* pCellCount, int* pCellSize)
{
    Q_ASSERT(pCellCount != NULL);
    Q_ASSERT(pCellSize != NULL);

    qint64 AvailPhys = AvailablePhysicalMemory();
    *pCellSize = 4*1024*1024;  // 4Mb

    if (AvailPhys <= 2 * 2 * 4*1024*1024) { // 2 x 2 x 4Mb = 16Mb
        *pCellCount = 2;
    }
    else {
        if (AvailPhys >= 2 * 32 * 4*1024*1024) { // 2 x 32 x 4Mb = 256Mb
            *pCellCount = 32;
        }
        else {
            *pCellCount = AvailPhys / *pCellSize / 2;
        }
    }

    qDebug("TTaskManager::bufferSizeAutodetect: %i x %i bytes (%s).",
           *pCellCount, *pCellSize,
           qPrintable(sizeToStr((*pCellCount) * (*pCellSize))));
}

//------------------------------------------------------------------------------
//! Проверка свободного места.

void TTaskManager::checkFreeSpace(QStringList* pDestList)
{
    Q_ASSERT(pDestList != NULL);
    Q_ASSERT(m_pSizeCalc != NULL);

    if (!m_pCurrentTask->TaskSettings.CheckFreeSpace)
        return;

    typedef QMap<QString, QStringList> TMountPoints;
    TMountPoints MountPoints;
    for (int i = 0; i < pDestList->count(); ++i) {
        QString MountPoint = GetDriveByPath(pDestList->at(i));
        if (!MountPoint.isEmpty())
            MountPoints[MountPoint].append(pDestList->at(i));
        else
            qWarning("TTaskManager::checkFreeSpace. "
                     "Cannot determine mount point for destination \"%s\".",
                     qPrintable(pDestList->at(i)));
    }

    qint64 TaskBytes = m_pSizeCalc->taskSize().TotalSize;
    TMountPoints::iterator I = MountPoints.begin();
    while (I != MountPoints.end())
    {
        qint64 Required = TaskBytes * I->count();
        do {
            qint64 FreeSpace = DiskFreeSpace(I.key());
            if (Required <= FreeSpace) {
                ++I;
                break;
            }
            else {
                TErrorData ErrorData;
                ErrorData.ErrorCode = ecNoFreeSpace;
                ErrorData.Message = tr("Disk %1 (required %2, free %3)")
                                    .arg(I.key(),
                                         sizeToStr(Required),
                                         sizeToStr(FreeSpace));
                ErrorData.pSender = this;
                ErrorData.DestsCount = MountPoints.count();

                QSemaphore Semaphore;
                ErrorData.pLocker = &Semaphore;
                processNextError(&ErrorData);
                Semaphore.acquire();

                if (ErrorData.Action != eaRetry) {
                    if (ErrorData.Action == eaCancelDest) {
                        for (int i = I->count() - 1; i >= 0; --i)
                            pDestList->removeAll(I->at(i));
                        I = MountPoints.erase(I);
                    }
                    else {
                        ++I;
                    }
                    break;
                }
           }
        } while (true);
    }
}

//------------------------------------------------------------------------------

void TTaskManager::initThreads(const QStringList& DestList)
{
    Q_ASSERT(!m_pReader->isRunning());

    // Проверка некритичных условий.
    if (DestList.isEmpty()) {
        qWarning("TTaskManager::initThreads. Destinations list is empty.");
        return;
    }

    // Поток чтения.
    m_pReader->setTask(m_pCurrentTask);
    m_pReader->setBuffer(m_pBuffer);

    // Потоки записи.
    m_DestsCounter.init(DestList.count());

    // По одному потоку на назначение.
    for (int i = 0; i < DestList.count(); ++i)
    {
        TWriter* pWriter = new TWriter();
        m_WritersList.append(pWriter);

        // Инициализация.
        QString Dest = DestList[i];
        pWriter->setDest(Dest);
        pWriter->setBuffer(m_pBuffer);
        pWriter->setTask(m_pCurrentTask);
        pWriter->setTaskStatus(m_pTaskStatus);

        // Регистрация.
        m_pBuffer->registerConsumer(pWriter);

        connect(pWriter, SIGNAL(error(TErrorData*)), SLOT(errorReceiver(TErrorData*)));
    }
}

//------------------------------------------------------------------------------
//! Обработка текущего задания.

void TTaskManager::processCurrentTask()
{
    Q_ASSERT(m_pBuffer == NULL);
    Q_ASSERT(!m_pCurrentTask.isNull());

    m_LastActions.clear();

    QStringList DestList = m_pCurrentTask->DestList;

    // Вычисляем размер задания.
    if (m_pCurrentTask->TaskSettings.TotalCalc)
    {
        Q_ASSERT(m_pSizeCalc != NULL);
        Q_ASSERT(!m_pSizeCalc->isRunning());

        // Сигнал beginCalculate сгенерирует m_pSizeCalc.

        m_pSizeCalc->setTask(m_pCurrentTask);
        m_pSizeCalc->start();
        m_pSizeCalc->wait();

        // Сигнал endCalculate сгенерирует m_pSizeCalc.

        if (m_pTaskStatus != NULL)
            m_pTaskStatus->setTaskSize(m_pSizeCalc->taskSize());

        checkFreeSpace(&DestList);
        if (DestList.isEmpty())
            return;

    }
    else {
        if (m_pTaskStatus != NULL)
            m_pTaskStatus->clearTaskSize();
    }

    if (m_Cancel) return;

    // Создание кольцевого буфера.
    int CellCount, CellSize;
    if (m_pCurrentTask->TaskSettings.BufferSizeAutoselect)
        bufferSizeAutodetect(&CellCount, &CellSize);
    else {
        CellCount = m_pCurrentTask->TaskSettings.RAMCellCount;
        CellSize  = m_pCurrentTask->TaskSettings.RAMCellSize;

        // CellSize должно быть кратно размеру физического сектора диска.
        if (CellSize % IO_BLOCK_SIZE != 0)
            CellSize = (CellSize / IO_BLOCK_SIZE + 1) * IO_BLOCK_SIZE;
    }
    m_pBuffer = new TCircularBuffer(CellCount,
                                    CellSize,
                                    m_pCurrentTask->TaskSettings.LockMemory);
    if (!m_pBuffer->isAllocated()) {
        emit outOfMemory();
    }
    else {
        // Память успешно распределена.

        initThreads(DestList);
        Q_ASSERT(m_WritersList.count() > 0);

        m_pTaskStatus->begin();

        // Этот сигнал должен быть сгенерирован ПОСЛЕ очистки счётчиков.
        emit beginCopy();

        // Запуск потоков.
        m_pReader->start();
        for (int i = 0; i < m_WritersList.count(); ++i)
            m_WritersList[i]->start();

        // Ожидание завершения потоков.
        m_pReader->wait();
        for (int i = 0; i < m_WritersList.count(); ++i)
            m_WritersList[i]->wait();

        m_pTaskStatus->end();

        // Этот сигнал должен быть сгененрирован ПОСЛЕ финализации счётчиков.
        emit endCopy();

        // Обнуление переменных и очистка памяти.
        m_pReader->setBuffer(NULL);
        m_pReader->setTask(TSharedConstTask(0));
        m_pTaskStatus->unregisterAllWriters();

        // Разрушение потоков записи.
        for (int i = m_WritersList.count() - 1; i >= 0; --i)
            delete m_WritersList[i];
        m_WritersList.clear();
    }

    delete m_pBuffer;
    m_pBuffer = NULL;
}

//------------------------------------------------------------------------------
//! Обработка всех заданий.

void TTaskManager::processAllTasks()
{
    // Сигнал о начале обработки заданий.
    emit begin();

    do {
        if (m_TaskList.isEmpty())
            break;
        m_pCurrentTask = m_TaskList.takeFirst();
        Q_ASSERT(!m_pCurrentTask.isNull());

        // Сигнал о начале обработки задания.
        emit beginTask(m_pCurrentTask);

        // Обрабатываем следующее задание.
        processCurrentTask();
        m_Cancel = false;

        // Сигнал о конце обработки задания.
        emit endTask(m_pCurrentTask);
        m_TaskList.removeAll(m_pCurrentTask);
        m_pCurrentTask.clear();

    } while (true);

    // Сигнал о завершении обработки заданий.
    emit end();
}

//------------------------------------------------------------------------------
//! Определение необходимости вывода запроса пользователю.
/*!

 */

bool TTaskManager::userPromptRequired(TErrorData* pErrorData)
{
    if (m_Cancel)
    {
        pErrorData->Action = eaCancelCurrentTask;
        return false;
    }

    TErrorAction Action = m_LastActions.lastAction();
    if (Action == eaCancelCurrentTask || Action == eaCancelAllTasks)
    {
        pErrorData->Action = Action;
        return false;
    }

    Action = m_LastActions.lastAction(pErrorData->ErrorCode);
    if (Action == eaOverwriteAll || Action == eaSkipAll ||
        Action == eaIgnoreAll)
    {
        pErrorData->Action = Action;
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------

void TTaskManager::processNextError(TErrorData *pErrorData)
{
    /* Возможна ситуация, когда в очередь добавлено несколько сообщений об
       ошибках и после обработки одного из них дла других вывод диалога уже
       не потребуется. Поэтому вначале проверяем необходимость вывода. */
    if (userPromptRequired(pErrorData))
    {
        if (receivers(SIGNAL(error(TErrorData*))) > 0)
        {
            pErrorData->TasksCount = m_TaskList.count();
            // Считаем число назначений только в том случае, когда
            // оно не установлено при генерации ошибки.
            if (pErrorData->DestsCount < 0) {
                pErrorData->DestsCount = 0;
                for (int i = m_WritersList.count() - 1; i >= 0; --i)
                    pErrorData->DestsCount += m_WritersList[i]->destsCount();
            }
            if (m_pTaskStatus != NULL)
                m_pTaskStatus->pause();
            pause();
            // Флаг должен сбросить метод errorProcessed.
            m_UserPromptInProcess = true;
            emit error(pErrorData);
            return;
        }
        else {
            qWarning("TTaskManager::processNextError. "
                     "No receivers for error signal.");
            pErrorData->Action = eaIgnore;
        }
    }
    finishProcessError(pErrorData);
}

//------------------------------------------------------------------------------

void TTaskManager::processNextError()
{
    // Защита от вывода одновременно двух диалоговых окон.
    if (m_UserPromptInProcess || (m_LockProcessErrors != 0))
        return;

    while (!m_ErrorsQueue.isEmpty())
        processNextError(m_ErrorsQueue.head());
}

//------------------------------------------------------------------------------

void TTaskManager::skipDest(TWriter* pWriter)
{
    Q_ASSERT(pWriter != NULL);
    Q_ASSERT(m_WritersList.contains(pWriter));

    if (pWriter->fileRelName() == m_pReader->fileRelName())
    {
        m_DestsCounter.skip(m_pReader->fileRelName());
        Q_ASSERT(m_DestsCounter.skipped() <= m_DestsCounter.dests());
        if (m_DestsCounter.skipped() == m_DestsCounter.dests())
            m_pReader->skip();
    }
}

//------------------------------------------------------------------------------

void TTaskManager::cancelDest()
{
    if (m_WritersList.isEmpty())
        return;

    // Поток разрегистрируется самостоятельно.

    m_DestsCounter.cancelDest();
    Q_ASSERT(m_DestsCounter.dests() >= 0);

    // Если потоку записи выдана команда Skip(All), он не может вызвать на
    // этом файле ошибку, поэтому возникновение cancelDest подразумевает
    // отсутствие флага Skip.
    Q_ASSERT(m_DestsCounter.skipped() <= m_DestsCounter.dests());

    if (m_DestsCounter.dests() == 0) {
        cancelCurrentTask();
    }
    else {
        if (m_DestsCounter.skipped() == m_DestsCounter.dests())
            m_pReader->skip();
    }
}

//------------------------------------------------------------------------------

void TTaskManager::finishProcessError(TErrorData *pErrorData)
{
    Q_ASSERT(pErrorData != NULL);

    switch (pErrorData->Action) {
        case eaSkip :
        case eaSkipAll :
        {
            TWriter* pWriter = qobject_cast<TWriter*>(pErrorData->pSender);
            if (pWriter != NULL)
                skipDest(pWriter);

            // Если источник ошибки - TReader, ничего не делаем.

            break;
        }
        case eaCancelDest :
        {
            if (pErrorData->pSender != this)
            {
                Q_ASSERT(m_WritersList.count() > 1);
                Q_ASSERT(QString::compare(pErrorData->pSender->metaObject()->className(), "TWriter") == 0);

                cancelDest();
            }
            break;
        }
        case eaCancelCurrentTask :
            cancelCurrentTask();
            break;
        case eaCancelAllTasks :
            cancelAllTasks();
            break;
        default:
            ;
    }

    if (pErrorData->pLocker != NULL)
        pErrorData->pLocker->release();

    // Поиск ошибки в очереди и её удаление.
    TErrorsQueue::iterator I = m_ErrorsQueue.begin();

    while (I != m_ErrorsQueue.end())
    {
        if (*I == pErrorData) {
            /*I =*/ m_ErrorsQueue.erase(I);
            break;
        }
        else
            ++I;
    }
}

//------------------------------------------------------------------------------
//! Основная функция потока.

void TTaskManager::run()
{
    // Проверка некритичных условий.
    if (receivers(SIGNAL(error(TErrorData*))) <= 0)
        qWarning("TaskManager running without error handler.");

    processAllTasks();
}

//------------------------------------------------------------------------------
//! Конструктор.

TTaskManager::TTaskManager(QObject* Parent)
    : QThread(Parent),
      m_Cancel(false),
      m_UserPromptInProcess(false),
      m_LockProcessErrors(0),
      m_pReader(new TReader()),
      m_pSizeCalc(new TSizeCalculator()),
      m_pTaskStatus(new TTaskStatus()),
      m_pBuffer(NULL)
{
    m_pReader->setTaskStatus(m_pTaskStatus);

    connect(m_pSizeCalc, SIGNAL(begin()),            SIGNAL(beginCalculate()));
    connect(m_pSizeCalc, SIGNAL(end(TTaskSize)),     SIGNAL(endCalculate(TTaskSize)));
    connect(m_pReader,   SIGNAL(error(TErrorData*)), SLOT(errorReceiver(TErrorData*)));
}

//------------------------------------------------------------------------------
//! Деструктор.

TTaskManager::~TTaskManager()
{
    Q_ASSERT(!m_pReader->isRunning());
    delete m_pReader;

    Q_ASSERT(!m_pSizeCalc->isRunning());
    delete m_pSizeCalc;

    delete m_pTaskStatus;
}

//------------------------------------------------------------------------------
//! Добавление одного задания.

void TTaskManager::addTask(TSharedConstTask Task)
{
    m_TaskList.append(Task);
}

//------------------------------------------------------------------------------
//! Добавление списка заданий.

void TTaskManager::addTasks(const TTaskList& TaskList)
{
    m_TaskList.append(TaskList);
}

//------------------------------------------------------------------------------
//! Удаление одного задания.

void TTaskManager::deleteTask(TSharedConstTask Task)
{
    if (Task == m_pCurrentTask)
        cancelCurrentTask();
    else
        m_TaskList.removeAll(Task);
}

//------------------------------------------------------------------------------
//! Удаление списка заданий.

void TTaskManager::deleteTasks(const TTaskList& TaskList)
{
    for (int i = 0; i < TaskList.count(); ++i)
        if (TaskList[i] != m_pCurrentTask)
            m_TaskList.removeAll(TaskList[i]);

    if (TaskList.contains(m_pCurrentTask))
        cancelCurrentTask();
}

//------------------------------------------------------------------------------

void TTaskManager::moveTask(int Index, int Delta)
{
    if (0 <= Index && Index < m_TaskList.count()) {
        int newIndex = Index + Delta;
        if (newIndex < 0) newIndex = 0;
        if (newIndex >= m_TaskList.count()) newIndex = m_TaskList.count();
        if (newIndex != Index) {
            TSharedConstTask Task = m_TaskList.takeAt(Index);
            m_TaskList.insert(newIndex, Task);
        }
    }
    else {
        qWarning("TTaskManager::moveTask. Index is out of range "
                 "(index = %i, count = %i).", Index, m_TaskList.count());
    }
}

//------------------------------------------------------------------------------
//! Текущее задание.

TSharedConstTask TTaskManager::currentTask() const
{
    return m_pCurrentTask;
}

//------------------------------------------------------------------------------

TTaskSize TTaskManager::taskSize() const
{
    Q_ASSERT(m_pSizeCalc != NULL);

    return m_pSizeCalc->taskSize();
}

//------------------------------------------------------------------------------
//! Приостановка выполнения.
/*!
   \sa resume, isPaused

   \return True, если операция успешно завершена и false, если выполнение уже
     было приостановлено.
 */

void TTaskManager::pause()
{
    Q_ASSERT(m_pSizeCalc != NULL);
    Q_ASSERT(m_pReader != NULL);
    Q_ASSERT(m_pTaskStatus != NULL);

    if (m_Paused.fetchAndAddOrdered(1) == 0) {
        m_pSizeCalc->pause();
        m_pReader->pause();
        for (int i = m_WritersList.count() - 1; i >= 0; --i)
            m_WritersList[i]->pause();
        m_pTaskStatus->pause();
    }
}

//------------------------------------------------------------------------------
//! Возобновление выполнения.
/*!
   \sa pause, isPaused
 */

void TTaskManager::resume()
{
    Q_ASSERT(m_pSizeCalc != NULL);
    Q_ASSERT(m_pReader != NULL);
    Q_ASSERT(m_pTaskStatus != NULL);

    if (m_Paused <= 0) {
        qWarning("TTaskManager::resume. pause() and resume() calls mismatch.");
        return;
    }

    if (!m_Paused.deref()) {
        m_pTaskStatus->resume();
        m_pSizeCalc->resume();
        m_pReader->resume();
        for (int i = m_WritersList.count() - 1; i >= 0; --i)
            m_WritersList[i]->resume();
    }
}

//------------------------------------------------------------------------------
//! Флаг приостановки выполнения.
/*!
   \sa pause, resume
 */

bool TTaskManager::isPaused() const
{
    return m_Paused != 0;
}

//------------------------------------------------------------------------------

bool TTaskManager::isCalculating() const
{
    Q_ASSERT(m_pSizeCalc != NULL);

    return m_pSizeCalc->isRunning();
}

//------------------------------------------------------------------------------
//! Отмена выполнения текущего задания.
/*!
   \sa cancelAllTasks
 */

void TTaskManager::cancelCurrentTask()
{
    m_Cancel = true;

    // Калькулятор.
    if (m_pSizeCalc != NULL)
        m_pSizeCalc->cancel();

    // Поток чтения.
    if (m_pReader != NULL)
        m_pReader->cancel();

    // Потоки записи.
    for (int i = m_WritersList.count() - 1; i >= 0; --i)
        if (m_WritersList[i] != NULL)
            m_WritersList[i]->cancel();

    // Принудительная разблокировка.
    if (m_pBuffer != NULL)
        m_pBuffer->unlock();
}

//------------------------------------------------------------------------------
//! Отмена выполнения всех заданий.
/*!
   \sa cancelCurrentTask
 */

void TTaskManager::cancelAllTasks()
{
    TTaskList TaskList = m_TaskList;
    TaskList.removeAll(m_pCurrentTask);
    emit cancelTasks(TaskList);
    m_TaskList.clear();
    cancelCurrentTask();
}

//------------------------------------------------------------------------------
//! Блокировка обработчика ошибок.

void TTaskManager::lockProcessErrors()
{
    /*if (!m_LockProcessErrors) {
        m_LockProcessErrors = true;
    }
    else {
        qWarning("TTaskManager::lockProcessErrors. Already locked.");
    }*/
    if (!m_LockProcessErrors.testAndSetRelaxed(0, 1))
        qWarning("TTaskManager::lockProcessErrors. Already locked.");
}

//------------------------------------------------------------------------------
//! Разблокировка обработчика ошибок.

void TTaskManager::unlockProcessErrors()
{
    /*if (m_LockProcessErrors) {
        m_LockProcessErrors = false;
        processNextError();
    }
    else {
        qWarning("TTaskManager::unlockProcessErrors. Already unlocked.");
    }*/
    if (!m_LockProcessErrors.testAndSetRelaxed(1, 0))
        qWarning("TTaskManager::unlockProcessErrors. Already unlocked.");
}

//------------------------------------------------------------------------------
//! Обработчик ошибок потоков чтения и записи.

void TTaskManager::errorReceiver(TErrorData *pErrorData)
{
    Q_ASSERT(pErrorData != NULL);

    /* Если вывод сообщения пользователю не требуется, сообщение в очередь не
       помещаем. Метод userPromptRequired установит в структуре pErrorData
       выбранное действие, после чего освобождаем семафор и завершаем
       обработку.
       Если нужен запрос действия у пользователя, помещаем сообщение в очередь
       и вызываем метод обработки. */
    if (userPromptRequired(pErrorData)) {
        m_ErrorsQueue.enqueue(pErrorData);
        processNextError();
    }
    else {
        finishProcessError(pErrorData);
    }
}

//------------------------------------------------------------------------------

void TTaskManager::errorProcessed(TErrorData* pErrorData)
{
    // Вызывать напрямую нельзя, только через сигналы.
    Q_ASSERT(sender() != NULL);

    m_LastActions.addAction(pErrorData->ErrorCode, pErrorData->Action);
    if (m_pTaskStatus != NULL) {
        if (m_pTaskStatus->isStarted() && !m_pTaskStatus->isPaused()) {
            qWarning("TTaskManager::errorProcessed. "
                     "TaskStatus started, but is not paused.");
        }
        m_pTaskStatus->resume();
    }

    if (isPaused())
        resume();
    else
        qWarning("TTaskManager::errorProcessed. Threads is not paused.");

    m_UserPromptInProcess = false;

    finishProcessError(pErrorData);
    processNextError();
}

//------------------------------------------------------------------------------
