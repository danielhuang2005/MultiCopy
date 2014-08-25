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

#include "GlobalStatistics.hpp"

#include <QSettings>

//------------------------------------------------------------------------------

#define WRITE(Field) \
    pSettings->setValue(#Field, Field);

#define READ(Field, Type) \
    Field = pSettings->value(#Field, Field).value<Type>();

//------------------------------------------------------------------------------
//! Имя группы в настройках по умолчанию.

const QString TGlobalStatistics::DefaultGroup = "GlobalStatistics";

//------------------------------------------------------------------------------
//! Конструктор.

TGlobalStatistics::TGlobalStatistics()
    : BytesReaded(0), BytesWrited(0),
      FilesReaded(0), FilesWrited(0),
      TasksCompleted(0)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TGlobalStatistics::~TGlobalStatistics()
{
}

//------------------------------------------------------------------------------
//! Указатель на экземпляр объекта.

TGlobalStatistics* TGlobalStatistics::instance()
{
    static TGlobalStatistics Instance;
    return &Instance;
}

//------------------------------------------------------------------------------
//! Чтение настроек.
/*!
   \arg pSettings Указатель на экземпляр класса настроек.
   \arg Group     Имя группы для чтения настроек.
 */

void TGlobalStatistics::read(QSettings* pSettings, QString Group)
{
    if (Group.isEmpty())
        Group = DefaultGroup;

    pSettings->beginGroup(Group);
    READ(BytesReaded,    qint64);
    READ(BytesWrited,    qint64);
    READ(FilesReaded,    qint64);
    READ(FilesWrited,    qint64);
    READ(TasksCompleted, qint64);
    pSettings->endGroup();
}

//------------------------------------------------------------------------------
//! Запись настроек.
/*!
   \arg pSettings Указатель на экземпляр класса настроек.
   \arg Group     Имя группы для сохранения настроек.
 */

void TGlobalStatistics::write(QSettings* pSettings, QString Group)
{
    if (Group.isEmpty())
        Group = DefaultGroup;

    pSettings->beginGroup(Group);
    WRITE(BytesReaded);
    WRITE(BytesWrited);
    WRITE(FilesReaded);
    WRITE(FilesWrited);
    WRITE(TasksCompleted);
    pSettings->endGroup();
}

//------------------------------------------------------------------------------
