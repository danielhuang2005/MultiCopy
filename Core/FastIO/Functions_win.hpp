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

#ifndef __FUNCTIONS_WIN__HPP__
#define __FUNCTIONS_WIN__HPP__

//------------------------------------------------------------------------------

#if defined(QT_VERSION) && !defined(Q_OS_WIN)
    #error "This file may be compiled in Windows only!"
#endif

//------------------------------------------------------------------------------

#include <QStringList>

#include <windows.h>

//------------------------------------------------------------------------------

bool enumServerShares(const QString& ServerName, QStringList *const pSharesList);
bool isNetworkShareExists(const QString& ServerName, const QString& Share);
bool getFileFindData(const QString& Name, WIN32_FIND_DATAW *const pData);
bool getFileAttributesData(const QString& Name, WIN32_FILE_ATTRIBUTE_DATA *const pData);

QString resolveVolume(const QString& VolumeName);
QString resolveReparsePoint(const QString& LongWinPath);
QString resolveLnkFile(const QString& LongWinPath);

//------------------------------------------------------------------------------

#endif // __FUNCTIONS_WIN__HPP__