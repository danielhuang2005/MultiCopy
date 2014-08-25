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

#include "FastFile.hpp"

#if defined(Q_OS_WIN) && !defined(_NO_FAST_FILE)

#include "CommonFn.hpp"

//------------------------------------------------------------------------------

void TFastFile::setErrorString()
{
    m_ErrorString = GetSystemErrorString();
}

//------------------------------------------------------------------------------
//! Конструктор.

TFastFile::TFastFile()
    : m_Handle(INVALID_HANDLE_VALUE)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TFastFile::~TFastFile()
{

}

//------------------------------------------------------------------------------

QString TFastFile::fileName() const
{
    return m_FileName;
}

//------------------------------------------------------------------------------

void TFastFile::setFileName(const QString& FileName)
{
    m_FileName = FileName;
}

//------------------------------------------------------------------------------

bool TFastFile::open(QIODevice::OpenMode Mode)
{
    close();
    if (isOpen()) return false;

    // TODO : Проверять неподдерживаемые флаги.

    LPWSTR wFileName = StrToLPWSTR(PathToLongWinPath(m_FileName));
    DWORD dwDesiredAccess = 0;            // Режим доступа.
    DWORD dwShareMode = FILE_SHARE_READ;  // Режим совместного доступа.
        // (Другие процессы могут только читать этот файл.)

    DWORD dwCreationDisposition = 0;      // Метод открытия/создания файла.

    // Флаги и атрибуты файла.
    DWORD dwFlagsAndAttributes =
            FILE_ATTRIBUTE_NORMAL     |  // Обычный файл.
            FILE_FLAG_SEQUENTIAL_SCAN |  // Оптимизировать для
                                         // последовательного доступа.
            FILE_FLAG_WRITE_THROUGH;     // Прямая запись на диск (без кэша).

    if (Mode.testFlag(QIODevice::ReadOnly))
    {
        dwDesiredAccess |= GENERIC_READ;        // Файл для чтения.
        dwCreationDisposition = OPEN_EXISTING;  // Открыть если существует.
    }
    if (Mode.testFlag(QIODevice::WriteOnly))
    {
        dwDesiredAccess |= GENERIC_WRITE;       // Файл для записи.
        dwCreationDisposition = OPEN_ALWAYS;   // Создать если не существует.
    }


    m_Handle = CreateFileW(wFileName,
                           dwDesiredAccess,
                           dwShareMode,
                           NULL,                  // Атрибуты безопасности.
                           dwCreationDisposition,
                           dwFlagsAndAttributes,
                           NULL                   // Файл-шаблон.
               );
    if (m_Handle == INVALID_HANDLE_VALUE)
    {
        setErrorString();
        qWarning("Error open file \"%s\". %s.",
                 qPrintable(m_FileName), qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }

    delete wFileName;
    return m_Handle != INVALID_HANDLE_VALUE;
}

//------------------------------------------------------------------------------

void TFastFile::close()
{
    if (m_Handle != INVALID_HANDLE_VALUE)
    {
        if (CloseHandle(m_Handle))
        {
            m_Handle = INVALID_HANDLE_VALUE;
            m_ErrorString.clear();
        }
        else {
            setErrorString();
            qWarning("Error close file \"%s\". %s.",
                     qPrintable(m_FileName),  qPrintable(m_ErrorString));
        }
    }
}

//------------------------------------------------------------------------------

QString TFastFile::errorString() const
{
    return m_ErrorString;
}

//------------------------------------------------------------------------------

bool TFastFile::isOpen() const
{
    return m_Handle != INVALID_HANDLE_VALUE;
}

//------------------------------------------------------------------------------

qint64 TFastFile::read(char* data, qint64 maxSize)
{
    if (!isOpen()) {
        qWarning("File not open!");
        return -1;
    }

    // TODO : Обрабатывать большие блоки.
    /* В Windows 7 x64 экспериментально проверена работа на блоках размером до
     * одного гигабайта. */

    DWORD NumberOfBytesRead;  // Число прочитанных байт.
    if (!ReadFile(m_Handle, data, maxSize, &NumberOfBytesRead, NULL))
    {
        setErrorString();
        qWarning("Error reading from file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    return NumberOfBytesRead;
}

//------------------------------------------------------------------------------

qint64 TFastFile::write(const char* data, qint64 maxSize)
{
    if (!isOpen()) {
        qWarning("File not open!");
        return -1;
    }

    // TODO : Обрабатывать большие блоки.
    /* В Windows 7 x64 экспериментально проверена работа на блоках размером до
     * одного гигабайта. */

    DWORD NumberOfBytesWritten;  // Число записанных байт.
    if (!WriteFile(m_Handle, data, maxSize, &NumberOfBytesWritten, NULL))
    {
        setErrorString();
        qWarning("Error writing to file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    return NumberOfBytesWritten;
}

//------------------------------------------------------------------------------

qint64 TFastFile::pos()
{
    LARGE_INTEGER sz;
    sz.QuadPart = 0;
    sz.u.LowPart = SetFilePointer(m_Handle,       // Дескриптор файла.
                                  0,              // Младшая часть сдвига.
                                  &sz.u.HighPart, // Старшая часть сдвига.
                                  FILE_CURRENT);  // От текущей позиции.
    if ((sz.u.LowPart == INVALID_SET_FILE_POINTER) &&
        (GetLastError() != NO_ERROR))
    {
        setErrorString();
        qWarning("Error get position on file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
        return -1;
    }

    return sz.QuadPart;
}

//------------------------------------------------------------------------------

bool TFastFile::seek(qint64 pos)
{
    LARGE_INTEGER sz;
    sz.QuadPart = pos;
    if ((SetFilePointer(m_Handle, sz.u.LowPart, &sz.u.HighPart, FILE_BEGIN) ==
         INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
    {
        setErrorString();
        qWarning("Error seek on file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool TFastFile::resize(qint64 Size)
{
    qint64 current = pos();
    if (current >= 0) {
        if (seek(Size)) {
            if (!SetEndOfFile(m_Handle)) {
                setErrorString();
                qWarning("Error set EOF on file \"%s\". %s.",
                         qPrintable(m_FileName),  qPrintable(m_ErrorString));
                seek(current);
            }
            else {
                if (seek(current))
                    return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------

qint64 TFastFile::size()
{
    LARGE_INTEGER Size;
    Size.u.LowPart = GetFileSize(m_Handle, (DWORD*)&Size.u.HighPart);
    if ((Size.u.LowPart == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR))
    {
        setErrorString();
        qWarning("Error get size of file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
        return -1;
    }
    else {
        m_ErrorString.clear();
    }
    return Size.QuadPart;
}

//------------------------------------------------------------------------------

bool TFastFile::remove()
{
    close();

    LPWSTR wFileName = StrToLPWSTR(PathToLongWinPath(m_FileName));
    bool Result = DeleteFileW(wFileName);
    if (!Result) {
        setErrorString();
        qWarning("Error deleting file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    delete wFileName;

    return Result;
}

//------------------------------------------------------------------------------


#endif // Q_OS_WIN
