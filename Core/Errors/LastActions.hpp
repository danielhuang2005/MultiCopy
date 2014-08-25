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

#ifndef __LASTACTIONS__HPP__
#define __LASTACTIONS__HPP__

//------------------------------------------------------------------------------

#include <QMap>

#include "ErrorsAndActions.hpp"

//------------------------------------------------------------------------------
//! Класс для хранения последних выбранных действий при ошибках.

class TLastActions
{
    private :
        //! Список последних выбранных действий.
        typedef QMap<TErrorCode, TErrorAction> TLastActionsMap;

        TLastActionsMap m_LastActionsMap;  //!< Последние выбранные действия.
        TErrorAction    m_LastAction;      //!< Последнее выбранное действие.

    public:
        TLastActions();

        TErrorAction lastAction(TErrorCode ErrorCode) const;
        void addAction(TErrorCode ErrorCode, TErrorAction Action);
        void clear();

        //! Последнее выбранное действие.
        inline TErrorAction lastAction() const { return m_LastAction; }
};

//------------------------------------------------------------------------------
#endif // __LASTACTIONS__HPP__
