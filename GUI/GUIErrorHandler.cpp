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

#include "GUIErrorHandler.hpp"

#include <QDir>
#include <QApplication>
#include <QMessageBox>

#include "GUI/Widgets/QPushButton2.hpp"

//------------------------------------------------------------------------------
//! Диалоговый обработчик ошибок.

void TGUIErrorHandler::userPrompt(TErrorData* pErrorData)
{
    Q_ASSERT(pErrorData != NULL);

    QMessageBox Box(m_pParent);
    Box.setIcon(QMessageBox::Warning);
    Box.setWindowTitle(QApplication::applicationName());

    // Построение текста сообщения об ошибке.
    QString Msg = errorText(pErrorData->ErrorCode);
    if (!pErrorData->Message.isEmpty())
        Msg += "\n\n" + pErrorData->Message.simplified();
    if (!pErrorData->FileName.isEmpty())
        Msg += "\n\n" + QDir::toNativeSeparators(pErrorData->FileName);
    Box.setText(Msg);


    // Добавление кнопок в диалог.
    for (TErrorActionSet::const_iterator I = m_Actions.constBegin();
         I != m_Actions.constEnd(); ++I)
    {
        QPushButton2* pBtn = new QPushButton2(*I);
        Box.addButton(pBtn, QMessageBox::ActionRole);
    }

    Box.exec();
    QPushButton2* pClicked = qobject_cast<QPushButton2*>(Box.clickedButton());
    Q_ASSERT(pClicked != NULL);
    pErrorData->Action = pClicked->action();

    // Созданные кнопки разрушаются диалогом, поэтому удалять их оператором
    // delete не нужно.
}

//------------------------------------------------------------------------------
//! Конструктор.

TGUIErrorHandler::TGUIErrorHandler(QWidget* Parent)
    : TErrorHandler(Parent),
      m_pParent(Parent)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TGUIErrorHandler::~TGUIErrorHandler()
{
}

//------------------------------------------------------------------------------

