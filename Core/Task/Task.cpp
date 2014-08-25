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

#include "Task.hpp"

#include <QMetaType>

Q_DECLARE_METATYPE(TTaskSize)
Q_DECLARE_METATYPE(TTaskList)

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                         T D i r S i z e
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TTaskSize::TTaskSize()
{
    static int id = qRegisterMetaType<TTaskSize>("TTaskSize");
    Q_UNUSED(id);
    clear();
}

//------------------------------------------------------------------------------
//! Очистка.

void TTaskSize::clear()
{
    FilesCount = 0;
    DirsCount  = 0;
    TotalSize  = 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                     T T a s k S e t t i n g s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TTaskSettings::TTaskSettings()
{
    setDefault();
}

//------------------------------------------------------------------------------
//! Сброс на значения по умолчанию.

void TTaskSettings::setDefault()
{
    BufferSizeAutoselect = true;
    RAMCellSize          = 16 * 1024 * 1024;  // 16Mb
    RAMCellCount         = 4;
    LockMemory           = false;
    NoUseCache           = false;
    TotalCalc            = true;
    CheckFreeSpace       = true;
    NoCreateRootDir      = false;
    SubDirsDepth         = -1;
    CopyDateTime         = true;
    CopyAttr             = true;
    CopyHidden           = false;
    CopySystem           = false;
    FollowShortcuts      = false;
    CopyEmptyDirs        = false;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                              T T a s k
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TTask::TTask()
{
    static int id = qRegisterMetaType<TSharedConstTask>("TSharedConstTask");
    Q_UNUSED(id);
}

//------------------------------------------------------------------------------
//! Проверка на пустоту.
/*!
   \remarks Задача считается пустой, если у неё пуст либо список источников,
     либо список назначений.
 */

bool TTask::isEmpty() const
{
    return SrcList.isEmpty() || DestList.isEmpty();
}

//------------------------------------------------------------------------------
