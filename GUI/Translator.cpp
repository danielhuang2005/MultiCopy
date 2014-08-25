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

#include "Translator.hpp"

#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>

//------------------------------------------------------------------------------
//! Построение списка каталогов для поиска языковых файлов.

QStringList langPaths()
{
    QStringList Result;
    QString AppPath = QCoreApplication::applicationDirPath();
    Result << AppPath + "/lang"
           << AppPath
           << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    return Result;
}

//------------------------------------------------------------------------------
//! Загрузка языковых файлов с указанным идентификатором языка.

void loadTranslators(QString LangId)
{
    static QTranslator qtTranslator;   // Общий переводчик Qt.
    static QTranslator appTranslator;  // Переводчик приложения.
    static QStringList LangPath = langPaths();  // Пути к файлам перевода.
    static QString CurrentLangId;  // Текущий идентификатор языка.

    if (LangId.isEmpty())
        LangId = QLocale::system().name();

    if (LangId == CurrentLangId) {
        return;
    }
    CurrentLangId = LangId;

    for (int i = 0; i < LangPath.count(); ++i)
    {
        if (qtTranslator.load("qt_" + LangId, LangPath[i]))
        {
            QCoreApplication::installTranslator(&qtTranslator);
            break;
        }
    }
    for (int i = 0; i < LangPath.count(); ++i)
    {
        if (appTranslator.load(qApp->applicationName() + "." + LangId,
                               LangPath[i]))
        {
            QCoreApplication::installTranslator(&appTranslator);
            break;
        }
    }
}

//------------------------------------------------------------------------------
