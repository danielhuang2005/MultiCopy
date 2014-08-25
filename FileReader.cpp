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

#include "FileReader.hpp"

#include "ControlThread.hpp"
#include "CircularBuffer.hpp"

//------------------------------------------------------------------------------
//! Конструктор

TFileReader::TFileReader(TControlThread* pControlThread)
    : TThreadEx(NULL),
      m_pControlThread(pControlThread),
      m_Cancel(false)
{
}

//------------------------------------------------------------------------------
//! Главная функция потока.

void TFileReader::run()
{
    m_Cancel = false;

    quint64 ReadedBytes = 0;
    int Length = -1;
    do {
        // Точка паузы потока.
        pausePoint();

        m_pControlThread->buffer()->acquireProducerSemaphore();

        if (m_Cancel)
            break;

        Length = readBlock();

        m_pControlThread->buffer()->releaseConsumerSemaphore();

        if (Length > 0) {
            ReadedBytes += Length;
            m_pControlThread->readedBlock(Length);
        }
    } while (Length > 0);

    closeFile();
}

//------------------------------------------------------------------------------
//! Имя файла.

QString TFileReader::fileName() const
{
    return m_File.fileName();
}

//------------------------------------------------------------------------------
//! Отмена операции.

void TFileReader::cancel()
{
    m_Cancel = true;
}

//------------------------------------------------------------------------------
//! Открытие файла.
/*!
 * \param FileName Имя файла.
 *
 * \return true, если файл успешно открыт и false в пртивном случае.
 */

bool TFileReader::openFile(const QString& FileName)
{
    Q_ASSERT(!isRunning());

    m_File.setFileName(FileName);
    do {
        // Пытаемся открыть файл.
        if (!m_File.open(QIODevice::ReadOnly))
        {
            // Открыть файл не удалось. Обрабатываем ошибку.
            TErrorHandler::ErrorData ED;
            ED.Code = TErrorHandler::eOpenFile;
            ED.Message = m_File.errorString();
            ED.FileName = FileName;
            if (m_pControlThread->error(ED, this) != TErrorHandler::aRetry)
                break;
        }
        else {
            // Файл успешно открыт. Прерываем цикл.
            break;
        }
    } while(true);

    return m_File.isOpen();
}

//------------------------------------------------------------------------------
//! Чтение блока из файла.
/*!
 * \return Число прочитанных байт.
 */

qint64 TFileReader::readBlock()
{
    // Получаем адрес ячейки.
    TBufferCell *pCell = m_pControlThread->buffer()->firstFreeBlock();
    int Length;
    do {
        // Читаем блок из файла.
        Length = m_File.read(pCell->data(), pCell->size());
        if (Length < 0) {
            // Ошибка при чтении. Вызываем обработчик.
            TErrorHandler::ErrorData ED;
            ED.Code = TErrorHandler::eReadFile;
            ED.Message = m_File.errorString();
            ED.FileName = fileName();
            if (m_pControlThread->error(ED, this) != TErrorHandler::aRetry)
                break;
        }
        else {
            // Блок успешно прочитан. Прерываем цикл.
            break;
        }
    } while(true);

    // Возвращаем число считанных байт (или -1 при возникновении ошибки).
    return pCell->Length = Length;
}

//------------------------------------------------------------------------------
//! Закрытие файла.

void TFileReader::closeFile()
{
    m_File.close();
}

//------------------------------------------------------------------------------
//! Признак отмены выполнения.
/*!
 * Возвращает true, если операция была отменена и false в противном случае.
 */

bool TFileReader::isCancelled() const
{
    return m_Cancel;
}

//------------------------------------------------------------------------------
//! Признак готовности потока к запуску.
/*!
 * Возвращает true, если поток готов к запуску и false в противном случае.
 */

bool TFileReader::readyToRun() const
{
    return m_File.isOpen();
}


//------------------------------------------------------------------------------
