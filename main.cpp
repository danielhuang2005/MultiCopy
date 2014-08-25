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

#include <QtGui/QApplication>
#include <QTextCodec>

#include "Core/Task/GlobalStatistics.hpp"
#include "Core/AppInstances/AppInstances.hpp"
#include "Core/Resources.hpp"
#include "GUI/Forms/MultiCopyForm.hpp"
#include "GUI/Translator.hpp"
#include "GUI/Settings.hpp"

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("KrugloffYV");
    a.setApplicationName("MultiCopy");
    int PointerSize = sizeof(void*);
    if (PointerSize != 4)
        a.setApplicationName(a.applicationName() +
                             QString(" (x%1)").arg(PointerSize * 8));

    #ifdef Q_OS_WIN
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    #endif

    TSettings* pSettings = TSettings::instance();

    TAppInstances AppInstances(baseAppName());
    if (pSettings->GeneralSettings.SingleInstance)
    {
        if (AppInstances.isRunning())
        {
            AppInstances.activateFirst();
            qWarning("Another instanse of %s is running. Exiting.",
                     qPrintable(baseAppName()));
            return 0;
        }
    }


    // Языковые настройки.
    loadTranslators(pSettings->langID());
    // Статистика работы.
    TGlobalStatistics::instance()->read(pSettings->getQSettings());

    if (AppInstances.index() != 0)
        a.setApplicationName(QString("[%1] ").arg(AppInstances.index() + 1) +
                             a.applicationName());
    TMultiCopy MainForm;
    AppInstances.setActivationWindow(&MainForm);
    AppInstances.setActivateOnMessage(true);

    MainForm.show();
    QApplication::processEvents();
    MainForm.loadListsFromSettings();

    return a.exec();
}

//------------------------------------------------------------------------------
