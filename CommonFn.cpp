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

#include "CommonFn.hpp"

#include <QDir>
#include <QDirIterator>

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <errno.h>
    #include <sys/vfs.h>
    //#include <sys/stat.h>
    #include <sys/resource.h>
    #include <stdlib.h>
#endif


//------------------------------------------------------------------------------
//! Преобразование QString в LPWSTR.
/*!
 * Создаёт массив элементов типа WCHAR, заканчивающийся нулём. Использует
 * минимально необходимый объём памяти. Динамически создаёт массив требуемого
 * размера и возвращает указатель на него. Вызывающая процедура должна
 * сама его разрушить вызовом оператора delete. Если при выделении памяти
 * возникла ошибка, возвращает NULL.
 */

wchar_t* StrToLPWSTR(const QString& Str)
{
    wchar_t* Result = new(std::nothrow) wchar_t[(Str.length() + 1)];
    if (Result != NULL)
        Result[Str.toWCharArray(Result)] = 0;
    return Result;
}

//------------------------------------------------------------------------------
//! Конкатенация строк со вставкой между ними символа-разделителя каталогов.

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
//! Перегруженный вариант функции.

QString AddWithSeparator(const QString& Initial, const QString& Added)
{
    QString Result = Initial;
    AddWithSeparator(&Result, Added);
    return Result;
}

//------------------------------------------------------------------------------
//! Выделение заблокированной (не сбрасываемой на диск) памяти.
/*!
 * Функция выделяет блок оперативной памяти размера Size и пытается защитить его
 * от сбрасывания на диск (заблокировать), если параметр Lock установлен в true.
 * Если память успешно выделена, возвращает указатель на неё, если память
 * выделить не удалось, вызвращает нулевой указатель. Если параметр Lock
 * установлен в false, память не блокируется. Если заблокировать память не
 * удалось, возвращает указатель на незаблокированную выделенную память.
 *
 * \remarks Для освобождения памяти, выделенной данной функцией,
 *   необходимо использовать функцию FreeLockedMem.
 */

void* GetLockedMem(size_t Size, bool Lock)
{
    void* Pointer;

#ifdef Q_OS_WIN
    Pointer = VirtualAlloc(NULL,
                           Size,
                           MEM_COMMIT | MEM_RESERVE,
                           PAGE_READWRITE);
    if (Pointer != NULL) {
        // Память успешно выделена.
        if (Lock) {
            // Пытаемся заблокировать память.
            if (!VirtualLock(Pointer, Size)) {
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
                        if (!VirtualLock(Pointer, Size)) {
                            // Повторная попытка блокировки не удалась.
                            qWarning("Error locking memory: %s",
                                     qPrintable(GetSystemErrorString()));
                        }
                    }
                    else {
                        // Ошибка при изменении размера рабочего набора памяти.
                        qWarning("SetProcessWorkingSetSize error: %s",
                                 qPrintable(GetSystemErrorString()));
                    }
                }
                else {
                    // Ошибка при получении размера рабочего набора памяти.
                    qWarning("GetProcessWorkingSetSize error: %s",
                             qPrintable(GetSystemErrorString()));
                }
            }
        }
    }
    else {
        // При выделении памяти произошла ошибка.
        qWarning("Error allocate memory: %s",
                 qPrintable(GetSystemErrorString()));
    }
#else
    Pointer = malloc(Size);
    //posix_memalign(&Pointer, 512, Size);
    if (Pointer) {
        // Память успешно выделена.
        /*if (Lock) {
            // Пытаемся заблокировать память.
            if (mlock(Pointer, Size) == -1) {
                // Ошибка при блокировке памяти.
                qWarning("Error locking memory: %s",
                         qPrintable(GetSystemErrorString()));
            }
        }*/
    }
    else {
        qWarning("Error allocate memory: %s",
                 qPrintable(GetSystemErrorString()));
    }
#endif

    return Pointer;
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
        qWarning("Error free memory: %s", qPrintable(GetSystemErrorString()));
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
    static const QString LocalPrefix = "\\\\?\\";
    static const QString UNCPrefix   = LocalPrefix + "UNC\\";

    if (!Path.startsWith(LocalPrefix))
    {
        QString S = QDir::toNativeSeparators(Path);
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
//! Определение свободного объёма дисковой памяти.
/*!
 * Функция определяет объём дисковой памяти, доступный пользователю для записи.
 * Если определить объём не удалось, возвращает -1.
 */

qint64 DiskFreeSpace(const QString& Path)
{
#ifdef Q_OS_WIN
    LPWSTR wPath = StrToLPWSTR(Path);
    ULARGE_INTEGER FreeBytesAvailable;
    FreeBytesAvailable.QuadPart = -1;
    if (!GetDiskFreeSpaceExW(wPath, &FreeBytesAvailable, NULL, NULL)) {
        qWarning("GetDiskFreeSpaceExW error: %s",
                 qPrintable(GetSystemErrorString()));
    }
    delete wPath;
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

void SubfoldersList(const QString& Dir,
                    const QString& SubDir,
                    QStringList* pList,
                    int Depth,
                    bool FullPath)
{
    if (Depth > 0) --Depth;

/*    QDir D(AddWithSeparator(Dir, SubDir));
    QStringList List = D.entryList(QDir::AllDirs | QDir::NoDotAndDotDot |
                                   QDir::Hidden | QDir::System);
    QString DirName;
    for (QStringList::const_iterator I = List.begin(); I != List.end(); ++I)
    {
        DirName = AddWithSeparator(SubDir, *I);
        if (FullPath)
            pList->append(AddWithSeparator(Dir, DirName));
        else
            pList->append(DirName);
        if (Depth != 0)
            SubfoldersList(Dir, DirName, pList, Depth, FullPath);
    }*/
    QDirIterator DirI(AddWithSeparator(Dir, SubDir), QDir::AllDirs |
                                                     QDir::NoDotAndDotDot |
                                                     QDir::Hidden |
                                                     QDir::System);
    QString DirName;
    while (DirI.hasNext()) {
        DirI.next();
        DirName = AddWithSeparator(SubDir, DirI.fileName());
        if (FullPath)
            pList->append(DirI.filePath());
        else
            pList->append(DirName);
        if (Depth != 0)
            SubfoldersList(Dir, DirName, pList, Depth, FullPath);
    }
}

//------------------------------------------------------------------------------
//! Список подкаталогов.
/*!
 * Функция возвращает список подкаталогов каталога Dir до глубины Depth
 * (ноль соответствует подкаталогам первого уровня). Если Depth установлено в
 * -1, глубина не ограничивается. Если параметр FullPath установлен в true,
 * имена подкаталогов будут содержать в начале каталог Dir, в противном случае
 * имена подкаталогов будут относительными.
 */

void SubfoldersList(QStringList* pList,
                    const QString& Dir,
                    int Depth,
                    bool FullPath)
{
    SubfoldersList(Dir, QString(), pList, Depth, FullPath);
}


//------------------------------------------------------------------------------
//! Список подкаталогов.
/*!
 * Перегруженный вариант функции.
 */

QStringList SubfoldersList(const QString& Dir, int Depth, bool FullPath)
{
    QStringList List;
    SubfoldersList(&List, Dir, Depth, FullPath);
    return List;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Получение статистики файла.
/*!
 * Функция получает данные о файле с именем FileName, указанные в поле Options
 * структуры pFileStat. После завершения работы функции в этом поле содержится
 * перечень успешно полученных данных, а соответствующие поля структуры
 * заполнены этими данными. Остальные поля не изменяются.
 */

void GetFileStat(const QString& FileName, TFileStat* pFileStat)
{
    TFileStatOptions Opts = pFileStat->Options;
    pFileStat->Options = TFileStatOptions();
#if defined(Q_OS_WIN)
    LPWSTR wFileName = StrToLPWSTR(PathToLongWinPath(FileName));
    if (Opts.testFlag(fsoTime))
    {
        HANDLE hFile = CreateFileW(wFileName,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_FLAG_BACKUP_SEMANTICS,
                                   NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            qWarning("Cannot open file %ls for reading: %s",
                     wFileName, qPrintable(GetSystemErrorString()));
        }
        else {
            if (!GetFileTime(hFile,
                             &pFileStat->CreationTime,
                             &pFileStat->LastAccessTime,
                             &pFileStat->LastWriteTime)) {
                qWarning("Cannot get date/time of file %ls: %s",
                         wFileName, qPrintable(GetSystemErrorString()));
            }
            else {
                pFileStat->Options |= fsoTime;
            }
            CloseHandle(hFile);
        }
    }

    if (Opts.testFlag(fsoAttr))
    {
        pFileStat->Attr = GetFileAttributesW(wFileName);
        if (pFileStat->Attr == INVALID_FILE_ATTRIBUTES) {
            qWarning("Cannot get attributes for file %ls: %s",
                     wFileName, qPrintable(GetSystemErrorString()));
        }
        else {
            pFileStat->Options |= fsoAttr;
        }
    }

    delete wFileName;
#else
    QByteArray File = FileName.toLocal8Bit();
    struct stat Stat;
    if (stat(File.data(), &Stat) != 0) {
        qWarning("Cannot get statistics for file %s: %s",
                 File.data(), qPrintable(GetSystemErrorString()));
    }
    else {
        if (Opts.testFlag(fsoTime)) {
            pFileStat->AccessTime = Stat.st_atime;
            pFileStat->ModificationTime = Stat.st_mtime;
            pFileStat->Options |= fsoTime;
        }
        if (Opts.testFlag(fsoAttr)) {
            pFileStat->Mode = Stat.st_mode;
            pFileStat->Options |= fsoAttr;
        }
    }

#endif
}

//------------------------------------------------------------------------------
//! Установка статистики файла.
/*!
 * Функция устанавливает параметры файла с именем FileName, перечисленные в поле
 * Options стркутуры pFileStat. Возвращает перечень успешно установленных
 * параметров.
 */

TFileStatOptions SetFileStat(const QString& FileName, const TFileStat* pFileStat)
{
    TFileStatOptions Result;
#ifdef Q_OS_WIN
    LPWSTR wFileName = StrToLPWSTR(PathToLongWinPath(FileName));

    if (pFileStat->Options.testFlag(fsoTime))
    {
        HANDLE hFile = CreateFileW(wFileName,
                                   GENERIC_WRITE,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_FLAG_BACKUP_SEMANTICS,
                                   NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            qWarning("Cannot open file %ls for writing: %s",
                     wFileName, qPrintable(GetSystemErrorString()));
        }
        else {
            if (!SetFileTime(hFile,
                             &pFileStat->CreationTime,
                             &pFileStat->LastAccessTime,
                             &pFileStat->LastWriteTime))
            {
                qWarning("Cannot set date/time for file %ls: %s",
                         wFileName, qPrintable(GetSystemErrorString()));
            }
            else {
                Result |= fsoTime;
            }
            CloseHandle(hFile);
        }
    }

    if (pFileStat->Options.testFlag(fsoAttr))
    {
        if (!SetFileAttributesW(wFileName, pFileStat->Attr)) {
            qWarning("Cannot set attributes for file %ls: %s",
                     wFileName, qPrintable(GetSystemErrorString()));
        }
        else {
            Result |= fsoAttr;
        }
    }

    delete wFileName;
#else
    QByteArray File = QDir::toNativeSeparators(FileName).toLocal8Bit();
    if (pFileStat->Options.testFlag(fsoTime))
    {
        utimbuf Utimbuf;
        Utimbuf.actime  = pFileStat->AccessTime;
        Utimbuf.modtime = pFileStat->ModificationTime;
        if (utime(File.data(), &Utimbuf) != 0) {
            qWarning("Cannot set time for file %s: %s",
                     File.data(), qPrintable(GetSystemErrorString()));
        }
        else {
            Result |= fsoTime;
        }
    }


    if (pFileStat->Options.testFlag(fsoAttr))
    {
        if (chmod(File.data(), pFileStat->Mode) != 0) {
            qWarning("Cannot set permissions for file %s: %s",
                     File.data(), qPrintable(GetSystemErrorString()));
        }
        else {
            Result |= fsoAttr;
        }
    }
#endif
    return Result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
