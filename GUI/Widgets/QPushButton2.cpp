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

#include "QPushButton2.hpp"

//------------------------------------------------------------------------------
//! Установка текста на кнопке и во всплывающей подсказке.

void QPushButton2::setTextAndToolTip()
{
    switch (m_Action)
    {
        case eaOverwrite :
            setText(tr("&Overwrite"));
            setToolTip(tr("Overvrite current file"));
            break;
        case eaOverwriteAll :
            setText(tr("O&verwrite All"));
            setToolTip(tr("Overwrite all files without question"));
            break;
        case eaRetry :
            setText(tr("&Retry"));
            setToolTip(tr("Retry operation"));
            break;
        case eaIgnore :
            setText(tr("&Ignore"));
            setToolTip(tr("Ignore this warning and continue"));
            break;
        case eaIgnoreAll :
            setText(tr("I&gnore All"));
            setToolTip(tr("Ignore all warnings of such type and continue"));
            break;
        case eaSkip :
            setText(tr("&Skip"));
            setToolTip(tr("Skip current file"));
            break;
        case eaSkipAll :
            setText(tr("Skip &All"));
            setToolTip(tr("Skip all files without questions"));
            break;
        case eaCancelDest :
            setText(tr("Cancel &Dest"));
            setToolTip(tr("Cancel copying to this destination"));
            break;
        case eaCancelCurrentTask :
            setText(tr("&Cancel"));
            setToolTip(tr("Cancel task"));
            break;
        case eaCancelAllTasks :
            setText(tr("Cancel A&ll"));
            setToolTip(tr("Cancel all tasks"));
            break;
        default :
            qWarning("QPushButton2::setTextAndToolTip. Unknown action (%i).",
                     m_Action);
    }
}

//------------------------------------------------------------------------------
//! Конструктор.

QPushButton2::QPushButton2(TErrorAction Action, QWidget *Parent)
    : QPushButton(Parent),
      m_Action(Action)
{
    setTextAndToolTip();
}

//------------------------------------------------------------------------------
//! Деструктор.

QPushButton2::~QPushButton2()
{
}

//------------------------------------------------------------------------------

