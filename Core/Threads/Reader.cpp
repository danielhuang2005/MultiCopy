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

#include "Reader.hpp"

#include <QSemaphore>
#include <QDir>

#include "Core/Common/CommonFn.hpp"
#include "Core/IO/DirEnumerator.hpp"
#include "Core/FastIO/FastFile.hpp"
#include "Core/FastIO/FileInfoEx.hpp"
#include "Core/Task/TaskStatus.hpp"
#include "Core/Errors/ErrorsAndActions.hpp"
#include "Core/Buffer/CircularBuffer.hpp"

//------------------------------------------------------------------------------
//! Получение и блокировка ячейки кольцевого буфера.

TBufferCell* TReader::acquireCell()
{
    Q_ASSERT(isRunning());
    Q_ASSERT(m_pBuffer != NULL);

    m_pBuffer->acquireProducerSemaphore();
    return m_pBuffer->firstFreeBlock();
}

//------------------------------------------------------------------------------
//! Освобождение ячейки кольцевого буфера.

void TReader::releaseCell()
{
    Q_ASSERT(isRunning());
    Q_ASSERT(m_pBuffer != NULL);

    m_pBuffer->releaseConsumerSemaphore();
}

//------------------------------------------------------------------------------
//! Открытие файла.

bool TReader::openFile()
{
    Q_ASSERT(isRunning());

    bool NoUseCache = m_Task->TaskSettings.NoUseCache &&
                      (m_pFileInfoEx->size() > m_Task->TaskSettings.NoUseCacheFor);

    while (!m_File.open(TFastFile::omRead,
                        NoUseCache,
                        IO_BLOCK_SIZE))
    {
        // Открыть файл не удалось. Обрабатываем ошибку.
        TErrorData ErrorData;
        ErrorData.ErrorCode = ecOpenFile;
        ErrorData.Message   = m_File.errorString();
        ErrorData.FileName  = m_File.fileName();

        errorHandler(&ErrorData);

        if (ErrorData.Action != eaRetry)
            break;
    }

    m_Readed = 0;

    return m_File.isOpen();
}

//------------------------------------------------------------------------------
//! Чтение следующего блока данных из файла.

qint64 TReader::readNextBlock()
{
    Q_ASSERT(isRunning());
    Q_ASSERT(m_File.isOpen());

    qint64 Readed = 0;
    TBufferCell* pCell = acquireCell();
    Q_ASSERT(pCell != NULL);

    do {
        if (Readed < 0) {
            /* Если length < 0, значит, при чтении блока произошла ошибка
               и был затребован повтор операции. */
            if (m_File.seek(m_Readed))
                Readed = 0;
        }
        if (Readed >= 0) {
            Q_ASSERT(Readed <= pCell->size());
            Readed = m_File.read(pCell->data(), pCell->size());
        }


        if (Readed < 0)
        {
            // Ошибка при чтении. Вызываем обработчик.
            TErrorData ErrorData;
            ErrorData.ErrorCode = ecReadFile;
            ErrorData.Message   = m_File.errorString();
            ErrorData.FileName  = m_File.fileName();

            errorHandler(&ErrorData);

            if (ErrorData.Action != eaRetry)
                break;
        }
        else {
            // Блок успешно прочитан. Прерываем цикл.
            break;
        }
    } while (true);

    /* Readed даже при прямом чтении с диска может быть некратно размеру
       физического сектора. Но такой вариант возможен только если мы считываем
       "остаток" файла. */

    if (Readed >= 0)
    {
        pCell->setUsedSize(Readed);
        m_Readed += Readed;

        if (Readed == pCell->size()) {
            /* Ячейка полностью заполнена. С высокой вероятностью в файле есть
               ещё данные. (Кратность размера файла размеру ячейки маловероятна)
            */
            pCell->setCommand(cmdWriteBlock);
        }
        else {
            /* Ячейка заполнена не полностью. Считаем, что файл закончился. */
            pCell->setCommand(cmdCloseFile);
            pCell->CommandData.Size = m_Readed;
        }
    }
    else /* Readed < 0 */ {
        // Произошла невосстановимая ошибка чтения.
        pCell->setCommand(cmdNoOp);
    }

    releaseCell();

    // Для экономии времени вначале освобождаем ячейку и только затем
    // пересчитываем статистику.
    if (Readed > 0) {
        if (m_pTaskStatus)
            m_pTaskStatus->readerProgress(Readed);
    }
    return Readed;
}

//------------------------------------------------------------------------------
//! Чтение файла.
/*!
   Метод читает файл от начала до конца. Возвращает true, если файл прочитан
   успешно и false, если произошла ошибка. Число успешно прочитанных байт можно
   получить с помощью метода \c readed. Если файл открыт в режиме разрешения
   записи другим процессам, число прочитанных байт может отличаться от
   первоначального размера файла.
 */

bool TReader::readFile()
{
    Q_ASSERT(isRunning());
    Q_ASSERT(m_File.isOpen());

    qint64 FileSize = m_File.size();
    do {
        pausePoint();
        if (isCancelled() || isSkipped())
            break;
    } while(readNextBlock() > 0);

    /* TODO: Здесь возможна ошибка. Если разрешить открывать файлы, открытые
       другими процессами для записи, то размер файла может измениться.
       Возможно, стоит вначале попытаться открыть файл с запретом записи другим
       процессам, а затем - с разрешением. Далее здесь проверяем режим. */
    qint64 NotReaded = FileSize - m_Readed;
    if (FileSize >= 0)
    {
        if (NotReaded < 0) {
            qWarning("TReader::readFile. "
                     "From the file \"%s\" is readed more than its size.",
                     qPrintable(m_File.fileName()));
        }

        if (NotReaded > 0) {
            // Файл прочитан не полностью.
            // TODO: Опция "не удалять неполные файлы".
            if (m_pTaskStatus != NULL) {
                m_pTaskStatus->readerSkip(NotReaded);
            }
            TBufferCell* pCell = acquireCell();
            pCell->setCommand(cmdUncompleteFile);
            pCell->CommandData.ObjName = m_FileRelName;
            releaseCell();
        }
    }
    else {
        /* m_File.size() < 0 - не удалось получить размер файла. Поскольку
           размер файла определить не удалось, мы не можем сказать, прочитан он
           полностью или нет. Возможно даже, что при чтении очередного блока
           произошла ошибка. В этом случае можно было бы сгенерировать команду
           "неполный файл", но невозможность получить размер файла является
           нестандартной ситуацией, поэтому такую команду генерировать не будем.
        */
    }

    return NotReaded <= 0;
}

//------------------------------------------------------------------------------
//! Обработка файла.

void TReader::processFile()
{
    Q_ASSERT(m_pFileInfoEx->isFile());

    pausePoint();
    if (isCancelled()) return;

    m_Skip = false;
    m_File.setFileName(m_pFileInfoEx->name());
    if (openFile())
    {
        ++m_FileNumber;

        // Команда создания нового файла.
        TBufferCell* pCell = acquireCell();
        Q_ASSERT(pCell != NULL);

        qint64 FileSize = m_File.size();
        m_FileRelName = m_pDirEnumerator->relName();
        pCell->setCommand(cmdNewFile);
        pCell->CommandData.ObjName = m_FileRelName;
        pCell->CommandData.Size    = FileSize;
        pCell->setUsedSize(0);

        /* В статистику информация о файле должна быть занесена ДО освобождения
           ячейки буфера. Иначе поток записи может успеть выдать какую-либо
           команду в объект TaskStatus до регистрации в нём файла. */
        if (m_pTaskStatus != NULL) {
            m_pTaskStatus->readerNewFile(m_pDirEnumerator->startDirPath(),
                                         m_pDirEnumerator->relName(false),
                                         FileSize);
        }
        releaseCell();

        readFile();

        m_File.close();

        if (m_pTaskStatus != NULL)
            m_pTaskStatus->readerEndFile();

        if (!m_Skip && m_FileStatOptions != 0)
        {
            // Команда установки параметров файла.
            TBufferCell* pCell = acquireCell();
            pCell->setCommand(cmdSetFileStat);
            pCell->CommandData.ObjName = m_FileRelName;
            pCell->CommandData.Stat    = m_pFileInfoEx->stat(m_FileStatOptions);
            // TODO: Выдать сообщение, если получены не все параметры.
            releaseCell();
        }

        m_FileRelName.clear();
    }
    else {
        if (m_pTaskStatus != NULL) {
            m_pTaskStatus->readerSkipFile(m_pDirEnumerator->startDirPath(),
                                          m_FileRelName,
                                          m_pFileInfoEx->size());
        }
    }
}

//------------------------------------------------------------------------------
//! Обработка входа в (под)каталог.

void TReader::processDirIn()
{
    Q_ASSERT(m_pFileInfoEx->isDir());

    pausePoint();
    if (isCancelled()) return;

    if (m_Task->TaskSettings.CopyEmptyDirs)
    {
        // Команда создания каталога.
        TBufferCell* pCell = acquireCell();
        pCell->setCommand(cmdNewDir);
        pCell->CommandData.ObjName = m_pDirEnumerator->relPath();
        releaseCell();
    }
}

//------------------------------------------------------------------------------
//! Обработка выхода из (под)каталога.

void TReader::processDirOut()
{
    for (int i = m_pDirEnumerator->subdirOutCount() - 1; i >= 0; --i)
    {
        pausePoint();
        if (isCancelled()) return;

         if (m_FileStatOptions != 0)
         {
            // Команда установки параметров каталога.
            TBufferCell* pCell = acquireCell();
            pCell->setCommand(cmdSetDirStat);
            pCell->CommandData.ObjName = m_pDirEnumerator->subdirOutRelName(i);
            pCell->CommandData.Stat    = m_pDirEnumerator->subdirOutStat(i);
            releaseCell();
        }
    }
}

//------------------------------------------------------------------------------
//! Обработка одного источника.

void TReader::processSource(const TDirEnumerator::TParams &Params)
{
    if (m_pDirEnumerator->isStarted()) {
        qWarning("TReader::run. Dir enumerator is not finished.");
    }

    pausePoint();
    if (isCancelled()) return;

    while (!m_pDirEnumerator->start(Params))
    {
        m_pFileInfoEx = m_pDirEnumerator->infoPtr();
        TErrorData ErrorData;
        ErrorData.ErrorCode = ecBadSrc;
        if (m_pFileInfoEx->isLink()) {
            ErrorData.FileName = m_pFileInfoEx->linkTarget();
            ErrorData.Message  = tr("(According to link \"%1\")")
                                 .arg(QDir::toNativeSeparators(m_pFileInfoEx->name()));
        }
        else {
            ErrorData.FileName = Params.startPath;
            // Строка сообщения пустая.
        }

        errorHandler(&ErrorData);

        if (ErrorData.Action != eaRetry)
            break;
    }

    if (m_pDirEnumerator->isStarted() && !isCancelled())
    {
        do {
            processDirOut();
            m_pFileInfoEx = m_pDirEnumerator->infoPtr();

            if (m_pFileInfoEx->exists()) {
                if (m_pFileInfoEx->isFile())
                {
                    // Обрабатываем объект как файл.
                    /* Если ссылки необходимо разыменовывать, это выполнит
                       перечислитель элементов файловой системы. Если здесь мы
                       получили ссылку, её необходимо обрабатывать как файл. */
                    processFile();
                }
                else {
                    // Обрабатываем объект как каталог.
                    processDirIn();
                }
            }
            else {
                qWarning("TReader::processSource. Object \"%s\" is not exists.",
                         qPrintable(m_pFileInfoEx->name()));
            }
        } while (m_pDirEnumerator->next() && !isCancelled());
        processDirOut();
    }

    m_pDirEnumerator->finish();
}

//------------------------------------------------------------------------------
//! Обработка всего списка источников.

void TReader::process()
{
    // Настройки задания.
    const TTaskSettings* pTaskSettings = &m_Task->TaskSettings;

    // Опции выборки статистики объектов файловой системы.
    m_FileStatOptions = 0;
    if (pTaskSettings->CopyAttr)
        m_FileStatOptions |= fsoAttr;
    if (pTaskSettings->CopyDateTime)
        m_FileStatOptions |= fsoTime;

    // Инициализация неизменяющихся параметров перечислителя.
    // TODO: Такой же код в коде класса TSizeCalculator.
    TDirEnumerator::TParams Params;
    Params.filter = TDirEnumerator::Files;
    if (pTaskSettings->CopyEmptyDirs)
        Params.filter |= TDirEnumerator::Dirs;
    if (pTaskSettings->CopyHidden)
        Params.filter |= TDirEnumerator::Hidden;
    if (pTaskSettings->CopySystem)
        Params.filter |= TDirEnumerator::System;
    if (pTaskSettings->FollowShortcuts)
        Params.filter |= TDirEnumerator::FollowShortcuts;
    Params.dirStatOptions = m_FileStatOptions;
    Params.subdirsDepth = pTaskSettings->SubDirsDepth;
    m_pDirEnumerator->setRelNameWithRoot(!pTaskSettings->NoCreateRootDir);

    if (m_pTaskStatus)
        m_pTaskStatus->readerBeginTask();

    // Цикл по всем элементам списка источников.
    for (int i = 0; i < m_Task->SrcList.count(); ++i)
    {
        pausePoint();
        if (isCancelled()) break;

        Params.startPath = m_Task->SrcList.at(i);
        processSource(Params);
    }

    TBufferCell* pCell = acquireCell();
    pCell->setCommand(cmdEnd);
    releaseCell();

    if (m_pTaskStatus)
        m_pTaskStatus->readerEndTask();
}

//------------------------------------------------------------------------------
//! Обработчик ошибок.
/*!
   Метод вызывается из других методов экземпляра класса, если произошла ошибка,
   при которой действия зависят от ответа пользователя. Аргумент метода -
   pErrorData - заполняется информацией об ошибке вызывающим методом.
   Обработчик приостанавливает выполнение текущего потока до получения ответа
   пользователя.
 */

void TReader::errorHandler(TErrorData* pErrorData)
{
    if (receivers(SIGNAL(error(TErrorData*))) > 0)
    {
        pErrorData->pSender = this;
        QSemaphore Semaphore;
        pErrorData->pLocker = &Semaphore;
        emit error(pErrorData);
        Semaphore.acquire();
    }
    else {
        pErrorData->Action = eaIgnore;
        qWarning("TReader::errorHandler. No receivers for error signal.");
    }
}

//------------------------------------------------------------------------------
//! Основная функция потока.

void TReader::run()
{
    Q_ASSERT(m_pBuffer != NULL);
    Q_ASSERT(!m_Task.isNull());

    // Восстановление состояния потока.
    clearStateFlags();
    m_FileRelName.clear();
    m_FileNumber = 0;


    // Проверка некритичных условий.
    if (receivers(SIGNAL(error(TErrorData*))) <= 0) {
        qWarning("TReader::run. Reader %p running without error handler.", this);
    }
    if (m_pTaskStatus == NULL) {
        qWarning("TReader::run. Reader running without TTaskStatus.");
    }
    if (m_Task->SrcList.isEmpty()) {
        qWarning("TReader::run. Sources list is empty.");
        return;
    }

    process();
}

//------------------------------------------------------------------------------
//! Конструктор.

TReader::TReader()
    : m_FileNumber(0),
      m_pDirEnumerator(new TDirEnumerator()),
      m_pTaskStatus(NULL),
      m_pBuffer(NULL),
      m_pFileInfoEx(NULL),
      m_Readed(0),
      m_Skip(false)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TReader::~TReader()
{
    delete m_pDirEnumerator;
}

//------------------------------------------------------------------------------
//! Установка задачи.
/*!
   \remarks Метод не должен выполняться на запущенном потоке.
 */

void TReader::setTask(TSharedConstTask Task)
{
    if (isRunning()) {
        qWarning("TReader::setTask. Method called on running thread.");
    }

    m_Task = Task;
}

//------------------------------------------------------------------------------
//! Установка указателя на кольцевой буфер.

void TReader::setBuffer(TCircularBuffer* pBuffer)
{
    Q_ASSERT(!isRunning());

    m_pBuffer = pBuffer;
}

//------------------------------------------------------------------------------
//! Пропуск текущего файла.
/*!
   Метод устанавливает внутренний флаг пропуска текущего обрабатываемого файла.
   Действие метода не мгновенно, поток чтения может после его вызова выполнить
   над файлом некоторые операции, например, открыть или прочитать следующую
   порцию данных. Гарантируется лишь, что поток прервёт обработку при первом
   удобном случае.

   \remarks Если поток не запущен на выполнение, вызов метода игнорируется.
 */

void TReader::skip()
{
    if (isRunning())
        m_Skip = true;
}

//------------------------------------------------------------------------------
//! Относительное имя текущего обрабатываемого файла.
/*!
   \remarks Если поток чтения обрабатывает каталог, возвращаемое значение
     будет пустым.
 */

QString TReader::fileRelName() const
{
    return m_FileRelName;
}

//------------------------------------------------------------------------------
