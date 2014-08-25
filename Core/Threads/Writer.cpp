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

#include "Writer.hpp"

#include <QDir>
#include <QSemaphore>

#include "Core/Common/CommonFn.hpp"
#include "Core/Buffer/CircularBuffer.hpp"
#include "Core/Errors/ErrorsAndActions.hpp"
#include "Core/Task/TaskStatus.hpp"

//------------------------------------------------------------------------------

enum TMkDirResult {
    mdrOK,            //!< Каталог создан успешно.
    mdrExists,        //!< Каталог уже существовал.
    mdrExistsNotDir,  //!< В пути существует объект, не являющийся каталогом.
    mdrCreateError    //!< Ошибка при создании каталога.
};


TMkDirResult MkPath(const QString& DirName, QString* pBadPath = NULL)
{
    QDir Dir;
    QString AbsName = Dir.absoluteFilePath(QDir::cleanPath(DirName));
    AbsName = QDir::toNativeSeparators(AbsName);

    Q_ASSERT(!AbsName.isEmpty());

    int CreatedDirs = 0;
    int SeparatorPos = -1;
    do {
        SeparatorPos = AbsName.indexOf(QDir::separator(), SeparatorPos + 1);

        if (SeparatorPos == 0) {
            /* Если путь начинается с сепаратора, то мы в UNIX, а там
               корневой каталог не существовать не может. А если даже его нет,
               то ошибка системы настолько критичная, что ошибкой приложения
               можно пренебречь. Проверку пропускаем. */
            continue;
        }

        QString Path;
        if (SeparatorPos >= 0)
            Path = AbsName.left(SeparatorPos);
        else
            Path = AbsName;

        Dir.setPath(Path);
        if (!Dir.exists()) {
            // Каталог не существует...
            if (Dir.exists(Path)) {
                // ... но какой-то объект с таким именем есть.
                if (pBadPath)
                    *pBadPath = Path;
                return mdrExistsNotDir;
            }
            else {
                // Нет ничего с таким именем. Пытаемся создать.
                if (!Dir.mkdir(Path)) {
                    // Создать не удалось.
                    if (pBadPath)
                        *pBadPath = Path;
                    return mdrCreateError;
                }
                else {
                    // Каталог создан успешно. Продолжаем обработку.
                    ++CreatedDirs;
                }
            }
        }
        else {
            // Каталог существует. Продолжаем обработку.
        }
    } while (SeparatorPos >= 0);

    return CreatedDirs > 0 ? mdrExists : mdrOK;
}

//------------------------------------------------------------------------------
//! Получение ячейки кольцевого буфера с ожиданием.

const TBufferCell* TWriter::acquireCell()
{
    Q_ASSERT(isRunning());
    Q_ASSERT(m_pBuffer != NULL);

    m_pBuffer->acquireConsumerSemaphore(this);
    return m_pBuffer->firstUsedBlock(this);
}

//------------------------------------------------------------------------------
//! Освобождение ячейки кольцевого буфера.

void TWriter::releaseCell()
{
    Q_ASSERT(isRunning());
    Q_ASSERT(m_pBuffer != NULL);

    m_pBuffer->releaseProducerSemaphore(this);
}

//------------------------------------------------------------------------------
//! Создание каталога с именем Name.

bool TWriter::createDir(TFileData* pFileData, const QString& DirName)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pFileData != NULL);
    Q_ASSERT(!DirName.isEmpty());

    QDir Dir(DirName);

    // Проверяем существование каталога.
    if (!Dir.exists())
    {
        // Каталог не существует.
        do {
            // Пытаемся создать каталог.
            QString BadDir;
            TMkDirResult MkDirResult = MkPath(DirName, &BadDir);
            if (MkDirResult != mdrOK && MkDirResult != mdrExists)
            {
                // Создать каталог не удалось.
                TErrorData ErrorData;
                ErrorData.ErrorCode = ecMakeDir;
                ErrorData.FileName  = BadDir;
                if (MkDirResult == mdrExistsNotDir) {
                    // Объект существует, но не является каталогом.
                    ErrorData.Message = tr("Object already exists, but it is not a folder.");
                }
                else {
                    // Строка сообщения пустая.
                }

                errorHandler(pFileData, &ErrorData);

                if (ErrorData.Action != eaRetry) {
                    // Если не была выбрана опция "повторить", то каталога не
                    // существует. Дальнейшая работа невозможна.
                    return false;
                }
                else {
                    // Повторяем попытку создания каталога.
                }
            }
            else {
                // Каталог успещно создан. Прерываем цикл.
                break;
            }
        } while (true);
    }
    return true;
}

//------------------------------------------------------------------------------
//! Создание нового файла-назначения.
/*!
   \arg pFileData Указатель на структуру информации о файле-назначении.
   \arg FileName Имя файла (путь относительно каталога назначения).
 */

bool TWriter::newFile(TFileData* pFileData, const QString &FileName)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pFileData != NULL);

    if (pFileData->File.isOpen())
        qWarning("TWriter::newFile. File already open.");

    pFileData->Skip = false;

    QFileInfo FileInfo(AddWithSeparator(pFileData->DestDir, FileName));
    if (!createDir(pFileData, FileInfo.absolutePath()))
        return false;

    TFastFile* pFile = &pFileData->File;
    pFile->setFileName(FileInfo.absoluteFilePath());


    // В данной точке каталог уже существует.
    // Проверка существования файла.
    if (!FileInfo.symLinkTarget().isEmpty() || FileInfo.exists())
    {
        // Файл уже существует.
        TErrorData ErrorData;
        ErrorData.ErrorCode = ecAlreadyExists;
        ErrorData.FileName  = FileInfo.absoluteFilePath();
        // Строка сообщения пустая.

        errorHandler(pFileData, &ErrorData);

        if ((ErrorData.Action != eaOverwrite)
            && (ErrorData.Action != eaOverwriteAll))
        {
            // Если не была затребована перезапись,
            // то файл должен быть пропущен.
            return false;
        }
        else {
            // Удаляем файл, а затем создаём заново.
            pFile->remove();
        }
    }


    // Открываем файл.
    do {
        bool NoUseCache = m_Task->TaskSettings.NoUseCache &&
                          (m_Size > m_Task->TaskSettings.NoUseCacheFor);
        if (!pFile->open(TFastFile::omWrite, NoUseCache, IO_BLOCK_SIZE))
        {
            // Открыть файл не удалось.

            TErrorData ErrorData;
            ErrorData.ErrorCode = ecCreateFile;
            ErrorData.Message   = pFile->errorString();
            ErrorData.FileName  = pFile->fileName();

            errorHandler(pFileData, &ErrorData);

            if (ErrorData.Action != eaRetry)
                break;
        }
        else {
            // Файл успешно открыт.
            pFileData->WritedBytes = 0;
            if (m_Size >= 0) {
                qint64 Size = m_Size / IO_BLOCK_SIZE;
                if (m_Size % IO_BLOCK_SIZE != 0)
                    ++Size;
                Size = Size * IO_BLOCK_SIZE;
                pFile->resize(Size);
            }
            break;
        }
    } while (true);

    return pFile->isOpen();
}

//------------------------------------------------------------------------------
//! Создание файлов-назначений.

void TWriter::newFiles(const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdNewFile);

    // Проверка некритичных условий.
    for (int i = 0; i < m_FileData.count(); ++i)
        if (m_FileData[i]->File.isOpen())
            qWarning("TWriter::newFile. File %s don't closed.",
                     qPrintable(m_FileData[i]->File.fileName()));

    /* Если здесь не вызвать этот метод, то возможно появление неполностью
       записанных файлов. */
    closeFiles();

    m_Size        = pCell->CommandData.Size;
    m_FileRelName = pCell->CommandData.ObjName;
    ++m_FileNumber;

    if (m_pTaskStatus)
        m_pTaskStatus->writerNewFile(this, m_FileRelName, m_Size);

    for (int i = 0; i < m_FileData.count(); ++i)
    {
        bool Created = newFile(m_FileData[i], m_FileRelName);
        if (m_pTaskStatus)
        {
            if (Created)
                m_pTaskStatus->writerNewFile(this, &m_FileData[i]->File);
            else
                m_pTaskStatus->writerSkipFile(this, &m_FileData[i]->File);
        }
    }

}

//------------------------------------------------------------------------------
//! Создание нового каталога.

void TWriter::newDir(TFileData* pFileData, const QString& DirName)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pFileData != NULL);
    Q_ASSERT(!DirName.isEmpty());

    QFileInfo FileInfo(AddWithSeparator(pFileData->DestDir, DirName));
    QString AbsPath = FileInfo.absoluteFilePath();
    if (!createDir(pFileData, AbsPath)) {
        /* TODO: При попытке создания файла вновь возникнет сообщение
                 о невозможности создания каталога. */
    }
}

//------------------------------------------------------------------------------
//! Создание всех новых каталогов.

void TWriter::newDir(const TBufferCell *pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdNewDir);

    if (pCell->CommandData.ObjName.isEmpty()) {
        qWarning("TWriter::newDir. Dir name is empty.");
        return;
    }

    for (int i = 0; i < m_FileData.count(); ++i)
        newDir(m_FileData[i], pCell->CommandData.ObjName);
}

//------------------------------------------------------------------------------
//! Запись блока в файл-назначение.

void TWriter::writeBlock(TFileData* pFileData, const TBufferCell *pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pFileData != NULL);
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdWriteBlock ||
             pCell->command() == cmdCloseFile);
    Q_ASSERT(pCell->usedSize() > 0);

    if (pFileData->Skip)
        return;

    if (isCancelled())
        return;

    TFastFile* pFile = &pFileData->File;
    int Size = pCell->usedSize();  // Объем данных для записи.

    if (!pFile->isOpen()) {
        if (m_pTaskStatus)
            m_pTaskStatus->writerSkip(this, pFile, Size);
        return;
    }

    // Коррекция размера для прямой записи.
    if (pFile->blockSize() > 0)
    {
        if (Size % pFile->blockSize() != 0)
        {
            /* Если запись идёт блоками, неполный блок может быть только при
               немедленном закрытии файла. */
            Q_ASSERT(pCell->command() == cmdCloseFile);
        }

        Size = (Size / pFile->blockSize() + (Size % pFile->blockSize() != 0 ? 1 : 0)) * pFile->blockSize();
        Q_ASSERT(Size <= pCell->size());
    }

    int Written = 0;                    // Записано байт за одну итерацию цикла.
                                        // (-1 - признак ошибки.)
    int WrittenTotal = 0;               // Всего записано байт (за все итерации).
    int Pos = pFile->pos();             // Текущая позиция указателя в файле.
    const char* pData = pCell->data();  // Данные для записи.
    Q_ASSERT(pData != NULL);

    do {
        if (Written < 0) {
            /* Если Written < 0, значит, при записи блока произошла ошибка
               и был затребован повтор операции. */
            if (pFile->seek(Pos)) {
                // Если удалось спозиционироваться, сбрасываем флаг ошибки.
                Written = 0;
            }
            else {
                // Если спозиционироваться не удалось, Written == -1.
            }
        }

        pausePoint();
        if (isCancelled())
            return;

        if (Written >= 0)
            Written = pFile->write(pData, Size);

        if (Written > 0)
        {
            // Что-то записалось.
            WrittenTotal += Written;
            Size -= Written;
            Q_ASSERT(Size >= 0);
            if (Size == 0)
                break;
            else {
                pData += Written;
                Pos = pFile->pos();
                continue;
            }
        }
        else {
            // Ошибка при записи.
            TErrorData ErrorData;
            ErrorData.ErrorCode = ecWriteFile;
            ErrorData.Message   = pFile->errorString();
            ErrorData.FileName  = pFile->fileName();

            errorHandler(pFileData, &ErrorData);

            if (ErrorData.Action != eaRetry)
                break;
        }
    } while (true);

    Q_ASSERT(WrittenTotal <= pCell->size());

    if (WrittenTotal > pCell->usedSize())
        WrittenTotal = pCell->usedSize();

    if (WrittenTotal > 0) {
        pFileData->WritedBytes += WrittenTotal;
        if (m_pTaskStatus)
            m_pTaskStatus->writerProgress(this, pFile, WrittenTotal);
    }

    if (m_pTaskStatus) {
        int Delta = pCell->usedSize() - WrittenTotal;
        if (Delta > 0)
            m_pTaskStatus->writerSkip(this, pFile, Delta);
    }
}

//------------------------------------------------------------------------------
//! Запись блока во все файлы-назначения.

void TWriter::writeBlock(const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdWriteBlock ||
             pCell->command() == cmdCloseFile);

    if (pCell->usedSize() == 0)
        return;

    if (pCell->usedSize() < 0) {
        qWarning("TWriter::writeBlock. "
                 "Attempt to write block with negative size (%i).",
                 pCell->usedSize());
        return;
    }

    for (int i = 0; i < m_FileData.count(); ++i)
        writeBlock(m_FileData[i], pCell);
}

//------------------------------------------------------------------------------
//! Закрытие файла-назначения.
/*!
   \remarks Если файл записан не полностью, он удаляется.
 */

void TWriter::closeFile(TFileData* pFileData, const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pFileData != NULL);
    if (pCell)
        Q_ASSERT(pCell->command() == cmdCloseFile);

    if (!pFileData->File.isOpen())
        return;

    pFileData->File.close();

    // Коррекция размера файла.
    if (pCell != NULL && pCell->CommandData.Size >= 0)
        m_Size = pCell->CommandData.Size;

    if (m_Size >= 0) {
        if (pFileData->WritedBytes < m_Size) {
            // Удаление файлов, записанных не полностью.
            pFileData->File.remove();
            // TODO: Выдать сообщение об удалённом файле.
        }
        else {
            TFastFile::resize(pFileData->File.fileName(), m_Size);
        }
    }

    if (m_pTaskStatus)
        m_pTaskStatus->writerEndFile(this, &pFileData->File);
}

//------------------------------------------------------------------------------
//! Закрытие всех файлов-назначений.

void TWriter::closeFiles(const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    if (pCell)
        Q_ASSERT(pCell->command() == cmdCloseFile);

    for (int i = m_FileData.count() - 1; i >= 0; --i)
        closeFile(m_FileData[i], pCell);

    m_FileRelName.clear();
}

//------------------------------------------------------------------------------
//! Установка статистической информации для файла-назначения.

void TWriter::setFileStat(TFileData* pFileData, const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pFileData != NULL);
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdSetFileStat);

    if (pFileData->Skip)
        return;

    const TFileStat* pFileStat = &pCell->CommandData.Stat;
    Q_ASSERT(pFileStat != NULL);

    if (pFileData->File.isOpen()) {
        qWarning("TWriter::setFileStat. File \"%s\" is open. "
                 "Possible command sequence error.",
                 qPrintable(pFileData->File.fileName()));
        pFileData->File.close();
    }

    if (pFileStat->Options != 0 && !pFileData->Skip)
    {
        QString FileName = pFileData->File.fileName();
        while (TFileInfoEx::setStat(FileName, *pFileStat) != pFileStat->Options)
        {
            TErrorData ErrorData;
            ErrorData.ErrorCode = ecSetFileStat;
            ErrorData.FileName  = FileName;
            // Строка сообщения пустая.

            errorHandler(pFileData, &ErrorData);

            if (ErrorData.Action != eaRetry)
                break;
        }
    }
}

//------------------------------------------------------------------------------
//! Установка статистической информации для всех файлов-назначений.

void TWriter::setFileStat(const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdSetFileStat);

    for (int i = 0; i < m_FileData.count(); ++i)
        setFileStat(m_FileData[i], pCell);
}

//------------------------------------------------------------------------------
//! Установка статистической информации для каталога.

void TWriter::setDirStat(TFileData* pFileData, const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdSetDirStat);
    Q_ASSERT(pFileData != NULL);

    const TFileStat* pDirStat = &pCell->CommandData.Stat;

    if (pDirStat->Options != 0)
    {
        QString DirName = AddWithSeparator(pFileData->DestDir,
                                           pCell->CommandData.ObjName);
        /* Может оказаться, что каталога по каким-то причинам не существует.
           Например, его не удалось создать или он удалён. Если в этом случае
           приходит команда установки параметров, то её нужно проигнорировать.
        */
        if (QFile::exists(DirName))
        {
            while (TFileInfoEx::setStat(DirName, *pDirStat) != pDirStat->Options)
            {
                TErrorData ErrorData;
                ErrorData.ErrorCode = ecSetDirStat;
                ErrorData.FileName  = DirName;
                // Строка сообщения пустая.

                errorHandler(pFileData, &ErrorData);

                if (ErrorData.Action != eaRetry)
                    break;
            }
        }
        else {
            qWarning("Attempt to set parameters for nonexistent folder (\"%s\").",
                     qPrintable(DirName));
        }
    }
}

//------------------------------------------------------------------------------
//! Установка статистической информации для всех каталогов.

void TWriter::setDirStat(const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell != NULL);
    Q_ASSERT(pCell->command() == cmdSetDirStat);

    for (int i = 0; i < m_FileData.count(); ++i)
        setDirStat(m_FileData[i], pCell);
}

//------------------------------------------------------------------------------
/*!
   Команда cmdUncompleteFile генерируется потоком чтения в том случае, когда
   чтение файла было прервано. В этом случае потоку записи мог быть передан
   неполный файл, который, вероятно, нужно удалить. Однако, чтение файла может
   быть прервано ДО того, как поток записи приступит к созданию файла. Например,
   такая ситуация возникает в случае, когда поток чтения открыл файл и выдал
   потоку записи команду создания нового файла, а поток записи при попытке
   создания файла потерпел неудачу и получил от пользователя команду пропуска.
   Менеджер в случае, когда потоков записи для этого файла не осталось, для
   оптимизации работы выдаёт потоку чтения команду пропустить оставшуюся часть
   файла и поток чтения сгенерирует команду cmdUncompleteFile.
 */

void TWriter::uncompleteFile(TFileData* pFileData, const TBufferCell* pCell)
{
    Q_UNUSED(pCell);
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell->command() == cmdUncompleteFile);

    // Если файл не открыт, возможно, неверная последовательность команд.
    if (pFileData->File.isOpen()) {
        // TODO: Опция "Не удалять неполные файлы".

        pFileData->File.close();
        pFileData->File.remove();
    }
}

//------------------------------------------------------------------------------

void TWriter::uncompleteFile(const TBufferCell* pCell)
{
    Q_ASSERT(isRunning());
    Q_ASSERT(pCell->command() == cmdUncompleteFile);

    for (int i = m_FileData.count() - 1; i >= 0; --i)
        uncompleteFile(m_FileData[i], pCell);
}

//------------------------------------------------------------------------------

void TWriter::errorHandler(TFileData* pFileData, TErrorData* pErrorData)
{
    if (receivers(SIGNAL(error(TErrorData*))) > 0)
    {
        pErrorData->pSender  = this;
        QSemaphore Semaphore;
        pErrorData->pLocker = &Semaphore;
        emit error(pErrorData);
        Semaphore.acquire();

        switch (pErrorData->Action) {
            case eaSkip :
            case eaSkipAll :
                pFileData->Skip = true;
                break;
            case eaCancelDest :
                pFileData->Cancel = true;
                closeFile(pFileData);
                if (m_FileData.count() <= 1)
                    cancel();
                break;
            default :
                ;
        }
    }
    else {
        pErrorData->Action = eaIgnore;
        qWarning("TWriter::errorHandler. No receivers for error signal. "
                 "Ignoring error.");
    }
}

//------------------------------------------------------------------------------

void TWriter::checkDests()
{
    TFileDataList::iterator I = m_FileData.begin();
    while(I != m_FileData.end())
    {
        if ((*I)->Cancel) {
            closeFile(*I);
            I = m_FileData.erase(I);
        }
        else
            ++I;
    }

    if (m_FileData.isEmpty()) {
        cancel();
    }
}

//------------------------------------------------------------------------------
//! Метод-обработчик команд процесса чтения.

void TWriter::process()
{
    Q_ASSERT(isRunning());

    if (m_pTaskStatus != NULL)
        m_pTaskStatus->writerBeginTask(this);

    bool RepeatFlag = true;
    do {
        const TBufferCell* pCell = acquireCell();
        if (pCell == NULL) {
            qWarning("TWriter::process. Writer %p acquired null cell. Exiting.",
                     this);
            break;
        }


        switch (pCell->command())
        {
            case cmdNoOp :
                break;
            case cmdNewFile :
                newFiles(pCell);
                break;
            case cmdNewDir :
                newDir(pCell);
                break;
            case cmdWriteBlock :
                writeBlock(pCell);
                break;
            case cmdCloseFile :
                writeBlock(pCell);
                closeFiles(pCell);
                break;
            case cmdSetFileStat :
                setFileStat(pCell);
                break;
            case cmdSetDirStat :
                setDirStat(pCell);
                break;
            case cmdUncompleteFile :
                uncompleteFile(pCell);
                break;
            case cmdEnd :
                RepeatFlag = false;
                break;
            default :
                qWarning("TWriter::process. Unknown command (%i).",
                         pCell->command());
        }

        checkDests();

        releaseCell();

        pausePoint();

        if (isCancelled()) {
            RepeatFlag = false;
            m_pBuffer->unregisterConsumer(this);
            setTaskStatus(NULL);
        }

    } while(RepeatFlag);

    closeFiles();

    if (m_pTaskStatus != NULL)
        m_pTaskStatus->writerEndTask(this);
}

//------------------------------------------------------------------------------
//! Основная функция потока.

void TWriter::run()
{
    Q_ASSERT(m_pBuffer != NULL);
    Q_ASSERT(!m_Task.isNull());

    // Проверка некритичных условий.
    if (receivers(SIGNAL(error(TErrorData*))) <= 0)
        qWarning("TWriter::run. Writer %p running without error handler.", this);
    if (m_FileData.isEmpty())
        qWarning("TWriter::run. Writer %p running with empty destinations list.", this);
    if (m_pTaskStatus == NULL)
        qWarning("TWriter::run. Writer %p running without TTaskStatus.", this);

    // Восстановление состояния потока.
    clearStateFlags();
    m_FileRelName.clear();
    m_FileNumber = 0;

    process();
}

//------------------------------------------------------------------------------
//! Конструктор.

TWriter::TWriter()
    : m_pBuffer(NULL),
      m_FileNumber(0),
      m_Size(-1),
      m_pTaskStatus(NULL)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TWriter::~TWriter()
{

}

//------------------------------------------------------------------------------
//! Установка задачи.
/*!
   \remarks Метод не должен выполняться на запущенном потоке.
 */

void TWriter::setTask(TSharedConstTask Task)
{
    if (isRunning())
        qWarning("TWriter::setTask. Method called on running thread.");

    m_Task = Task;
}

//------------------------------------------------------------------------------
//! Список каталогов назначения.

QStringList TWriter::dests() const
{
    QStringList Result;
    for (int i = 0; i < m_FileData.count(); ++i)
        Result.append(m_FileData[i]->DestDir);
    return Result;
}

//------------------------------------------------------------------------------
//! Количество каталогов назначения.

int TWriter::destsCount() const
{
    return m_FileData.count();
}

//------------------------------------------------------------------------------
//! Очистка списка каталогов назначения.

void TWriter::clearDests()
{
    for (int i = m_FileData.count() - 1; i >= 0; --i)
        delete m_FileData[i];
    m_FileData.clear();

    if (m_pTaskStatus)
        m_pTaskStatus->unregisterWriter(this);
}

//------------------------------------------------------------------------------
//! Число открытых потоком файлов-назначений.
/*!
   \remarks Возвращаемое методом значение может быть меньше числа каталогов
     назначения, возвращаемого методом \c destsCount.
 */

int TWriter::openedFiles() const
{
    int Result = 0;
    for (int i = m_FileData.count() - 1; i >= 0; --i)
        if (m_FileData[i]->File.isOpen())
            ++Result;
    return Result;
}

//------------------------------------------------------------------------------
//! Список всех обработчиков файлов-назначений потока.

QList<const void*> TWriter::handlers() const
{
    // TODO: Сделать код потокобезопасным.

    QList<const void*> Result;
    for (int i = 0; i < m_FileData.count(); ++i)
        Result.append(m_FileData[i]);
    return Result;
}

//------------------------------------------------------------------------------
//! Число активных обработчиков файлов-назначений потока.
/*!
   Следует отличать число всех обработчиков от числа активных обработчиков.
   Если копирование в некоторое назначение прервано, то число всех обработчиков
   уменьшается на единицу; если же затребован пропуск какого-либо файла, то
   число всех обработчиков не меняется, а число активных уменьшается на единицу.
   При начале обработки следующего файла число активных обработчиков вновь
   становится равным числу всех обработчиков. Если число активных обработчиков
   потока равно нулю, то поток пропускает все блоки текущего файла без
   обработки. Если суммарное число активных обработчиков всех потоков записи
   равно нулю, файл-источник не записывается никуда и можно прекратить
   операцию чтения.

   \remarks Всегда гарантируется выполнение следующих соотношений:
     0 <= activeHandlersCount <= handlersCount == destsCount
 */

int TWriter::activeHandlersCount() const
{
    int Result = 0;
    for (int i = m_FileData.count() - 1; i >= 0; --i)
        if (!m_FileData[i]->Skip)
            ++Result;
    return Result;
}

//------------------------------------------------------------------------------
//! Установка назначения.

void TWriter::setDest(const QString& Dest)
{
    Q_ASSERT(!isRunning());

    clearDests();
    TFileData* pFileData = new TFileData(Dest);
    m_FileData.append(pFileData);

    if (m_pTaskStatus)
        m_pTaskStatus->registerWriter(this, &pFileData->File);
}

//------------------------------------------------------------------------------
//! Добавление назначения.

void TWriter::addDest(const QString& Dest)
{
    Q_ASSERT(!isRunning());

    TFileData* pFileData = new TFileData(Dest);
    m_FileData.append(pFileData);

    if (m_pTaskStatus)
        m_pTaskStatus->registerWriter(this, &pFileData->File);
}

//------------------------------------------------------------------------------
//! Установка списка назначений.

void TWriter::setDests(const QStringList& Dests)
{
    Q_ASSERT(!isRunning());

    if (Dests.isEmpty())
        qWarning("TWriter::setDests. Destinations list is empty.");

    clearDests();
    QList<const void*> Files;
    for (int i = 0; i < Dests.count(); ++i)
    {
        TFileData* pFileData = new TFileData(Dests[i]);
        m_FileData.append(pFileData);
        Files.append(&pFileData->File);
    }

    if (m_pTaskStatus)
        m_pTaskStatus->registerWriter(this, Files);
}

//------------------------------------------------------------------------------
//! Добавление списка назначений.

void TWriter::addDests(const QStringList& Dests)
{
    Q_ASSERT(!isRunning());

    if (Dests.isEmpty())
        qWarning("TWriter::setDests. Destinations list is empty.");

    QList<const void*> Files;
    for (int i = 0; i < Dests.count(); ++i)
    {
        TFileData* pFileData = new TFileData(Dests[i]);
        m_FileData.append(pFileData);
        Files.append(&pFileData->File);
    }

    if (m_pTaskStatus)
        m_pTaskStatus->registerWriter(this, Files);
}

//------------------------------------------------------------------------------
//! Установка указателя на статус задания.

void TWriter::setTaskStatus(TTaskStatus* TaskStatus)
{
    if (m_pTaskStatus)
        m_pTaskStatus->unregisterWriter(this);
    m_pTaskStatus = TaskStatus;
    if (m_pTaskStatus) {
        QList<const void*> Files;
        for (int i = 0; i < m_FileData.count(); ++i)
            Files.append(&m_FileData[i]->File);
        if (Files.count() > 0)
            m_pTaskStatus->registerWriter(this, Files);
    }
}

//------------------------------------------------------------------------------
//! Относительное имя текущего обрабатываемого файла.
/*!
   \remarks Если поток записи закончил обработку файла, возвращаемое значение
     будет пустым.
 */

QString TWriter::fileRelName() const
{
    return m_FileRelName;
}

//------------------------------------------------------------------------------
