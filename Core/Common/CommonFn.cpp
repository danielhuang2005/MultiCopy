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

#include <QtGlobal>

#ifdef Q_OS_WIN
    #define _WIN32_WINNT 0x5000 // Windows 2000 and above.
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <errno.h>
    #include <sys/vfs.h>
    #include <sys/resource.h>
    #include <stdlib.h>

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <utime.h>
#endif

#include "CommonFn.hpp"

#include <QDir>
#include <QCoreApplication>

//------------------------------------------------------------------------------
//! Конкатенация строк со вставкой между ними символа-разделителя каталогов.
/*!
   \remarks Используется символ-разделитель, специфичный для операционной
     системы, на которой выполняется приложение.
 */

void AddWithSeparator(QString* Initial, const QString& Added)
{
    if (!Initial->isEmpty() && !Added.isEmpty())
    {
        if (!Initial->endsWith(QDir::separator()))
            *Initial += QDir::separator();
    }
    *Initial += Added;
}

//------------------------------------------------------------------------------
/*!
   \overload
 */

QString AddWithSeparator(const QString& Initial, const QString& Added)
{
    QString Result = Initial;
    AddWithSeparator(&Result, Added);
    return Result;
}

//------------------------------------------------------------------------------
//! Выделение заблокированной (не сбрасываемой на диск) памяти.
/*!
   Функция выделяет блок оперативной памяти размера Size и пытается защитить его
   от сбрасывания на диск (заблокировать), если параметр Lock установлен в true.
   Если память успешно выделена, возвращает указатель на неё, если память
   выделить не удалось, вызвращает нулевой указатель. Если параметр Lock
   установлен в false, память не блокируется. Если заблокировать память не
   удалось, возвращает указатель на незаблокированную выделенную память.

   Если затребована блокировка памяти, т.е. в параметре Lock передано true, то
   при успешном выделении памяти в этом параметре возвращается true, если память
   заблокирована и false, если блокировка не удалась. Если блокировка памяти не
   затребована, значение параметра Lock не изменяется.

   \remarks Для освобождения памяти, выделенной данной функцией,
     необходимо использовать функцию FreeLockedMem.
 */

void* GetLockedMem(size_t Size, bool *Lock)
{
    void* Pointer;

#ifdef Q_OS_WIN
    Pointer = VirtualAlloc(NULL,
                           Size,
                           MEM_COMMIT | MEM_RESERVE,
                           PAGE_READWRITE);
    if (Pointer != NULL) {
        // Память успешно выделена.
        if (Lock && *Lock) {
            // Пытаемся заблокировать память.
            if (!VirtualLock(Pointer, Size)) {
                *Lock = false;
                // Память не удалось заблокировать.
                SIZE_T Min, Max;
                HRESULT HResult = GetProcessWorkingSetSize(GetCurrentProcess(),
                                                           &Min, &Max);
                if (SUCCEEDED(HResult)) {
                    Min += Size;
                    Max += Size;
                    if (SetProcessWorkingSetSize(GetCurrentProcess(), Min, Max))
                    {
                        // Размер рабочего набора памяти изменён.
                        // Повторно пытаемся заблокировать память.
                        if (VirtualLock(Pointer, Size)) {
                            *Lock = true;
                        }
                        else {
                            // Повторная попытка блокировки не удалась.
                            qWarning("GetLockedMem. Error locking memory: %s",
                                     qPrintable(GetSystemErrorString()));
                        }
                    }
                    else {
                        // Ошибка при изменении размера рабочего набора памяти.
                        qWarning("GetLockedMem. SetProcessWorkingSetSize error: %s",
                                 qPrintable(GetSystemErrorString()));
                    }
                }
                else {
                    // Ошибка при получении размера рабочего набора памяти.
                    qWarning("GetLockedMem. GetProcessWorkingSetSize error: %s",
                             qPrintable(GetSystemErrorString()));
                }
            }
        }
    }
    else {
        // При выделении памяти произошла ошибка.
        qWarning("GetLockedMem. Error allocate memory: %s",
                 qPrintable(GetSystemErrorString()));
    }
#else
    // TODO: Реализовать для UNIX.
    Pointer = malloc(Size);
    //posix_memalign(&Pointer, 512, Size);
    if (Pointer) {
        // Память успешно выделена.
        if (Lock && *Lock) {
            // Пытаемся заблокировать память.
            /*if (mlock(Pointer, Size) == -1) {
                // Ошибка при блокировке памяти.
                qWarning("Error locking memory: %s",
                         qPrintable(GetSystemErrorString()));
            }*/
            qWarning("GetLockedMem. Memory lock function is not implemented.");
            *Lock = false;
        }
    }
    else {
        qWarning("GetLockedMem. Error allocate memory: %s",
                 qPrintable(GetSystemErrorString()));
    }
#endif

    return Pointer;
}

//------------------------------------------------------------------------------
//! Объём доступной физической памяти.
/*!
 * Функция возвращает объём доступной физической памяти в системе. Если объём
 * определить не удалось, возвращает -1.
 */

qint64 AvailablePhysicalMemory()
{
    #ifdef Q_OS_WIN
        MEMORYSTATUSEX MemStatus;
        MemStatus.dwLength = sizeof(MemStatus);
        if (GlobalMemoryStatusEx(&MemStatus)) {
            return MemStatus.ullAvailPhys;
        }
        else {
            qWarning("AvailablePhysicalMemory. GlobalMemoryStatusEx error: %s.",
                     qPrintable(GetSystemErrorString()));
        }
    #else
        long AvailPhysPages = sysconf(_SC_AVPHYS_PAGES),
             PageSize = sysconf(_SC_PAGE_SIZE);
        if ((AvailPhysPages > 0) && (PageSize > 0))  {
            return AvailPhysPages * PageSize;
        }
        else {
            qWarning("AvailablePhysicalMemory. Error detecting RAM Size.");
        }
    #endif

    return -1;
}

//------------------------------------------------------------------------------
//! Освобожение заблокированной (не сбрасываемой на диск) памяти.
/*!
 * Функция используется для освобождения памяти, выделенной функцией
 * GetLockedMem.
 *
 * \param Pointer Указатель на память, выделенную функцией GetLockedMem.
 */

void FreeLockedMem(void* Pointer)
{
    #ifdef Q_OS_WIN
        if (!VirtualFree(Pointer, 0, MEM_RELEASE)) {
            qWarning("FreeLockedMem. Error free memory: %s",
                     qPrintable(GetSystemErrorString()));
        }
    #else
        free(Pointer);
    #endif
}

//------------------------------------------------------------------------------
//! Строковое описание системной ошибки.

QString GetSystemErrorString(
            #ifdef Q_OS_WIN
                DWORD
            #else
                int
            #endif
                ErrCode)
{
#ifdef Q_OS_WIN
    WCHAR* errStr = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   ErrCode,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPWSTR)&errStr,
                   0,
                   NULL);
    QString Result = QString::fromWCharArray(errStr);
    LocalFree(errStr);
    return Result;
#else
    return QString::fromLocal8Bit(strerror(ErrCode));
#endif
}

//------------------------------------------------------------------------------
//! Строковое описание последней системной ошибки.

QString GetSystemErrorString()
{
    #ifdef Q_OS_WIN
        return GetSystemErrorString(GetLastError());
    #else
        return GetSystemErrorString(errno);
    #endif
}

//------------------------------------------------------------------------------
//! Преобразование пути в длинный путь Windows.

QString PathToLongWinPath(const QString& Path)
{
    static const QLatin1String LocalPrefix = QLatin1String("\\\\?\\");
    static const QLatin1String UNCPrefix   = QLatin1String("\\\\?\\UNC\\");

    QString S = QDir::toNativeSeparators(Path);
    if (!S.startsWith(LocalPrefix))
    {
        if (S.startsWith("\\\\"))
        {
            return UNCPrefix + S.mid(2);
        }
        else {
            return LocalPrefix + S;
        }
    }
    else return Path;
}

//------------------------------------------------------------------------------

bool isNetworkPath(const QString& Path)
{
    return Path.startsWith("\\\\") || Path.startsWith("//");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

QString speedToStr(qint64 BytesPerSec, int Digits)
{
    if (BytesPerSec < 1024)
        return QString::number(BytesPerSec) + " " +
                QCoreApplication::translate("Common::speedToStr", "B/s");
    qreal f = BytesPerSec / 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', Digits) + " " +
                QCoreApplication::translate("Common::speedToStr", "kB/s");
    f /= 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', Digits) + " " +
                QCoreApplication::translate("Common::speedToStr", "MB/s");
    f /= 1024.0;
    if (f < 1024.0)
        return QString::number(f, 'g', Digits) + " " +
                QCoreApplication::translate("Common::speedToStr", "GB/s");
    f /= 1024.0;
    return QString::number(f, 'g', Digits) + " " +
            QCoreApplication::translate("Common::speedToStr", "TB/s");
}

//------------------------------------------------------------------------------

QString sizeToStr(qint64 Bytes, int Digits)
{
    if (Bytes < 1024)
        return QString::number(Bytes) + " " +
                QCoreApplication::translate("Common::sizeToStr", "B");
    qreal f = Bytes / 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', Digits) + " " +
                QCoreApplication::translate("Common::sizeToStr", "kB");
    f /= 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', Digits) + " " +
                QCoreApplication::translate("Common::sizeToStr", "MB");
    f /= 1024.0;
    if (f < 1024.0)
        return QString::number(f, 'g', Digits) + " " +
                QCoreApplication::translate("Common::sizeToStr", "GB");
    f /= 1024.0;
    return QString::number(f, 'g', Digits) + " " +
            QCoreApplication::translate("Common::sizeToStr", "TB");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Определение логического тома (точки монтирования) по пути.
/*!
   Функция определяет точку монтирования тома, на котором расположен объект с
   путём Path. Если при определении точки монтирования произошла ошибка,
   возвращает пустую строку.
 */

QString GetDriveByPath(const QString& Path)
{
    #ifdef Q_OS_WIN
        // Имя точки монтирования не может быть длиннее пути!
        int BufferLength = Path.length() + 1;
        WCHAR VolumePathName[BufferLength];
        if (GetVolumePathNameW((WCHAR*)Path.utf16(), VolumePathName, BufferLength) != 0)
        {
            return QString::fromWCharArray(VolumePathName);
        }
        else {
            qWarning("GetDriveByPath. GetVolumePathNameW error on \"%s\": %s",
                     qPrintable(Path), qPrintable(GetSystemErrorString()));
        }

        return QString();
    #else
        Q_ASSERT_X(false, "GetDriveByPath", "Function is not implemented!");
        return QString();
    #endif
}

//------------------------------------------------------------------------------
//! Определение свободного объёма дисковой памяти.
/*!
 * Функция определяет объём дисковой памяти, доступный пользователю для записи.
 * Если определить объём не удалось, возвращает -1.
 */

qint64 DiskFreeSpace(const QString& Path)
{
    #ifdef Q_OS_WIN
        ULARGE_INTEGER FreeBytesAvailable;
        FreeBytesAvailable.QuadPart = -1;

        if (!GetDiskFreeSpaceExW((LPCWSTR)Path.utf16(), &FreeBytesAvailable, NULL, NULL)) {
            qWarning("GetDiskFreeSpaceExW error: %s",
                     qPrintable(GetSystemErrorString()));
        }
        return FreeBytesAvailable.QuadPart;
    #else
        struct statfs StatFS;
        if (::statfs(Path.toLocal8Bit().constData(), &StatFS) != -1) {
            struct stat Stat;
            if (::stat(Path.toLocal8Bit().constData(), &Stat) != -1) {
                return StatFS.f_bavail * Stat.st_blksize;
            }
        }

        qWarning("statfs error: %s", qPrintable(GetSystemErrorString()));
        return -1;
    #endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
