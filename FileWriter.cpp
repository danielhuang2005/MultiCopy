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

#include "FileWriter.hpp"

#include <QDir>

#include "ControlThread.hpp"
#include "CircularBuffer.hpp"

//------------------------------------------------------------------------------
//! Конструктор.

TFileWriter::TFileWriter(TControlThread* pControlThread)
    : TThreadEx(NULL),
      m_pControlThread(pControlThread),
      m_Cancel(false),
      m_Size(-1)
      #ifndef _NO_CHECK_MD5
          , m_MD5(QCryptographicHash::Md5)
      #endif
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TFileWriter::~TFileWriter()
{
    closeFile(); // На всякий случай... ;-)
}

//------------------------------------------------------------------------------
//! Главная функция потока.

void TFileWriter::run()
{
    if (!readyToRun())
    {
        qWarning("The thread which isn't ready to start is launched!");
        return;
    }
    m_Cancel = false;

    #ifndef _NO_CHECK_MD5
        m_MD5.reset();
    #endif

    m_Written = 0;
    int Length = -1;
    do {
        // Точка паузы потока.
        pausePoint();

        m_pControlThread->buffer()->acquireConsumerSemaphore(this);

        if (m_Cancel)
            break;

        Length = writeBlock();

        m_pControlThread->buffer()->releaseProducerSemaphore(this);

        if (Length > 0) {
            m_Written += Length;
            m_pControlThread->writedBlock(this, Length);
        }
    } while(Length > 0);


    closeFile();
}

//------------------------------------------------------------------------------
//! Имя файла.

QString TFileWriter::fileName() const
{
    return m_File.fileName();
}

//------------------------------------------------------------------------------
//! Отмена операции.

void TFileWriter::cancel()
{
    m_Cancel = true;
}

//------------------------------------------------------------------------------
//! Открытие файла.
/*!
 * \param FileName Имя файла.
 * \param Size Требуемый объём файла. Если по завершении потока объём файла
 *   не совпадёт с требуемым, файл удаляется. Чтобы оставить недописанный
 *   файл, укажите объём равным -1.
 * \param UseCache Флаг использования системного кеша.
 *
 * \return true, если файл успешно открыт и false в пртивном случае.
 */

bool TFileWriter::openFile(const QString& FileName, qint64 Size, bool UseCache)
{
    Q_ASSERT(!isRunning());

    m_Size = Size;

    QFileInfo FileInfo(FileName);
    QString AbsPath = FileInfo.absolutePath();
    QDir Dir;

    // Проверяем существование каталога.
    if (!Dir.exists(AbsPath))
    {
        // Каталог не существует.
        do {
            // Пытаемся создать каталог.
            if (!Dir.mkpath(AbsPath))
            {
                // Создать каталог не удалось. Обрабатываем ошибку.
                TErrorHandler::ErrorData ED;
                ED.Code = TErrorHandler::eMakeDir;
                ED.FileName = AbsPath;
                // Строка сообщения пустая.
                if (m_pControlThread->error(&ED, this) != TErrorHandler::aRetry)
                {
                    // Если не была выбрана опция "повторить", то каталога не
                    // существует. Дальнейшая работа невозможна. Возвращаем
                    // false и выходим.
                    return false;
                }
            }
            else {
                // Каталог успещно создан. Прерываем цикл.
                break;
            }
        } while (true);
    }

    if (FileInfo.exists())
    {
        // Файл уже существует. Обрабатываем ситуацию.
        TErrorHandler::ErrorData ED;
        ED.Code = TErrorHandler::eAlreadyExists;
        ED.FileName = FileName;
        // Строка сообщения пустая.
        TErrorHandler::Action A = m_pControlThread->error(&ED, this);
        if ((A != TErrorHandler::aOverwrite) && (A != TErrorHandler::aOverwriteAll))
        {
            // Если не была затребована перезапись,
            // то файл должен быть пропущен.
            return false;
        }
    }

    m_File.setFileName(FileName);
    do {
        // Пытаемся открыть файл.
        if (!m_File.open(QIODevice::WriteOnly, UseCache))
        {
            // Открыть файл не удалось. Обрабатываем ошибку.
            TErrorHandler::ErrorData ED;
            ED.Code = TErrorHandler::eCreateFile;
            ED.Message = m_File.errorString();
            ED.FileName = FileName;
            if (m_pControlThread->error(&ED, this) != TErrorHandler::aRetry)
                break;
        }
        else {
            // Файл успешно открыт. Прерываем цикл.
            break;
        }
    } while (true);

    m_File.resize(Size);
    return m_File.isOpen();
}

//------------------------------------------------------------------------------
//! Запись блока в файл.
/*!
 * \return Число записанных байт.
 */

qint64 TFileWriter::writeBlock()
{
    // Получаем адрес ячейки.
    TBufferCell* pCell = m_pControlThread->buffer()->firstUsedBlock(this);
    Q_ASSERT(pCell);
    int Written;
    int WrittenTotal = 0;
    int toWrite = pCell->Length;
    int Delta = 0;
    do {
        Q_ASSERT(Delta + toWrite <= pCell->Length);
        Written = m_File.write(pCell->data() + Delta, toWrite);
        if (Written > 0)
            WrittenTotal += Written;
        if (Written != toWrite) {
            // Ошибка при записи. Вызываем обработчик.
            TErrorHandler::ErrorData ED;
            ED.Code = TErrorHandler::eWriteFile;
            ED.Message = m_File.errorString();
            ED.FileName = fileName();
            if (m_pControlThread->error(&ED, this) == TErrorHandler::aRetry)
            {
                // Если что-то записалось, корректируем число байт для записи
                // и сдвиг относительно начала блока.
                if (Written > 0) {
                    toWrite -= Written;
                    Delta += Written;
                }
            }
            else {
                break;
            }
        }
        else {
            // Блок записан успешно. Прерываем цикл.
            #ifndef _NO_CHECK_MD5
                m_MD5.addData(pCell->data(), pCell->Length);
            #endif

            break;
        }
    } while (true);

    // Если блок записан не полностью, возвращаем -1.
    return (WrittenTotal == pCell->Length) ? WrittenTotal : -1;
}

//------------------------------------------------------------------------------
//! Закрытие файла.

void TFileWriter::closeFile()
{
    if (m_File.isOpen() && (m_Size >= 0) && (m_Written != m_Size))
    {
        // Удаление недописанных файлов.
        m_File.remove();
    }
    m_File.close();
}

//------------------------------------------------------------------------------
//! Признак отмены выполнения.
/*!
 * Возвращает true, если операция была отменена и false в противном случае.
 */

bool TFileWriter::isCancelled() const
{
    return m_Cancel;
}

//------------------------------------------------------------------------------
//! Признак готовности потока к запуску.
/*!
 * Возвращает true, если поток готов к запуску и false в противном случае.
 */


bool TFileWriter::readyToRun() const
{
    return m_File.isOpen();
}


//------------------------------------------------------------------------------
//! Число записанных байт.

qint64 TFileWriter::written() const
{
    return m_Written;
}

//------------------------------------------------------------------------------
