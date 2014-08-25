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

#include "FileDialogs.hpp"

#include <QFileDialog>

#ifdef Q_OS_WIN
    #include "FileDialogs_win.hpp"
#endif

//------------------------------------------------------------------------------
//! Получение существующих каталогов.
/*!
   Функция возвращает список существующих каталогов, выбранных пользователем.
   В отличие от QFileDialog::getExistingDirectory в Windows Vista и выше
   позволяет корректно выбрать несколько каталогов. Если ОС не Windows или
   версия Windows ниже Vista, вызывает QFileDialog::getExistingDirectory.

   \param parent  Родительский виджет.
   \param caption Заголовок окна.
   \param dir     Начальный каталог.
 */

QStringList getExistingDirectories(QWidget* parent,
                                   const QString& caption,
                                   const QString& dir)
{
    QStringList Result;

#if defined(Q_OS_WIN)
    if (getExistingDirectories_win(&Result, parent, caption, dir))
        return Result;
#endif

    // Not supported Windows versions or other OS.

    QString Dir = QFileDialog::getExistingDirectory(parent, caption, dir);
    if (!Dir.isEmpty())
        Result.append(Dir);
    return Result;
}

//------------------------------------------------------------------------------
//! Получение существующего каталога.
/*!
   Функция возвращает существующий каталог, выбранный пользователем.
   В отличие от QFileDialog::getExistingDirectory в Windows Vista и выше
   не позволяет выбирать несколько каталогов. Если ОС не Windows или
   версия Windows ниже Vista, вызывает QFileDialog::getExistingDirectory.

   \param parent  Родительский виджет.
   \param caption Заголовок окна.
   \param dir     Начальный каталог.
 */

QString getExistingDirectory(QWidget* parent,
                             const QString& caption,
                             const QString& dir)
{
#if defined(Q_OS_WIN)
    QString Result;
    if (getExistingDirectory_win(&Result, parent, caption, dir))
        return Result;
#endif

    // Not supported Windows versions or other OS.

    return QFileDialog::getExistingDirectory(parent, caption, dir);
}

//------------------------------------------------------------------------------
