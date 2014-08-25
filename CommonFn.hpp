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

#ifndef __COMMONFN__HPP__
#define __COMMONFN__HPP__

#include <QString>
#include <QStringList>
#include <QFlags>

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <utime.h>
    #include <sys/stat.h>
#endif


//------------------------------------------------------------------------------

wchar_t* StrToLPWSTR(const QString& Str);
void AddWithSeparator(QString* Initial, const QString& Added);
QString AddWithSeparator(const QString& Initial, const QString& Added);
QString PathToLongWinPath(const QString& Path);

//------------------------------------------------------------------------------

void* GetLockedMem(size_t Size, bool Lock = false);
void FreeLockedMem(void* Pointer);

//------------------------------------------------------------------------------

qint64 DiskFreeSpace(const QString& Path);

//------------------------------------------------------------------------------

QString GetSystemErrorString();
QString GetSystemErrorString(
            #ifdef Q_OS_WIN
                DWORD
            #else
                int
            #endif
                ErrCode);

//------------------------------------------------------------------------------

void SubfoldersList(QStringList* pList,
                    const QString& Dir,
                    int Depth = -1,
                    bool FullPath = true);
QStringList SubfoldersList(const QString& Dir,
                           int Depth = -1,
                           bool FullPath = true);

//------------------------------------------------------------------------------
//! Параметры статистической информации о файлах/каталогах.
enum TFileStatOption {
    fsoTime = 0x01,  //!< Дата/время.
    fsoAttr = 0x02   //!< Атрибуты.
};

typedef QFlags<TFileStatOption> TFileStatOptions;
Q_DECLARE_OPERATORS_FOR_FLAGS(TFileStatOptions)

//! Статистическая информация о файлах/каталогах.
struct TFileStat {
    TFileStatOptions Options;
    #ifdef Q_OS_WIN
        FILETIME CreationTime,    //!< Время создания.
                 LastAccessTime,  //!< Время последнего доступа.
                 LastWriteTime;   //!< Время последнего изменения.
        DWORD    Attr;            //!< Атрибуты файла.
    #else
        time_t AccessTime,        //!< Время последнего доступа.
               ModificationTime;  //!< Время последнего изменения.
        mode_t Mode;              //!< Права доступа.
    #endif
};

void GetFileStat(const QString& FileName, TFileStat* pFileStat);
TFileStatOptions SetFileStat(const QString& FileName, const TFileStat* pFileStat);

//------------------------------------------------------------------------------

#endif // __COMMONFN__HPP__
