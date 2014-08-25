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

#include "AppInstances.hpp"

#include <QApplication>
#include <QWidget>

//------------------------------------------------------------------------------
//! Метод инициализации.

void TAppInstances::init()
{
    connect(this, SIGNAL(messageReceived(QString)), SLOT(activateWindow()));
}

//------------------------------------------------------------------------------
//! Конструктор.
/*!
   \sa TCoreAppInstances
 */

TAppInstances::TAppInstances(const QString& AppId, QObject* Parent)
    : TCoreAppInstances(AppId, Parent),
      m_pActivationWidget(NULL),
      m_ActivateOnMessage(true)
{
    init();
}

//------------------------------------------------------------------------------
//! Конструктор.
/*!
   \sa TCoreAppInstances
 */

TAppInstances::TAppInstances(QObject* Parent)
    : TCoreAppInstances(Parent),
      m_pActivationWidget(NULL),
      m_ActivateOnMessage(true)
{
    init();
}

//------------------------------------------------------------------------------
//! Деструктор.

TAppInstances::~TAppInstances()
{
}


//------------------------------------------------------------------------------
//! Активация окна первого запущенного экземпляра приложения.
/*!
   Метод посылает пустое сообщение первому запущенному экземпляру приложения.
   Если у первого экземпляра установлен флаг \c activateOnMessage, экземпляр
   помещает окно, указанное в вызове метода \c setActivationWindow, на
   передний план.

   \remarks Вызов метода из первого запущенного экземпляра приложения
     игнорируется.
 */

bool TAppInstances::activateFirst()
{
    if (isFirst()) {
        qWarning("TAppInstances::activateFirst called from first instance.");
        return false;
    }

    return sendMessageToFirst();
}

//------------------------------------------------------------------------------
//! Активация окна приложения.

void TAppInstances::activateWindow()
{
    QWidget* pWindow = m_pActivationWidget;
    if (pWindow == NULL) {
        QWidgetList Windows = QApplication::topLevelWidgets();
        for (int i = 0; i < Windows.count(); ++i)
            if (!Windows[i]->isHidden()) {
                pWindow = Windows[i];
                break;
            }
    }
    if (pWindow) {
        pWindow->setWindowState(pWindow->windowState() & ~Qt::WindowMinimized);
        pWindow->raise();
        pWindow->activateWindow();
    }
}

//------------------------------------------------------------------------------
