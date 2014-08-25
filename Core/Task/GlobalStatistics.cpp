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
#include <QCoreApplication>

#include "../Common/SharedMemory.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//         T G l o b a l S t a t i s t i c s : : T S t a t
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Очистка.

void TGlobalStatistics::TStat::clear()
{
    BytesReaded = 0;     BytesWrited = 0;
    FilesReaded = 0;     FilesWrited = 0;
    TasksCompleted = 0;
}

//------------------------------------------------------------------------------
//! Конструктор.

TGlobalStatistics::TStat::TStat()
{
    clear();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//        T G l o b a l S t a t i s t i c s : : T S t a t 2
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Очистка.

void TGlobalStatistics::TStat2::clear()
{
    TStat::clear();
    Readed = false;
}

//------------------------------------------------------------------------------
//! Конструктор.

TGlobalStatistics::TStat2::TStat2()
{
    clear();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                T G l o b a l S t a t i s t i c s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define WRITE(Field) \
    pSettings->setValue(#Field, m_pStat2->Field);

#define READ(Field, Type) \
    m_pStat2->Field = pSettings->value(#Field, m_pStat2->Field).value<Type>();

//------------------------------------------------------------------------------
//! Имя группы в настройках по умолчанию.

const QString TGlobalStatistics::DefaultGroup = QLatin1String("GlobalStatistics");

//------------------------------------------------------------------------------
//! Конструктор.

TGlobalStatistics::TGlobalStatistics()
    : m_pSharedMemory(NULL),
      m_pStat2(NULL)
{
    QString AppId = QCoreApplication::applicationFilePath();
    #ifdef Q_OS_WIN
        AppId = AppId.toLower();
    #endif
    AppId = QLatin1String("MultiCopy-GlobalStatistics-") + AppId.toUtf8().toHex();
    m_pSharedMemory = new TSharedMemory<TStat2>(AppId);
    m_pStat2 = m_pSharedMemory->data();
}

//------------------------------------------------------------------------------
//! Деструктор.

TGlobalStatistics::~TGlobalStatistics()
{
    delete m_pSharedMemory;
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
    TSharedMemoryLocker Locker(m_pSharedMemory);
    Q_UNUSED(Locker);

    if (m_pStat2 != NULL)
    {
        if (!m_pStat2->Readed)
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

            m_pStat2->Readed = true;
        }
        else {
            qWarning("TGlobalStatistics::read. Data already readed.");
        }

    }
    else {
        qWarning("TGlobalStatistics::read. Data pointer is NULL.");
    }
}

//------------------------------------------------------------------------------
//! Запись настроек.
/*!
   \arg pSettings Указатель на экземпляр класса настроек.
   \arg Group     Имя группы для сохранения настроек.
 */

void TGlobalStatistics::write(QSettings* pSettings, QString Group)
{
    TSharedMemoryLocker Locker(m_pSharedMemory);
    Q_UNUSED(Locker);

    if (m_pStat2 != NULL)
    {
        if (m_pStat2->Readed)
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
        else {
            qWarning("TGlobalStatistics::write. Data not readed.");
        }
    }
    else {
        qWarning("TGlobalStatistics::write. Data pointer is NULL.");
    }
}

//------------------------------------------------------------------------------
//! Добавление статистики.

void TGlobalStatistics::append(const TStat& Stat)
{
    TSharedMemoryLocker Locker(m_pSharedMemory);
    Q_UNUSED(Locker);

    if (m_pStat2 != NULL)
    {
        if (m_pStat2->Readed)
        {
            m_pStat2->BytesReaded += Stat.BytesReaded;
            m_pStat2->BytesWrited += Stat.BytesWrited;
            m_pStat2->FilesReaded += Stat.FilesReaded;
            m_pStat2->FilesWrited += Stat.FilesWrited;
            m_pStat2->TasksCompleted += Stat.TasksCompleted;
        }
        else {
            qWarning("TGlobalStatistics::append. Data not readed.");
        }
    }
    else {
        qWarning("TGlobalStatistics::append. Data pointer is NULL.");
    }
}

//------------------------------------------------------------------------------
//! Получение статистики.

TGlobalStatistics::TStat TGlobalStatistics::get() const
{
    TSharedMemoryLocker Locker(m_pSharedMemory);
    Q_UNUSED(Locker);

    TStat Result;

    if (m_pStat2 != NULL)
        Result = *m_pStat2;
    else
        qWarning("TGlobalStatistics::get. Data pointer is NULL.");

    return Result;
}

//------------------------------------------------------------------------------
