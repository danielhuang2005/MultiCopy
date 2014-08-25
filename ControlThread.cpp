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

#include "ControlThread.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QLibrary>

#if defined(Q_OS_WIN)
    #define _WIN32_WINNT 0x500 // Windows 2000 and above.
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <utime.h>
    #include <errno.h>
#endif

#include "RWCalculator.hpp"
#include "CircularBuffer.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "ProgressForm.hpp"
#include "CommonFn.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//             В с п о м о г а т е л ь н ы е   ф у н к ц и и .
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Копирование даты и времени файлов и каталогов.
/*!
 * \param SrcName Файл/каталог, дата и время которого копируются.
 * \param DestName Файл/каталог, дата и время которого устанавливаются.
 *
 * \return true если операция прошла успешно и false в противном случае.
 *
 * \remarks В Windows поддерживаются имена длиной до 32767 символов.
 */
/*
bool CopyFileTime(const QString& SrcName, const QString& DestName)
{
    bool Result = false;
#if defined(Q_OS_WIN)
    LPWSTR wSrcName  = StrToLPWSTR(PathToLongWinPath(SrcName));
    LPWSTR wDestName = StrToLPWSTR(PathToLongWinPath(DestName));
    FILETIME CreationTime, LastWriteTime;
    HANDLE hSrc = CreateFileW(wSrcName,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_BACKUP_SEMANTICS,
                              NULL);
    if (hSrc != INVALID_HANDLE_VALUE)
    {
        if (GetFileTime(hSrc, &CreationTime, NULL, &LastWriteTime))
        {
            HANDLE hDest = CreateFileW(wDestName,
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_FLAG_BACKUP_SEMANTICS,
                                       NULL);
            if (hDest != INVALID_HANDLE_VALUE)
            {
                Result = SetFileTime(hDest, &CreationTime, NULL, &LastWriteTime);
                if (!Result)
                {
                    qWarning("Cannot set date/time for file %ls: %s",
                             wDestName, qPrintable(GetSystemErrorString()));
                }
                CloseHandle(hDest);
            }
            else {
                qWarning("Cannot open file %ls for writing: %s",
                         wDestName, qPrintable(GetSystemErrorString()));

            }
        }
        else {
            qWarning("Cannot get date/time of file %ls: %s",
                     wSrcName, qPrintable(GetSystemErrorString()));
        }
        CloseHandle(hSrc);
    }
    else {
        qWarning("Cannot open file %ls for reading: %s",
                 wSrcName, qPrintable(GetSystemErrorString()));
    }
    delete wSrcName;
    delete wDestName;
#else
    QByteArray Src = QDir::toNativeSeparators(SrcName).toLocal8Bit();
    struct stat Stat;
    if (stat(Src.data(), &Stat) == 0)
    {
        utimbuf Utimbuf;
        Utimbuf.actime = 0; //Stat.st_atime;
        Utimbuf.modtime = Stat.st_mtime;

        QByteArray Dest = QDir::toNativeSeparators(DestName).toLocal8Bit();
        Result = utime(Dest.data(), &Utimbuf) == 0;
        if (!Result)
        {
            qWarning("Cannot set time for file %s: %s",
                     Dest.data(), qPrintable(GetSystemErrorString()));
        }
        Result = chmod(Dest.data(), Stat.st_mode) == 0;
        if (!Result) {
            qWarning("Cannot set permissions for file %s: %s",
                     Dest.data(), qPrintable(GetSystemErrorString()));
        }
    }
    else {
        qWarning("Cannot get statistics for file %s: %s",
                 Src.data(), qPrintable(GetSystemErrorString()));
    }
#endif
    return Result;
}
*/
//------------------------------------------------------------------------------

bool AutodetectRAMBuffer(int* pCellSize, int* pCellsCount)
{
    qint64 AvailPhys;

#ifdef Q_OS_WIN
    MEMORYSTATUSEX MemStatus;
    MemStatus.dwLength = sizeof(MemStatus);
    if (GlobalMemoryStatusEx(&MemStatus)) {
        AvailPhys = MemStatus.ullAvailPhys;
    }
    else {
        qDebug() << "GlobalMemoryStatusEx error.\n" << GetSystemErrorString();

        // Значения по умолчанию (32Mb).
        *pCellSize = 4*1024*1024;  // 4Mb
        *pCellSize = 8;
        return false;
    }
#else
    //#error "Function is not realized!"
    long AvailPhysPages = sysconf(_SC_AVPHYS_PAGES),
         PageSize = sysconf(_SC_PAGE_SIZE);
    if ((AvailPhysPages > 0) && (PageSize > 0))  {
        AvailPhys = AvailPhysPages * PageSize;
    }
    else {
        qDebug() << "Error detecting RAM Size.";

        // Значения по умолчанию (32Mb).
        *pCellSize = 4*1024*1024;  // 4Mb
        *pCellSize = 8;
        return false;
    }
#endif

    *pCellSize = 4*1024*1024;  // 4Mb
    if (AvailPhys <= 2 * 2 * 4*1024*1024) { // 2 x 2 x 4Mb = 16Mb
        *pCellsCount = 2;
    }
    else {
        if (AvailPhys >= 2 * 32 * 4*1024*1024) { // 2 x 32 x 4Mb = 256Mb
            *pCellsCount = 32;
        }
        else {
            *pCellsCount = AvailPhys / *pCellSize / 2;
        }
    }
    qDebug() << "AutodetectRAMBuffer: " << *pCellsCount << "x" << *pCellSize;
    return true;
}





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                      T C o n t r o l T h r e a d
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TControlThread::TControlThread(TProgressFormPrivate *Parent)
    : QThread(Parent),
      m_pBuffer(NULL),
      m_pReader(NULL),
      m_ErrorHandler(Parent),
      m_Cancel(cNoCancel),
      m_Paused(false),
      m_Status(Finished)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TControlThread::~TControlThread()
{
}

//------------------------------------------------------------------------------
//! Создание потоков.
/*!
 * \param WritersCount Число создаваемых потоков записи.
 */

void TControlThread::createThreads(int WritersCount)
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    m_pReader = new TFileReader(this);
    for (int i = WritersCount; i > 0; --i)
    {
        TFileWriter* pWriter = new TFileWriter(this);
        m_Writers.append(pWriter);
    }
}

//------------------------------------------------------------------------------
//! Регистрация потоков.
/*!
 * Регистрация потоков в калькуляторе и синхронизаторе.
 */

void TControlThread::registerThreads()
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    m_pBuffer->unregisterAll();
    m_RWCalculator.unregisterAll();

    for (int i = m_Writers.count() - 1; i >= 0; --i)
    {
        register TFileWriter* pFW = m_Writers[i].pWriter;

        Q_ASSERT(pFW != NULL);
        Q_ASSERT(!pFW->isRunning());

        m_pBuffer->registerConsumer(pFW);
        m_RWCalculator.registerWriter(pFW);
    }
}

//------------------------------------------------------------------------------
//! Разрушение всех потоков.

void TControlThread::destroyAllThreads()
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    m_RWCalculator.unregisterAll();
    m_pBuffer->unregisterAll();
    for (int i = m_Writers.count() - 1; i >= 0; --i)
        delete m_Writers[i].pWriter;
    m_Writers.clear();
    delete m_pReader;  m_pReader = NULL;
}

//------------------------------------------------------------------------------
//! Разрушение потоков, ожидающих удаления.

void TControlThread::destroyPendingThreads()
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    int i = 0;
    while (i < m_Writers.count())
    {
        if (m_Writers[i].DestroyPending)
        {
            register TFileWriter* pFW = m_Writers[i].pWriter;

            Q_ASSERT(!pFW->isRunning());
            Q_ASSERT(!m_pBuffer->isConsumerRegistered(pFW));
            Q_ASSERT(!m_RWCalculator.isWriterRegistered(pFW));

            m_Writers.remove(i);
            delete pFW;

            // Удаляем также соответствующее назначение.
            m_CurrentJob.Dests.removeAt(i);
        }
        else ++i;
    }
}

//------------------------------------------------------------------------------
//! Ставит поток в очередь на удаление.

void TControlThread::destroyThreadLater(QThread* pThread)
{
    TFileWriter* pFW = dynamic_cast<TFileWriter*>(pThread);
    Q_ASSERT(pFW != NULL);
    pFW->cancel();
    unregisterThread(pFW);

    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    for (int i = m_Writers.count() - 1; i >= 0; --i)
    {
        if (pFW == m_Writers[i].pWriter)
        {
            m_Writers[i].DestroyPending = true;
            break;
        }
    }

    // Защита от блокировок - если всех потребителей поместили в очередь на
    // удаление, нужно прекратить обработку текущего задания.
    bool _Cancel = true;
    for (int i = m_Writers.count() - 1; i >= 0; --i)
    {
        if (!m_Writers[i].DestroyPending)
        {
            _Cancel = false;
            break;
        }
    }
    if (_Cancel)
        cancelCurrentJob();
}

//------------------------------------------------------------------------------
//! Отмена регистрации потока записи.
/*!
 * Отмена регистрации потока в калькуляторе и синхронизаторе.
 */

void TControlThread::unregisterThread(TFileWriter* pFileWriter)
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    m_pBuffer->unregisterConsumer(pFileWriter);
    m_RWCalculator.unregisterWriter(pFileWriter);

    // Защита от блокировок - если всех потребителей разрегистрировали,
    // нужно прекратить обработку текущего файла.
    if (m_pBuffer->consumersCount() == 0)
    {
        m_pReader->cancel();
        m_pBuffer->unlock();
    }
}

//------------------------------------------------------------------------------
//! Копирование файла.
/*!
 * \param FileName Полное имя копируемого файла.
 * \param RelativePath Путь, добавляемый к путям назначения.
 */

void TControlThread::copyFile(const QString& FileName,
                              const QString& RelativePath)
{
    if (m_pBuffer->isUnlocked())
    {
        // Если предыдущее копирование прервано методом unlock,
        // то реинициализируем кольцевой буфер.
        m_pBuffer->reset();
    }
    m_RWCalculator.newFile();
    registerThreads();

    if (m_Cancel != cNoCancel)
        return;

    Q_ASSERT(!m_pReader->isRunning());
    Q_ASSERT(m_CurrentJob.Dests.count() == m_Writers.count());

    m_FileInfo.setFile(FileName);
    QString Name = m_FileInfo.fileName();

    // Генерируем сигнал о начале копирования файла.
    emit beginCopyFile(FileName, m_FileInfo.size());


    // Сохраняем статистику файла.
    // TODO : Убрать в TFileReader (?).
    TFileStat FileStat;
    if (m_CurrentJob.SettingsData.CopyDateTime)
        FileStat.Options |= fsoTime;
    if (m_CurrentJob.SettingsData.CopyAttr)
        FileStat.Options |= fsoAttr;
    GetFileStat(FileName, &FileStat);

    // Пытаемся открыть файлы.
    bool ReadyWrite = false;
    bool ReadyRead = m_pReader->openFile(FileName,
                                         !m_CurrentJob.SettingsData.NotUseCache);
    if (ReadyRead)
    {
        // Файл-источник успешно открыт. Открываем файлы-назначения.
        for (int i = 0; i < m_CurrentJob.Dests.count(); ++i)
        {
            Q_ASSERT(!m_Writers[i].pWriter->isRunning());

            // Строим путь к файлу...
            QString Dest = m_CurrentJob.Dests.at(i);
            AddWithSeparator(&Dest, RelativePath);

            // ...и добавляем к нему имя файла.
            AddWithSeparator(&Dest, Name);
            m_Writers[i].pWriter->openFile(Dest,
                                           m_FileInfo.size(),
                                           !m_CurrentJob.SettingsData.NotUseCache);
            ReadyWrite |= m_Writers[i].pWriter->readyToRun();
        }
    }

    // Здесь может быть отменено копирование в некоторое назначение. Проверяем.
    destroyPendingThreads();

    if (ReadyRead && ReadyWrite)
    {
        // Запуск потоков на выполнение.
        m_pReader->start();
        for (int i = 0; i < m_Writers.count(); ++i)
        {
            Q_ASSERT(!m_Writers[i].pWriter->isRunning());
            m_Writers[i].pWriter->start();
        }

        // Ожидание завершения потоков.
        m_pReader->wait();
        for (int i = 0; i < m_Writers.count(); ++i)
        {
            m_Writers[i].pWriter->wait();
            if (!m_Writers[i].pWriter->isCancelled())
            {
                // Устанавливаем статистику файла.
                // TODO : Убрать в TFileWriter (?).
                SetFileStat(m_Writers[i].pWriter->fileName(), &FileStat);
            }
        }

        #ifndef _NO_CHECK_MD5
        QByteArray Reader_MD5 = m_pReader->m_MD5.result().toHex();
        qDebug() << "Reader_MD5 = " << Reader_MD5;
        for (int i = 0; i < m_Writers.count(); ++i)
        {
            QByteArray Writer_MD5 = m_Writers[i].pWriter->m_MD5.result().toHex();
            qDebug() << "Writer_MD5 (" << m_Writers[i].pWriter->fileName()
                     << ") = " << Writer_MD5;
            Q_ASSERT(Reader_MD5 == Writer_MD5);
        }
        #endif

        // Разрушаем отменённые потоки. Отмена регистрации должна быть выполнена
        // в обработчике ошибок!
        destroyPendingThreads();
    }
    else {
        m_pReader->closeFile();
        for (int i = m_Writers.count() - 1; i >= 0; --i)
            m_Writers[i].pWriter->closeFile();
    }

    // Генерируем сигнал об окончании копирования файла.
    emit endCopyFile();
}

//------------------------------------------------------------------------------
//! Копирование содержимого каталога.
/*!
 * Метод рекурсивно копирует содержимое каталога.
 *
 * \param DirName Путь к копируемому каталогу.
 * \param RelativePath Путь, добавляемый к путям назначения.
 * \param SubDirsDepth Глубина копирования подкаталогов (-1 - неограниченно).
 */

void TControlThread::copyFolderEntry(const QString& DirName,
                                     const QString& RelativePath,
                                     int SubDirsDepth)
{
    if (m_Cancel != cNoCancel)
        return;

    // Сохраняем статистику каталога.
    TFileStat DirStat;
    if (m_CurrentJob.SettingsData.CopyDateTime)
        DirStat.Options |= fsoTime;
    if (m_CurrentJob.SettingsData.CopyAttr)
        DirStat.Options |= fsoAttr;
    GetFileStat(DirName, &DirStat);

    // Рекурсивный обход подкаталогов.
    bool WithSubDirs = SubDirsDepth != 0;
    if (SubDirsDepth > 0)
        --SubDirsDepth;
    QDirIterator DirI(DirName, QDir::NoDotAndDotDot |
                               QDir::Files |
                               QDir::Dirs |
                               QDir::Hidden |
                               QDir::System);
    QFileInfo FileInfo;
    while (DirI.hasNext()) {
        if (m_Cancel)
            return;
        DirI.next();
        FileInfo = DirI.fileInfo();
        if (FileInfo.isFile())
            copyFile(AddWithSeparator(DirName, DirI.fileName()), RelativePath);
        else if (WithSubDirs)
            copyFolderEntry(AddWithSeparator(DirName, DirI.fileName()),
                            AddWithSeparator(RelativePath, DirI.fileName()),
                            SubDirsDepth);
    }

    if (m_CurrentJob.SettingsData.CopyDateTime && !RelativePath.isEmpty())
    {
        for (QStringList::const_iterator I = m_CurrentJob.Dests.constBegin();
             I != m_CurrentJob.Dests.constEnd(); ++I)
        {
            // Устанавливаем статистику каталога.
            SetFileStat(AddWithSeparator(*I, RelativePath), &DirStat);
        }
    }
}

//------------------------------------------------------------------------------
//! Копирование каталога.
/*!
 * Метод копирует содержимое каталога, создавая каталог в месте назначения.
 *
 * \param DirName Путь к копируемому каталогу.
 * \param SubDirsDepth Глубина копирования подкаталогов (-1 - неограниченно).
 */

void TControlThread::copyFolder(const QString& DirName,
                                     int SubDirsDepth)
{
    QDir Dir(DirName);
    copyFolderEntry(DirName, Dir.dirName(), SubDirsDepth);
}


//------------------------------------------------------------------------------
//! Стартовая точка потока.

void TControlThread::run()
{
    m_Status = Preparing;
    {
        QMutexLocker Locker(&m_ThreadsLocker);
        Q_UNUSED(Locker);

        m_Paused = false;
    }

    emit begin();

    m_Cancel = cNoCancel;

    do {
        if (m_Cancel == cAll)
        {
            QMutexLocker Locker(&m_JobsLocker);
            Q_UNUSED(Locker);

            m_Jobs.clear();
            break;
        }

        {
            QMutexLocker Locker(&m_JobsLocker);
            Q_UNUSED(Locker);
            if (m_Jobs.count() == 0) {
                break;
            }
            m_CurrentJob = m_Jobs.at(0);
            m_Jobs.pop_front();
        }

        // Очистка.
        m_RWCalculator.clear();
        m_ErrorHandler.clear();

        emit beginJob(&m_CurrentJob);

        // Подсчёт размера задания.
        calculate(&m_CurrentJob);

        // Проверка свободного места.
        checkFreeSpace(&m_CurrentJob);

        if (m_Cancel != cNoCancel)
            break;

        m_Status = Copying;
        emit beginCopy();

        // Распределение памяти.
        int CellSize, CellCount;
        if (m_CurrentJob.SettingsData.RAMAutodetect) {
            AutodetectRAMBuffer(&CellSize, &CellCount);
        }
        else {
            CellCount = m_CurrentJob.SettingsData.RAMCellCount;
            CellSize  = m_CurrentJob.SettingsData.RAMCellSize;

        }
        m_pBuffer = new TCircularBuffer(CellCount, CellSize,
                                        m_CurrentJob.SettingsData.LockMemory);
        if (!m_pBuffer->isAllocated())
        {
            emit outOfMemory();
            break;
        }

        createThreads(m_CurrentJob.Dests.count());
        m_RWCalculator.begin();

        QFileInfo FileInfo;
        for (QStringList::const_iterator I = m_CurrentJob.Srcs.constBegin();
             I != m_CurrentJob.Srcs.constEnd(); ++I)
        {
            FileInfo.setFile(*I);
            if (FileInfo.isDir())
            {
                if (m_CurrentJob.SettingsData.DirContentsOnly)
                {
                    copyFolderEntry(*I, QString(),
                                    m_CurrentJob.SettingsData.SubDirsDepth);
                }
                else {
                    copyFolder(*I, m_CurrentJob.SettingsData.SubDirsDepth);
                }
            }
            else {
                copyFile(*I);
            }

            if (m_Cancel != cNoCancel)
                break;
        }

        m_RWCalculator.end();

        // Очистка памяти.
        destroyAllThreads();
        delete m_pBuffer;  m_pBuffer = NULL;
        m_CurrentJob.clear();

        m_Status = Finished;
        emit endJob();

    } while(true);

    emit end();
}

//------------------------------------------------------------------------------
//! Добавление задания.

void TControlThread::addJob(const TJob& Job)
{
    QMutexLocker Locker(&m_JobsLocker);
    Q_UNUSED(Locker);

    m_Jobs.push_back(Job);
}

//------------------------------------------------------------------------------
//! Приостановка выполнения задания.
/*!
 * \remarks Повторный вызов функции ничего не делает.
 *
 * \remarks Вызов функции на неработающем потоке игнорируется.
 *
 * \sa resume, isPaused
 */

void TControlThread::pause()
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    if (isRunning() && !m_Paused)
    {
        m_Paused = true;

        m_RWCalculator.pause();

        if (m_pReader)
            m_pReader->pause();
        for (int i = m_Writers.count() - 1; i >= 0; --i)
            m_Writers[i].pWriter->pause();
    }
}

//------------------------------------------------------------------------------
//! Возобновление выполнения задания.
/*!
 * \remarks Повторный вызов функции ничего не делает.
 *
 * \remarks Вызов функции на неработающем потоке игнорируется.
 *
 * \sa pause, isPaused
 */

void TControlThread::resume()
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    if (isRunning() && m_Paused)
    {
        if (m_pReader)
            m_pReader->resume();
        for (int i = m_Writers.count() - 1; i >= 0; --i)
            m_Writers[i].pWriter->resume();

        m_RWCalculator.resume();

        m_Paused = false;
    }
}
//------------------------------------------------------------------------------
//!
/*!
 * Возвращает true, если выполнение заданий приостановлено и false в противном
 * случае.
 *
 * \remarks Поскольку потоки чтения и записи сами решают в какой точке им
 *   приостановить выполнение, то возвращённое значение нужно трактовать скорее
 *   как указание приостановиться, а не приостановку.
 *
 * \sa pause, resume
 */

bool TControlThread::isPaused() const
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    return m_Paused;
}

//------------------------------------------------------------------------------
//! Отмена обработки текущего файла.

void TControlThread::cancelCurrentFile()
{
    QMutexLocker Locker(&m_ThreadsLocker);
    Q_UNUSED(Locker);

    if (m_pReader)
        m_pReader->cancel();
    for (int i = m_Writers.count() - 1; i >= 0; --i)
        m_Writers[i].pWriter->cancel();
    if (m_pBuffer)
        m_pBuffer->unlock();
}

//------------------------------------------------------------------------------
//! Отмена обработки текущего задания.

void TControlThread::cancelCurrentJob()
{
    m_Cancel = cCurrent;
    cancelCurrentFile();
}

//------------------------------------------------------------------------------
//! Отмена обработки всех заданий.

void TControlThread::cancelAllJobs()
{
    m_Cancel = cAll;
    cancelCurrentJob();
}

//------------------------------------------------------------------------------
//! Обработчик ошибок.

TErrorHandler::Action TControlThread::error(TErrorHandler::ErrorData* pErrorData,
                                            QThread* pThread)
{
    QMutex Mutex;
    QMutexLocker Locker(&Mutex);
    Q_UNUSED(Locker);

    pErrorData->DestsCount = buffer()->consumersCount();
    TErrorHandler::Action A = m_ErrorHandler.error(*pErrorData);
    switch (A)
    {
        case TErrorHandler::aCancelAllJobs :
            cancelAllJobs();
            break;
        case TErrorHandler::aCancelCurrentJob :
            cancelCurrentJob();
            break;
        case TErrorHandler::aSkip :
        case TErrorHandler::aSkipAll :
        {
            TFileWriter* pFW = dynamic_cast<TFileWriter*>(pThread);
            if (pFW) {
                unregisterThread(pFW);
            }
            else {
                TFileReader* pFR = dynamic_cast<TFileReader*>(pThread);
                if (pFR)
                    cancelCurrentFile();
            }
            break;
        }
        case TErrorHandler::aCancelDest :
            destroyThreadLater(pThread);
            break;
        default :
            ;
    }

    return A;
}

//------------------------------------------------------------------------------

void TControlThread::readedBlock(qint64 Length)
{
    if (m_RWCalculator.readProgress(Length))
        emit readProgress(&m_RWCalculator);
}

//------------------------------------------------------------------------------

void TControlThread::writedBlock(const TFileWriter* pFileWriter, qint64 Length)
{
    if (m_RWCalculator.writeProgress(pFileWriter, Length))
        emit writeProgress(&m_RWCalculator);
}


//------------------------------------------------------------------------------
//! Рекурсивный подсчёт объёма текущего задания.
/*!
 * \param Name Имя объекта (файл или каталог).
 * \param SubDirDepth Глубина обработки подкаталогов (-1 - неограниченно).
 */

void TControlThread::calculate(const QString& Name, int SubDirsDepth)
{
    if (m_Cancel != cNoCancel)
        return;

    m_FileInfo.setFile(Name);
    if (m_FileInfo.isDir())
    {
        // Рекурсивный обход подкаталогов.
        bool WithSubDirs = SubDirsDepth != 0;
        if (WithSubDirs && SubDirsDepth > 0)
            --SubDirsDepth;

        QDirIterator DirI(Name, QDir::NoDotAndDotDot |
                                QDir::Files |
                                QDir::Dirs |
                                QDir::Hidden |
                                QDir::System);
        while (DirI.hasNext())
        {
            if (m_Cancel)
                break;
            DirI.next();
            if ((DirI.fileName() != ".") && (DirI.fileName() != ".."))
            {
                m_FileInfo = DirI.fileInfo();
                if (m_FileInfo.isFile())
                    m_JobSize.addFile(m_FileInfo.size());
                else if (WithSubDirs)
                         calculate(AddWithSeparator(Name, DirI.fileName()),
                                   SubDirsDepth);
            }
        }
    }
    else {
        m_JobSize.addFile(m_FileInfo.size());
    }
}

//------------------------------------------------------------------------------
//! Подсчёт объёма задания.

void TControlThread::calculate(const TJob* Job)
{
    if (Job->SettingsData.TotalCalc)
    {
        m_Status = Calculating;
        emit beginCalculate();
        m_JobSize.FilesCount = 0;
        m_JobSize.FilesSize = 0;
        for (QStringList::const_iterator I = Job->Srcs.constBegin();
             I != Job->Srcs.constEnd(); ++I)
        {
            if (m_Cancel) break;
            calculate(*I, Job->SettingsData.SubDirsDepth);
        }
        emit endCalculate(m_JobSize);
    }
}

//------------------------------------------------------------------------------
/*!
 *
 * \remarks Метод может удалить часть назначений из списка, если пользователь
 *   отказался от их использования.
 */

void TControlThread::checkFreeSpace(TJob* Job)
{
    if (!Job->SettingsData.CheckFreeSpace)
        return;

    QStringList ToDel;
    for (QStringList::const_iterator I = Job->Dests.constBegin();
         I != Job->Dests.constEnd(); ++I)
    {
        if (m_JobSize.FilesSize > DiskFreeSpace(*I))
        {
            do {
                TErrorHandler::ErrorData ED;
                ED.Code = TErrorHandler::eNoFreeSpace;
                ED.FileName = *I;
                ED.DestsCount = Job->Dests.count() - ToDel.count();

                TErrorHandler::Action A = m_ErrorHandler.error(ED);
                if (A == TErrorHandler::aCancelDest) {
                    ToDel.append(*I);
                    break;
                }
                else if (A == TErrorHandler::aIgnore) {
                    break;
                }

                switch (A) {
                    case TErrorHandler::aRetry :
                        continue;
                    case TErrorHandler::aIgnoreAll :
                        return;
                    case TErrorHandler::aCancelCurrentJob :
                        cancelCurrentJob();
                        return;
                    case TErrorHandler::aCancelAllJobs :
                        cancelAllJobs();
                        return;
                    default :
                        qWarning("Unknown action!");
                        return;
                }
            } while (true);
        }
    }

    for (int i = ToDel.count() - 1; i >= 0; --i)
        Job->Dests.removeAll(ToDel.at(i));
}

//------------------------------------------------------------------------------
