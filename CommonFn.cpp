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

#ifdef Q_OS_WIN

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

//! Преобразование QString в LPWSTR.
/*!
 * Создаёт массив элементов типа WCHAR, заканчивающийся нулём. Использует
 * минимально необходимый объём памяти. Динамически создаёт массив требуемого
 * размера и возвращает указатель на него. Вызывающая процедура должна
 * сама его разрушить вызовом оператора delete. Если при выделении памяти
 * возникла ошибка, возвращает NULL.
 */

LPWSTR StrToLPWSTR(const QString& Str)
{
    LPWSTR Result = new(std::nothrow) WCHAR[(Str.length() + 1)];
    if (Result != NULL)
        Result[Str.toWCharArray(Result)] = 0;
    return Result;
}

//------------------------------------------------------------------------------
//! Строковое описание системной ошибки.

QString GetSystemErrorString(DWORD ErrCode)
{
    WCHAR* errStr = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  ErrCode,
                  MAKELANGID(LANG_USER_DEFAULT, SUBLANG_DEFAULT),
                  errStr,
                  0,
                  NULL);
    QString Result = QString::fromWCharArray(errStr);
    LocalFree(errStr);
    return Result;
}

//------------------------------------------------------------------------------
//! Строковое описание последней системной ошибки.

QString GetSystemErrorString()
{
    return GetSystemErrorString(GetLastError());
}

//------------------------------------------------------------------------------

#endif //Q_OS_WIN
