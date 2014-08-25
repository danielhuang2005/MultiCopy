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

#define _WIN32_WINNT 0x0500  // Win 2000 and above.

#include "FastFile.hpp"

#ifdef Q_OS_UNIX
    #ifndef _LARGEFILE64_SOURCE
        #define _LARGEFILE64_SOURCE
    #endif
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <stdio.h>
#endif

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! Установка строки с сообщением об ошибке.

void TFastFile::setErrorString()
{
    m_ErrorString = GetSystemErrorString();
}

//------------------------------------------------------------------------------
//! Конструктор.

TFastFile::TFastFile()
    : m_DirectAccess(true),
      m_BlockSize(-1),
      #ifdef Q_OS_WIN
          m_Handle(INVALID_HANDLE_VALUE)
      #else
          m_fd(-1)
      #endif
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TFastFile::~TFastFile()
{
}

//------------------------------------------------------------------------------
//! Имя файла.
/*!
   \sa setFileName
 */

QString TFastFile::fileName() const
{
    return m_FileName;
}

//------------------------------------------------------------------------------
//! Установка имени файла.
/*!
   \remarks Вызов этой функции при открытом файле не приведёт к его закрытию.
     Если надо открыть другой файл, используйте последовательность вызовов
     \c close, \c setFileName, \c open.
 */

void TFastFile::setFileName(const QString& FileName)
{
    m_FileName = FileName;
}

//------------------------------------------------------------------------------
//! Открывает файл.
/*!
   \arg Mode Режим открытия файла
   \arg DirectAccess Флаг прямого доступа к файлу (без кэширования).
   \arg BlockSize Размер блока при прямом доступе (если флаг прямого доступа
     не установлен, этот параметр игнорируется).

   \return true, если файл успешно открыт и false в противном случае. В случае
     ошибки текст сообщения можно получить методом \c errorString.

   \remarks Если файл был открыт, метод его закроет.

   \sa close, setFileName
 */

bool TFastFile::open(OpenMode Mode, bool DirectAccess, int BlockSize)
{
    if (DirectAccess && BlockSize <= 0) {
        qWarning("Direct access with non-positive block size. Using cached I/O.");
        DirectAccess = false;
    }

    close();
    if (isOpen()) return false;

    m_DirectAccess = DirectAccess;
    // Если нет прямого доступа, размер блока устанавливается в -1.
    m_BlockSize = m_DirectAccess ? BlockSize : -1;

#ifdef Q_OS_WIN
    DWORD dwDesiredAccess = 0;            // Режим доступа.
    DWORD dwShareMode = FILE_SHARE_READ;  // Режим совместного доступа.
        // (Другие процессы могут только читать этот файл.)

    DWORD dwCreationDisposition = 0;      // Метод открытия/создания файла.

    // Флаги и атрибуты файла.
    DWORD dwFlagsAndAttributes =
            FILE_ATTRIBUTE_NORMAL     |  // Обычный файл.
            FILE_FLAG_SEQUENTIAL_SCAN |  // Оптимизировать для
                                         // последовательного доступа.
            FILE_FLAG_BACKUP_SEMANTICS;  // Для работы с датой.
    if (m_DirectAccess) {
        // Прямой доступ к диску (без кэша).
        dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
    }

    if (Mode.testFlag(omRead))
    {
        dwDesiredAccess |= GENERIC_READ;        // Файл для чтения.
        dwCreationDisposition = OPEN_EXISTING;  // Открыть существующий.
    }
    if (Mode.testFlag(omWrite))
    {
        dwDesiredAccess |= GENERIC_WRITE;       // Файл для записи.
        dwCreationDisposition = OPEN_ALWAYS;    // Создать, если не существует.
    }


    m_Handle = CreateFileW((LPCWSTR)PathToLongWinPath(m_FileName).utf16(),
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
        qWarning("TFastFile::open. Error open file \"%s\". %s.",
                 qPrintable(m_FileName), qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }

    return m_Handle != INVALID_HANDLE_VALUE;
#else
    int flags = O_NOFOLLOW /*0*/;
    if (Mode.testFlag(omRead)) {
        flags |= O_RDONLY;
    }
    if (Mode.testFlag(omWrite))
    {
        flags |= O_WRONLY | O_CREAT;
    }
    // TODO: Реализовать некэшированную запись для Linux.
    Q_UNUSED(m_DirectAccess);
    /*if (m_DirectAccess)
        flags |= O_DIRECT | O_SYNC;*/
    mode_t mode = S_IRUSR | S_IWUSR |  // user
                  S_IRGRP |            // group
                  S_IROTH;             // other
    m_fd = ::open(m_FileName.toLocal8Bit().data(), flags, mode);
    if (m_fd == -1) {
        setErrorString();
        qWarning("TFastFile::open. Error open file \"%s\". %s.",
                 qPrintable(m_FileName), qPrintable(m_ErrorString));
    }
    else {
       m_ErrorString.clear();
    }
    return m_fd != -1;
#endif
}

//------------------------------------------------------------------------------
//! Закрывает файл.

void TFastFile::close()
{
#ifdef Q_OS_WIN
    if (m_Handle != INVALID_HANDLE_VALUE)
    {
        if (CloseHandle(m_Handle))
        {
            m_Handle = INVALID_HANDLE_VALUE;
            m_ErrorString.clear();
        }
        else {
            setErrorString();
            qWarning("TFastFile::close. Error close file \"%s\". %s.",
                     qPrintable(m_FileName),  qPrintable(m_ErrorString));
        }
    }
#else
    if (m_fd != -1) {
        if (::close(m_fd) != -1) {
            m_fd = -1;
            m_ErrorString.clear();
        }
        else {
            setErrorString();
            qWarning("TFastFile::close. Error close file \"%s\". %s.",
                     qPrintable(m_FileName),  qPrintable(m_ErrorString));
        }
    }
#endif
}

//------------------------------------------------------------------------------
//! Возвращает строку с сообщением об ошибке.

QString TFastFile::errorString() const
{
    return m_ErrorString;
}

//------------------------------------------------------------------------------
//! Метод проверки того, открыт файл или нет.

bool TFastFile::isOpen() const
{
#ifdef Q_OS_WIN
    return m_Handle != INVALID_HANDLE_VALUE;
#else
    return m_fd != -1;
#endif
}

//------------------------------------------------------------------------------
//! Чтение из файла.
/*!
   \arg data Массив, в который будут помещены считанные данные.
   \arg maxSize Максимальное число считываемых байт.

   \return Число считанных байт (может быть меньше maxSize) или -1, если
     произошла ошибка.

   \sa write
 */

qint64 TFastFile::read(char* data, qint64 maxSize)
{
    if (!isOpen()) {
        qWarning("TFastFile::read. File not open!");
        return -1;
    }

    if (maxSize < 0) {
        qWarning("TFastFile::read. "
                 "Attempt to read block with negative size.");
        return -1;
    }

    if (m_BlockSize > 0 && maxSize % m_BlockSize != 0) {
        qWarning("TFastFile::read. "
                 "Maximum size (%s) is not multiple of block size (%i).",
                 qPrintable(QString::number(maxSize)), m_BlockSize);
        return -1;
    }

#ifdef Q_OS_WIN
    // TODO: Обрабатывать большие блоки.
    /* В Windows 7 x64 экспериментально проверена работа на блоках размером до
       одного гигабайта. */

    DWORD NumberOfBytesRead;  // Число прочитанных байт.
    if (!ReadFile(m_Handle, data, maxSize, &NumberOfBytesRead, NULL))
    {
        setErrorString();
        qWarning("TFastFile::read. Error reading from file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
        return -1;
    }
    else {
        m_ErrorString.clear();
    }
    return NumberOfBytesRead;
#else
    ssize_t Readed = ::read(m_fd, data, maxSize);
    if (Readed == -1) {
        setErrorString();
        qWarning("TFastFile::read. Error reading from file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    return Readed;
#endif
}

//------------------------------------------------------------------------------
//! Запись в файл.
/*!
   \arg data Массив, данные из которого будут записаны в файл.
   \arg maxSize Максимальное число записываемых байт.

   \return Число записанных байт (может быть меньше maxSize) или -1, если
     произошла ошибка.

   \sa read
 */

qint64 TFastFile::write(const char* data, qint64 maxSize)
{
    if (!isOpen()) {
        qWarning("TFastFile::write. File not open!");
        return -1;
    }

    if (maxSize < 0) {
        qWarning("TFastFile::write. "
                 "Attempt to write block with negative size.");
        return -1;
    }

    if (m_BlockSize > 0 && maxSize % m_BlockSize != 0) {
        qWarning("TFastFile::write. "
                 "Maximum size (%s) is not multiple of block size (%i).",
                 qPrintable(QString::number(maxSize)), m_BlockSize);
        return -1;
    }

#ifdef Q_OS_WIN
    // TODO: Обрабатывать большие блоки.
    /* В Windows 7 x64 экспериментально проверена работа на блоках размером до
     * одного гигабайта. */

    DWORD NumberOfBytesWritten = -1;  // Число записанных байт.
    if (!WriteFile(m_Handle, data, maxSize, &NumberOfBytesWritten, NULL))
    {
        setErrorString();
        qWarning("TFastFile::write. Error writing to file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    return NumberOfBytesWritten;
#else
    ssize_t Written = ::write(m_fd, data, maxSize);
    if (Written == -1) {
        setErrorString();
        qWarning("TFastFile::write. Error writing to file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    return Written;
#endif
}

//------------------------------------------------------------------------------
//! Текущая позиция указателя чтения/записи в файле.
/*!
   \remarks Если произошла ошибка или файл не открыт, возвращает -1.

   \sa seek
 */

qint64 TFastFile::pos()
{
    if (!isOpen()) {
        qWarning("TFastFile::pos. File not open!");
        return -1;
    }

#ifdef Q_OS_WIN
    LARGE_INTEGER sz;
    sz.QuadPart = 0;

    if (SetFilePointerEx(m_Handle, sz, &sz, FILE_CURRENT) == 0) {
        setErrorString();
        qWarning("TFastFile::pos. Error get position on file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
        return -1;
    }
    else {
        m_ErrorString.clear();
    }
    return sz.QuadPart;
#else
    off64_t Offset = lseek64(m_fd, 0, SEEK_CUR);
    if (Offset == -1) {
        setErrorString();
        qWarning("TFastFile::pos. Error seek on file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
    }
    return Offset;
#endif
}

//------------------------------------------------------------------------------
//! Установка позиция указателя чтения/записи в файле.
/*!
   \arg Требуемая позиция указателя.

   \return true, если операция успешно завершена и false, если произошла ошибка
     или файл не открыт.

   \sa pos, resize
 */

bool TFastFile::seek(qint64 pos)
{
    if (!isOpen()) {
        qWarning("TFastFile::seek. File not open!");
        return false;
    }

    if (pos < 0) {
        qWarning("TFastFile::seek. "
                 "Attempt to seek negative file position.");
        return false;
    }

    if (m_BlockSize > 0 && pos % m_BlockSize != 0) {
        qWarning("TFastFile::seek. "
                 "Position (%s) is not multiple of block size (%i).",
                 qPrintable(QString::number(pos)), m_BlockSize);
        return false;
    }

#ifdef Q_OS_WIN
    LARGE_INTEGER sz;
    sz.QuadPart = pos;
    if (SetFilePointerEx(m_Handle, sz, NULL, FILE_BEGIN) == 0) {
        setErrorString();
        qWarning("TFastFile::seek. Error seek on file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
        return true;
    }
    return false;
#else
    off64_t Offset = lseek64(m_fd, pos, SEEK_SET);
    if (Offset == -1) {
        setErrorString();
        qWarning("TFastFile::seek. Error seek on file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }
    else {
        m_ErrorString.clear();
        return true;
    }

    return false;
#endif
}

//------------------------------------------------------------------------------
//! Изменение размера файла.
/*!
   \arg Size Требуемый размер файла.

   \return true, если операция успешно завершена и false, если произошла ошибка
     или файл не открыт, возвращает -1.

   \remarks Если операция прошла успешно, позиция указателя чтения/записи не
     меняется, в случае ошибки она может сместиться в Size.

   \remarks Если Size меньше текущего размера файла, файл урезается; если
     больше - содержимое добавленных байтов не определено.
 */

bool TFastFile::resize(qint64 Size)
{
    if (!isOpen()) {
        qWarning("TFastFile::resize. File not open!");
        return false;
    }

    if (Size < 0) {
        qWarning("TFastFile::resize. "
                 "Attempt to change file size to negative.");
        return false;
    }

    if (m_BlockSize > 0 && Size % m_BlockSize != 0) {
        qWarning("TFastFile::seek. "
                 "File size (%s) is not multiple of block size (%i).",
                 qPrintable(QString::number(Size)), m_BlockSize);
        return false;
    }

#ifdef Q_OS_WIN
    qint64 current = pos();
    if (current >= 0) {
        if (seek(Size)) {
            if (!SetEndOfFile(m_Handle)) {
                setErrorString();
                qWarning("TFastFile::resize. Error set EOF on file \"%s\". %s.",
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
#else
    if (ftruncate(m_fd, Size) != -1) {
        m_ErrorString.clear();
        return true;
    }
    else {
        setErrorString();
        qWarning("TFastFile::resize. Error truncate of file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
    }

    return false;
#endif
}

//------------------------------------------------------------------------------
//! Текущий размер файла.
/*!
   \remarks Если произошла ошибка или файл не открыт, возвращает -1.

   \sa resize
 */

qint64 TFastFile::size()
{
    if (!isOpen()) {
        qWarning("TFastFile::size. File not open!");
        return -1;
    }

#ifdef Q_OS_WIN
    LARGE_INTEGER Size;
    if (GetFileSizeEx(m_Handle, &Size) == 0) {
        setErrorString();
        qWarning("TFastFile::size. Error getting size of file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
        return -1;
    }
    else {
        m_ErrorString.clear();
        return Size.QuadPart;
    }
#else
    struct stat Stat;
    if (::fstat(m_fd, &Stat) == -1) {
        setErrorString();
        qWarning("TFastFile::size. Error getting size of file \"%s\". %s.",
                 qPrintable(m_FileName),  qPrintable(m_ErrorString));
        return -1;
    }
    else {
        return Stat.st_size;
    }
#endif
}

//------------------------------------------------------------------------------
//! Удаление файла.
/*!
   \return true, если операция успешно завершена и false, если произошла ошибка
 */

bool TFastFile::remove()
{
    close();
    return remove(m_FileName, &m_ErrorString);
}

//------------------------------------------------------------------------------

bool TFastFile::remove(const QString& FileName, QString* pErrorString)
{
    QString ErrStr;
    bool Result;
#ifdef Q_OS_WIN
    QString wFileName = PathToLongWinPath(FileName);

    // Файл с атрибутом "Read only" удалить не удастся.
    WIN32_FILE_ATTRIBUTE_DATA FileAttrData;
    if (GetFileAttributesExW((LPCWSTR)wFileName.utf16(),
                             GetFileExInfoStandard,
                             &FileAttrData) != 0)
    {
        if (FileAttrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
            // Снимаем атрибут "Read only".
            if (SetFileAttributesW((LPCWSTR)wFileName.utf16(),
                                   FileAttrData.dwFileAttributes & !FILE_ATTRIBUTE_READONLY)
                == 0)
            {
                qWarning("TFastFile::remove. Cannot set attributes for file \"%s\": %s",
                         qPrintable(FileName),  qPrintable(ErrStr));
            }
        }
    }
    else {
        qWarning("TFastFile::remove. Cannot get attributes for file \"%s\": %s",
                 qPrintable(FileName),  qPrintable(ErrStr));
    }

    // Если не удалось получить и/или изменить атрибуты, это ещё не значит,
    // что файл удалить не удастся.
    Result = DeleteFileW((LPCWSTR)wFileName.utf16());
    if (!Result) {
        ErrStr = GetSystemErrorString();
        qWarning("TFastFile::remove. Error deleting file \"%s\". %s.",
                 qPrintable(FileName),  qPrintable(ErrStr));
    }
#else
    Result = ::remove(FileName.toLocal8Bit().constData()) != -1;
    if (!Result) {
        ErrStr = GetSystemErrorString();
        qWarning("TFastFile::remove. Error deleting file \"%s\". %s.",
                 qPrintable(FileName),  qPrintable(ErrStr));
    }
#endif
    if (pErrorString != NULL)
        *pErrorString = ErrStr;

    return Result;
}

//------------------------------------------------------------------------------

bool TFastFile::resize(const QString& FileName, qint64 Size)
{
    bool Result = false;
    TFastFile FastFile;
    FastFile.setFileName(FileName);
    if (FastFile.open(omWrite)) {
        if (FastFile.resize(Size))
            Result = true;
        FastFile.close();
    }
    return Result;
}

//------------------------------------------------------------------------------
