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

#include "ErrorHandler.hpp"

#include "Core/Task/TaskStatus.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                     T E r r o r H a n d l e r
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TErrorHandler::TErrorHandler(QObject *Parent)
    : QObject(Parent)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TErrorHandler::~TErrorHandler()
{

}

//------------------------------------------------------------------------------
//! Текст сообщения об ошибке для указанной ошибки.

QString TErrorHandler::errorText(TErrorCode Code)
{
    switch (Code)
    {
        case ecNoError :
            qWarning("TErrorHandler::errorText. "
                     "Requested error text for ecNoError.");
            return "";

        case ecBadSrc :
            return tr("Bad source.");

        case ecOpenFile :
            return tr("Error opening file.");

        case ecReadFile :
            return tr("Error reading file.");

        case ecMakeDir :
            return tr("Error creating folder.");

        case ecAlreadyExists :
            return tr("File already exists.");

        case ecCreateFile :
            return tr("Error creating file.");

        case ecWriteFile :
            return tr("Error writing file.");

        case ecSetFileStat :
            return tr("Error set file parameters.");

        case ecSetDirStat :
            return tr("Error set folder parameters.");

        case ecNoFreeSpace :
            return tr("Not enough free space.");

        default :
            return tr("Unknown error at processing file.");
    }
}

//------------------------------------------------------------------------------
//! Список возможных действий для указанного кода ошибки.

TErrorActionSet TErrorHandler::actions(TErrorCode Code)
{
    switch (Code)
    {
        case ecNoError :
            qWarning("TErrorHandler::actions. "
                     "Requested actions for ecNoError.");
            return TErrorActionSet();

        case ecBadSrc :
        case ecOpenFile :
            return TErrorActionSet()
                   << eaRetry
                   << eaSkip
                   << eaSkipAll
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecReadFile :
            return TErrorActionSet()
                   << eaRetry
                   << eaSkip
                   << eaSkipAll
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecMakeDir :
            return TErrorActionSet()
                   << eaRetry
                   << eaSkip
                   << eaSkipAll
                   << eaCancelDest
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecAlreadyExists :
            return TErrorActionSet()
                   << eaOverwrite
                   << eaOverwriteAll
                   << eaSkip
                   << eaSkipAll
                   << eaCancelDest
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecCreateFile :
            return TErrorActionSet()
                   << eaRetry
                   << eaSkip
                   << eaSkipAll
                   << eaCancelDest
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecWriteFile :
            return TErrorActionSet()
                   << eaRetry
                   << eaIgnore
                   << eaIgnoreAll
                   << eaCancelDest
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecSetFileStat :
        case ecSetDirStat :
            return TErrorActionSet()
                   << eaRetry
                   << eaSkip
                   << eaSkipAll
                   << eaCancelDest
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        case ecNoFreeSpace :
            return TErrorActionSet()
                   << eaRetry
                   << eaIgnore
                   << eaIgnoreAll
                   << eaCancelDest
                   << eaCancelCurrentTask
                   << eaCancelAllTasks;

        default :
            qWarning("TErrorHandler::actions. "
                     "Requested actions for uncnown error code (%i).",
                     Code);
            return TErrorActionSet();

    }
}

//------------------------------------------------------------------------------
//! Слот-обработчик ошибок.

void TErrorHandler::errorReceiver(TErrorData* pErrorData)
{
    Q_ASSERT(receivers(SIGNAL(errorProcessed(TErrorData*))) > 0);

    // Построение множества возможных действий.
    m_Actions = actions(pErrorData->ErrorCode);
    if (pErrorData->DestsCount < 2)
        m_Actions -= eaCancelDest;
    if (pErrorData->TasksCount < 2)
        m_Actions -= eaCancelAllTasks;
    if (pErrorData->ErrorCode == ecNoFreeSpace && pErrorData->DestsCount < 2)
        m_Actions -= eaIgnoreAll;
    Q_ASSERT(!m_Actions.isEmpty());


    // Запрос действия у пользователя.
    userPrompt(pErrorData);
    emit errorProcessed(pErrorData);
}

//------------------------------------------------------------------------------

