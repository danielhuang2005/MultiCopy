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

#include "TimeCounter.hpp"

#include <QDateTime>

//------------------------------------------------------------------------------
//! Конструктор.

TTimeCounter::TTimeCounter()
    : m_StartTime(0),
      m_PauseTime(0),
      m_Started(false),
      m_Paused(false),
      m_msec(0),
      m_Pause_msec(0)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TTimeCounter::~TTimeCounter()
{
}

//------------------------------------------------------------------------------
//! Запуск таймера.
/*!
 * \remarks Повторный вызов метода перезапускает таймер.
 */

void TTimeCounter::start()
{
    m_Started = true;
    m_Paused = false;
    m_StartTime = QDateTime::currentMSecsSinceEpoch();
    m_msec = 0;
    m_Pause_msec = 0;
}

//------------------------------------------------------------------------------
//! Остановка таймера.

void TTimeCounter::stop()
{
    if (m_Started)
    {
        m_Started = false;
        m_Paused = false;
        m_msec = QDateTime::currentMSecsSinceEpoch() - m_StartTime
                                                     - m_Pause_msec;
    }
}

//------------------------------------------------------------------------------
//! Приостановка таймера.
/*!
 * \remarks Если таймер не запущен или уже приостановлен, метод ничего не
 *   делает.
 */

void TTimeCounter::pause()
{
    if (!m_Paused && m_Started)
    {
        m_Paused = true;
        m_PauseTime = QDateTime::currentMSecsSinceEpoch();
    }
}

//------------------------------------------------------------------------------
//! Возобновление работы таймера после приостановки.
/*!
 * Если таймер не запущен или не приостановлен, метод ничего не делает.
 */

void TTimeCounter::resume()
{
    if (m_Paused)
    {
        Q_ASSERT(m_Started);
        m_Paused = false;
        m_Pause_msec += QDateTime::currentMSecsSinceEpoch() - m_PauseTime;
    }
}

//------------------------------------------------------------------------------
//! Очистка счётчиков времени и остановка таймера.

void TTimeCounter::clear()
{
    m_Started = false;
    m_Paused = false;
    m_StartTime = 0;
    m_PauseTime = 0;
    m_msec = 0;
    m_Pause_msec = 0;
}

//------------------------------------------------------------------------------
//! Число миллисекунд с момента запуска таймера.
/*!
 * Метод возвращает число микросекунд, прошедшее с момента последнего запуска
 * таймера методом start до текущего момента или его остановки методом stop.
 */

qint64 TTimeCounter::msec() const
{
    if (m_Started)
    {
        if (m_Paused)
            // Запущено и на паузе.
            return m_PauseTime - m_StartTime - m_Pause_msec;
        else
            // Запущено и работает.
            return QDateTime::currentMSecsSinceEpoch() - m_StartTime
                                                       - m_Pause_msec;
    }
    else {
        // Остановлено.
        return m_msec;
    }
}

//------------------------------------------------------------------------------

