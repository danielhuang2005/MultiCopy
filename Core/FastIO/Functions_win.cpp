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

#if defined(QT_VERSION) && !defined(Q_OS_WIN)
    #error "This file may be compiled in Windows only!"
#endif

//------------------------------------------------------------------------------
// Этот define должен быть раньше всех include!
#define _WIN32_WINNT 0x0501  // Windows XP или выше.

// Этот include должен быть первым!
#include "Functions_win.hpp"

#include <winioctl.h>
#include <shlobj.h>
#include <lm.h>

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! Перечисление сетевых ресурсов.
/*!
   Функция перечисляет сетевые ресурсы сервера с именем ServerName и заполняет
   полученными именами список pSharesList. Возвращает true, если перечисление
   успешно завершено и false в случае ошибки.

   \remarks Параметр ServerName не должны иметь в себе элементов пути,
     т.е. не должен содержать символы-разделители '\', '/', ':'.
 */

bool enumServerShares(const QString& ServerName, QStringList *const pSharesList)
{
    if (pSharesList == NULL)
        return false;

    SHARE_INFO_0* pInfo = NULL;
    DWORD EntriesRead  = 0;
    DWORD TotalEntries = 0;
    DWORD ResumeHandle = 0;
    DWORD naStatus     = 0;

    do {
        naStatus = NetShareEnum((LPWSTR)ServerName.utf16(),
                                0,
                                (LPBYTE*)&pInfo,
                                MAX_PREFERRED_LENGTH,
                                &EntriesRead,
                                &TotalEntries,
                                &ResumeHandle);
        if (naStatus == ERROR_SUCCESS || naStatus == ERROR_MORE_DATA)
        {
            SHARE_INFO_0* p = pInfo;
            for (DWORD i = 0; i < EntriesRead; ++i)
            {
                if (p->shi0_netname != NULL)
                {
                    pSharesList->append(QString::fromWCharArray(p->shi0_netname));
                }
                ++p;
            }
            DWORD ErrCode = NetApiBufferFree(pInfo);
            if (ErrCode != NERR_Success) {
                qWarning("enumServerShares. NetApiBufferFree error: %s",
                         qPrintable(GetSystemErrorString(ErrCode)));
            }
        }
        else {
            qWarning("enumServerShares. NetShareEnum error on server name \"%s\": %s",
                     qPrintable(ServerName),
                     qPrintable(GetSystemErrorString(naStatus)));
        }
    } while (naStatus == ERROR_MORE_DATA);

    return naStatus == ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
/*!
   Проверка существования сетевого каталога Share на сервере с именем
   ServerName.

   \remarks Параметры ServerName и Share не должны иметь в себе элементов пути,
     т.е. не должны содержать символы-разделители '\', '/', ':'.
 */

bool isNetworkShareExists(const QString& ServerName, const QString& Share)
{
    QStringList Shares;
    if (!enumServerShares(ServerName, &Shares)) {
        qWarning("Error enumerating shares on server \"%s\".",
                 qPrintable(ServerName));
    }
    return Shares.contains(Share, Qt::CaseInsensitive);
}

//------------------------------------------------------------------------------
/*!
   Метод для объекта файловой системы с именем Name заполняет данными объекта
   структуру WIN32_FIND_DATAW. Возвращает true, если данные успешно получены
   и false в случае ошибки.
 */

bool getFileFindData(const QString& Name, WIN32_FIND_DATAW *const pData)
{
    if (pData == NULL) {
        qWarning("getFileFindData. Null pointer to WIN32_FIND_DATAW.");
        return false;
    }

    HANDLE hFind = FindFirstFileExW((LPCWSTR)Name.utf16(),
                                    FindExInfoStandard,
                                    pData,
                                    FindExSearchNameMatch,
                                    NULL,
                                    0);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        if (!FindClose(hFind)) {
            qWarning("getFileFindData. FindClose error: %s",
                     qPrintable(GetSystemErrorString()));
        }
        return true;
    }
    else {
        qWarning("getFileFindData. FindFirstFileExW error "
                 "on object \"%s\": %s",
                 qPrintable(Name), qPrintable(GetSystemErrorString()));
    }

    return false;
}

//------------------------------------------------------------------------------
/*!
   Метод для объекта файловой системы с именем Name заполняет данными объекта
   структуру WIN32_FILE_ATTRIBUTE_DATA. Возвращает true, если данные успешно
   получены и false в случае ошибки.
 */

bool getFileAttributesData(const QString& Name, WIN32_FILE_ATTRIBUTE_DATA *const pData)
{
    if (pData == NULL) {
        qWarning("getFileAttributesData. Data pointer is NULL.");
        return false;
    }

    if (GetFileAttributesExW((LPCWSTR)Name.utf16(), GetFileExInfoStandard, pData) != 0)
    {
        return true;
    }
    else {
        qWarning("getFileAttributesData. GetFileAttributesExW error "
                 "on object \"%s\": %s",
                 qPrintable(Name), qPrintable(GetSystemErrorString()));
    }

    return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
    #define MAXIMUM_REPARSE_DATA_BUFFER_SIZE 16384
#endif

#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
    struct REPARSE_DATA_BUFFER {
        DWORD  ReparseTag;
        WORD   ReparseDataLength;
        WORD   Reserved;
        union {
            struct {
                WORD   SubstituteNameOffset;
                WORD   SubstituteNameLength;
                WORD   PrintNameOffset;
                WORD   PrintNameLength;
                ULONG  Flags;
                WCHAR PathBuffer[1];
            } SymbolicLinkReparseBuffer;
            struct {
                WORD   SubstituteNameOffset;
                WORD   SubstituteNameLength;
                WORD   PrintNameOffset;
                WORD   PrintNameLength;
                WCHAR PathBuffer[1];
            } MountPointReparseBuffer;
            struct {
                BYTE   DataBuffer[1];
            } GenericReparseBuffer;
        };
    };
    #define REPARSE_DATA_BUFFER_HEADER_SIZE \
            FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
#endif

//------------------------------------------------------------------------------
/*!
   Возвращает точку монтирования тома по его GUID-имени.

   \remarks Ищет самый короткий путь, т.е. тот, в котором наименьшее количество
     каталогов. Если несколько путей имеют одинаковое число каталогов,
     возвращается первый встретившийся.
 */

QString resolveVolume(const QString& VolumeName)
{
    QString Result;

    QLatin1String Prefix("\\\\?\\");
    QString wS;
    if (!VolumeName.startsWith(Prefix))
        wS = Prefix + VolumeName;
    else
        wS = VolumeName;

    DWORD Length;
    GetVolumePathNamesForVolumeNameW((LPCWSTR)wS.utf16(), NULL, 0, &Length);
    DWORD ErrCode = GetLastError();
    if (ErrCode == ERROR_MORE_DATA)
    {
        WCHAR* Names = new WCHAR[Length];
        if (GetVolumePathNamesForVolumeNameW((LPCWSTR)wS.utf16(), Names, Length, &Length) != 0)
        {
            int SeparatorsCount = -1;
            WCHAR* pS = Names;
            while (*pS != '\0') {
                QString Path = QString::fromWCharArray(pS);
                int SeparatorsCount2 = Path.count('\\');
                if (SeparatorsCount < 0 || SeparatorsCount2 < SeparatorsCount)
                {
                    Result = Path;
                    SeparatorsCount = SeparatorsCount2;
                }
                pS += Path.length() + 1;
            }
        }
        else {
            qWarning("resolveVolume. GetVolumePathNamesForVolumeNameW error: \"%s\"",
                     qPrintable(GetSystemErrorString()));
        }
        delete[] Names;
    }
    else {
        qWarning("resolveVolume. GetVolumePathNamesForVolumeNameW error: \"%s\"",
                 qPrintable(GetSystemErrorString(ErrCode)));
    }

    return Result;
}

//------------------------------------------------------------------------------
/*!
   Возвращает имя, на которое указывает точка повторной обработки NTFS с именем
   LongWinPath. Если объект не является точкой повторной обработки, возвращает
   пустую строку.

   \remarks Возвращённый объект может оказаться ещё одной точкой повторной
     обработки.

   \remarks Если точка повторной обработки указывает на диск, не имеющий других
     точек монтирования, то возвращается исходное имя.
 */

QString resolveReparsePoint(const QString& LongWinPath)
{
    QString Result;

    HANDLE hFile = CreateFileW((LPCWSTR)LongWinPath.utf16(),
                               FILE_READ_EA,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               0,
                               OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                               0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        REPARSE_DATA_BUFFER* pRDB = static_cast<REPARSE_DATA_BUFFER*>(
                                        VirtualAlloc(NULL,
                                            MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                                            MEM_COMMIT | MEM_RESERVE,
                                            PAGE_READWRITE)
                                    );
        if (pRDB != NULL)
        {
            DWORD BytesReturned = 0;
            if (DeviceIoControl(hFile,
                                FSCTL_GET_REPARSE_POINT,
                                NULL,
                                0,
                                pRDB,
                                MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                                &BytesReturned,
                                NULL) != 0)
            {
                if (pRDB->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
                {
                    int Length = pRDB->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
                    int Offset = pRDB->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* pName = &pRDB->MountPointReparseBuffer.PathBuffer[Offset];
                    Result = QString::fromWCharArray(pName, Length);
                }
                else if (pRDB->ReparseTag == IO_REPARSE_TAG_SYMLINK)
                {
                    int Length = pRDB->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
                    int Offset = pRDB->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* pName = &pRDB->SymbolicLinkReparseBuffer.PathBuffer[Offset];
                    Result = QString::fromWCharArray(pName, Length);
                }
                else {
                    qWarning("resolveReparsePoint. Unknown reparse point tag: %lX",
                             pRDB->ReparseTag);
                }

                if (Result.startsWith("\\??\\") || Result.startsWith("\\\\?\\"))
                {
                    Result.remove(0, 4);
                }

                if (Result.startsWith("Volume{"))
                {
                    Result = resolveVolume(Result);
                }
            }
            else {
                qWarning("resolveReparsePoint. DeviceIoControl error on file \"%s\": %s",
                         qPrintable(LongWinPath), qPrintable(GetSystemErrorString()));
            }

            if (!VirtualFree(pRDB, 0, MEM_RELEASE)) {
                qWarning("resolveReparsePoint. VirtualFree error: %s",
                         qPrintable(GetSystemErrorString()));
            }
        }
        else {
            qWarning("resolveReparsePoint. VirtualAlloc error on %i bytes: %s",
                     MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                     qPrintable(GetSystemErrorString()));
        }

        if (!CloseHandle(hFile)) {
            qWarning("resolveReparsePoint. CloseHandle error: %s",
                     qPrintable(GetSystemErrorString()));
        }
    }
    else {
        qWarning("resolveReparsePoint. CreateFileW error on file \"%s\": %s",
                 qPrintable(LongWinPath), qPrintable(GetSystemErrorString()));
    }

    return Result;
}

//------------------------------------------------------------------------------
/*!
   Возвращает имя, на которое указывает ярлык Windows (lnk-файл) с именем
   LongWinPath.

   \remarks Возвращённый объект может оказаться ещё одним ярлыком.
 */

QString resolveLnkFile(const QString& LongWinPath)
{
    QString Result;

    HRESULT hResult = CoInitialize(NULL);
    if (hResult == S_OK || hResult == S_FALSE)
    {
        IShellLink* pShellLink = NULL;
        hResult = CoCreateInstance(CLSID_ShellLink,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IShellLink,
                                   (void**)&pShellLink);
        if (SUCCEEDED(hResult))
        {
            IPersistFile* pPersistFile = NULL;
            hResult = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
            if (SUCCEEDED(hResult))
            {
                hResult = pPersistFile->Load((LPCOLESTR)LongWinPath.utf16(), STGM_READ);
                if (SUCCEEDED(hResult))
                {
                    const int BufferLength = 32768;
                    wchar_t TargetPath[BufferLength];
                    hResult = pShellLink->GetPath(TargetPath, BufferLength, NULL, 0);
                    if (SUCCEEDED(hResult))
                    {
                        Result = QString::fromWCharArray(TargetPath);
                    }
                    else {
                        qWarning("resolveLnkFile. IShellLink::GetPath error: %li", hResult);
                    }
                }
                else {
                    qWarning("resolveLnkFile. IPersistFile::Load error: %li", hResult);
                }
                pPersistFile->Release();
            }
            else {
                qWarning("resolveLnkFile. Error querying interface IPersistFile: %li", hResult);
            }
            pShellLink->Release();
        }
        else {
            qWarning("resolveLnkFile. Error creating IShellLink instance: %li", hResult);
        }
        CoUninitialize();
    }
    else {
        qWarning("resolveLnkFile. Error initializing COM: %li", hResult);
    }

    return Result;
}

//------------------------------------------------------------------------------
