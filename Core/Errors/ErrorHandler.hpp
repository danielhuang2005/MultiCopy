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

#ifndef __ERRORHANDLER__HPP__
#define __ERRORHANDLER__HPP__

//------------------------------------------------------------------------------

#include <QString>
#include <QMap>
#include <QMutex>

#include "ErrorsAndActions.hpp"

//------------------------------------------------------------------------------
//! базовый класс обработчика ошибок.

class TErrorHandler : public QObject
{
    Q_OBJECT
    protected :
        //! Диалоговый обработчик ошибок.
        /*!
           Этот метод должен быть определён в потомке класса. Назначение
           метода - вывод запроса пользователю о возможных действиях.
           Метод вызывается только в случае необходимости. Например, если
           пользователь в определённой ситуации выбрал вариант ответа,
           предполагающий повтор выбранного действия ("пропустить всё",
           "игнорировать всё" и т.п.), то метод вызван не будет.
         */
        virtual TErrorAction userPrompt(const TErrorData* pErrorData) = 0;

        TErrorActionSet m_Actions;

    public:
        TErrorHandler(QObject* Parent = NULL);
        virtual ~TErrorHandler();

        static QString errorText(TErrorCode Code);
        static TErrorActionSet actions(TErrorCode Code);

    signals :
        //! Сигнал завершения обработки ошибки.
        /*!
           Сигнал генерируется после завершения обработки ошибки. В поле Action
           структуры pErrorData возвращается выбранное пользователем
           (или автоматически) действие.
         */
        void errorProcessed(TErrorData* pErrorData);

    public slots :
        void errorReceiver(TErrorData* pErrorData);
};

//------------------------------------------------------------------------------

#endif // __ERRORHANDLER__HPP__
