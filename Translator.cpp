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

#include "Translator.hpp"

#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>

#include "MultiCopyForm.hpp"
#include "SettingsForm.hpp"
#include "ProgressForm.hpp"

//------------------------------------------------------------------------------

QStringList langPaths()
{
    QStringList Result;
    QString AppPath = QCoreApplication::applicationDirPath();
    Result << AppPath + "/lang"
           << AppPath;
    return Result;
}

//------------------------------------------------------------------------------

void loadTranslators(const QString& LangID)
{
    static QTranslator qtTranslator;   // Общий переводчик Qt.
    static QTranslator appTranslator;  // Переводчик приложения.
    QStringList LangPath;       // Пути к файлам перевода.
    LangPath << langPaths()
             << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    for (int i = 0; i < LangPath.count(); ++i)
    {
        if (qtTranslator.load("qt_" + LangID, LangPath[i]))
        {
            qApp->installTranslator(&qtTranslator);
            break;
        }
    }
    for (int i = 0; i < LangPath.count(); ++i)
    {
        if (appTranslator.load(qApp->applicationName() + "." + LangID, LangPath[i]))
        {
            qApp->installTranslator(&appTranslator);
            break;
        }
    }

}

//------------------------------------------------------------------------------
