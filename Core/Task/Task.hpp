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

#ifndef __TASK__HPP__
#define __TASK__HPP__

//------------------------------------------------------------------------------

#include <QStringList>
#include <QSharedPointer>

//------------------------------------------------------------------------------
//! Размер задания.

struct TTaskSize {
    qint64 FilesCount;   //!< Число файлов.
    qint64 DirsCount;    //!< Число каталогов.
    qint64 TotalSize;    //!< Суммарный размер файлов.

    TTaskSize();
    void clear();
};


//------------------------------------------------------------------------------
//! Настройки задачи копирования.

struct TTaskSettings
{
    bool   BufferSizeAutoselect;  //!< Автовыбор объёма буфера.
    int    RAMCellSize;           //!< Объём ячейки буфера (байт).
    int    RAMCellCount;          //!< Число ячеек буфера.
    bool   LockMemory;            //!< Блокировать страницы памяти.
    bool   NoUseCache;            //!< Не использовать системный кэш.
    qint64 NoUseCacheFor;         //!< Не использовать системный кэш для файлов
                                  //!< больших чем указано в этом параметре (байт).
    bool   TotalCalc;             //!< Подсчитывать размер задания.
    bool   CheckFreeSpace;        //!< Проверять свободное место.
    bool   NoCreateRootDir;       //!< Не создавать корневой каталог.
    int    SubDirsDepth;          //!< Глубина обхода подкаталогов.
    bool   CopyDateTime;          //!< Копировать дату и время.
    bool   CopyAttr;              //!< Копировать атрибуты.
    bool   CopyHidden;            //!< Копировать скрытые файлы и каталоги.
    bool   CopySystem;            //!< Копировать системные файлы и каталоги.
    bool   FollowShortcuts;       //!< Следовать по ярлыкам Windows.
    bool   CopyEmptyDirs;         //!< Копировать пустые каталоги.

    TTaskSettings();
    void setDefault();
};


//------------------------------------------------------------------------------
//! Задача копирования.

struct TTask
{
    QStringList   SrcList;       //!< Список источников.
    QStringList   DestList;      //!< Список назначений.
    TTaskSettings TaskSettings;  //!< Настройки задачи.

    TTask();
    bool isEmpty() const;
};

//! Общий указатель на задачу (константную).
typedef QSharedPointer<const TTask> TSharedConstTask;

//! Список задач копирования.
typedef QList<TSharedConstTask> TTaskList;

//------------------------------------------------------------------------------

#endif // __TASK__HPP__
