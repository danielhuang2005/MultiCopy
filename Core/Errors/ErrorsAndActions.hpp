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

#ifndef __ERRORSANDACTIONS__HPP__
#define __ERRORSANDACTIONS__HPP__

#include <QSet>
#include <QString>
#include <QSemaphore>

//------------------------------------------------------------------------------
//! Возможные действия после ошибки.

enum TErrorAction {
    eaNoAction,          //!< Нет действия (действие не определено).
    eaOverwrite,         //!< Перезаписать файл.
    eaOverwriteAll,      //!< Перезаписать все файлы.
    eaRetry,             //!< Повторить.
    eaIgnore,            //!< Игнорировать и продолжить.
    eaIgnoreAll,         //!< Игнорировать всё и продолжать.
    eaSkip,              //!< Пропустить.
    eaSkipAll,           //!< Пропустить всё.
    eaCancelDest,        //!< Отменить копирование в данное назначение.
    eaCancelCurrentTask, //!< Отменить текущее задание.
    eaCancelAllTasks     //!< Отменить все задания.
};

typedef QSet<TErrorAction> TErrorActionSet;

//------------------------------------------------------------------------------
//! Типы ошибок.

enum TErrorCode {
    ecNoError,       //!< Нет ошибок.
    ecBadSrc,        //!< Источник неправильный.
    ecOpenFile,      //!< Ошибка при открытии файла (для чтения).
    ecReadFile,      //!< Ошибка при чтении файла.
    ecMakeDir,       //!< Ошибка при создании каталога.
    ecAlreadyExists, //!< Файл уже существует
    ecCreateFile,    //!< Ошибка при создании файла.
    ecWriteFile,     //!< Ошибка при записи файла.
    ecSetFileStat,   //!< Ошибка установки параметров файла.
    ecSetDirStat,    //!< Ошибка установки параметров каталога.
    ecNoFreeSpace    //!< Недостаточно свободного места.
};

//------------------------------------------------------------------------------
//! Структура с информацией об ошибке.

struct TErrorData
{
    TErrorCode   ErrorCode;   //!< Тип (код) ошибки.
    TErrorAction Action;      //!< Выбранное действие.
    int          DestsCount;  //!< Количество оставшихся назначений.
    int          TasksCount;  //!< Количество оставшихся задач.
    QString      Message;     //!< Сообщение системы об ошибке.
    QString      FileName;    //!< Имя файла (каталога), обработка которого
                              //!< вызвала ошибку.
    QObject*     pSender;     //!< Указатель на источник ошибки.
    QSemaphore*  pLocker;     //!< Блокировщик вызываемого потока.

    TErrorData()
        : ErrorCode(ecNoError), Action(eaNoAction),
          DestsCount(-1),       TasksCount(-1),
          pSender(NULL),        pLocker(NULL)
    { }
};

//------------------------------------------------------------------------------

#endif // __ERRORSANDACTIONS__HPP__
