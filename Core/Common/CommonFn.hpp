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

#ifndef __COMMONFN__HPP__
#define __COMMONFN__HPP__

#include <QString>
#include <QStringList>
#include <QFileInfo>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif


//------------------------------------------------------------------------------

void AddWithSeparator(QString* Initial, const QString& Added);
QString AddWithSeparator(const QString& Initial, const QString& Added);
QString PathToLongWinPath(const QString& Path);
bool isNetworkPath(const QString& Path);

//------------------------------------------------------------------------------

void* GetLockedMem(size_t Size, bool* Lock = NULL);
void FreeLockedMem(void* Pointer);
qint64 AvailablePhysicalMemory();

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

QString speedToStr(qint64 BytesPerSec, int Digits = 4);
QString sizeToStr(qint64 Bytes, int Digits = 4);

//------------------------------------------------------------------------------

QString GetDriveByPath(const QString& Path);
qint64 DiskFreeSpace(const QString& Path);

//------------------------------------------------------------------------------

#endif // __COMMONFN__HPP__
