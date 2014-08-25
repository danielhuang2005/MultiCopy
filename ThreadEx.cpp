/*******************************************************************************
 *
 *            Copyright (С) 2011 Юрий Владимирович Круглов
 *
 * Эта программа является свободным программным обеспечением. Вы можете
 * распространять и/или  модифицировать её согласно условиям Стандартной
 * Общественной Лицензии GNU, опубликованной Организацией Свободного
 * Программного Обеспечения, версии 3, либо по Вашему желанию, любой более
 * поздней версии.
 *
 * Эта программа распространяется в надежде на то, что окажется полезной, но
 * БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, даже без подразумеваемых гарантий ОКУПАЕМОСТИ или
 * СООТВЕТСТВИЯ КОНКРЕТНЫМ ЦЕЛЯМ.
 * Подробнее - см. Стандартной Общественную Лицензию GNU.
 *
 * Вы должны были получить копию Основной Общественной Лицензии GNU вместе с
 * этой программой. При её отсутствии обратитесь на сайт
 * http://www.gnu.org/licenses/.
 *
 *******************************************************************************
 *
 *                 Copyright (C) 2011 Yuri V. Krugloff
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "ThreadEx.hpp"

//------------------------------------------------------------------------------
//! Конструктор.

TThreadEx::TThreadEx(QObject* Parent)
    : QThread(Parent), m_Paused(false)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TThreadEx::~TThreadEx()
{
}

//------------------------------------------------------------------------------
//! "Точка паузы".
/*!
 * Поток должен вызвать эту функцию в той точке, где он может быть приостановлен
 * на неопределённое время.
 *
 * \sa pause, resume
 */

void TThreadEx::pausePoint()
{
    m_PauseMutex.lock();
    if (m_Paused)
        m_PauseCondition.wait(&m_PauseMutex);
    m_PauseMutex.unlock();
}

//------------------------------------------------------------------------------
//! Приостановка выполнения потока.
/*!
 * Устанавливает флаг приостановки потока. Действие этого метода не мгновенно,
 * поток может быть приостановлен только в тех точках, где он сам вызывает
 * метод pausePoint.
 *
 * \sa resume, pausePoint
 */

void TThreadEx::pause()
{
    m_PauseMutex.lock();
    m_Paused = true;
    m_PauseMutex.unlock();
}

//------------------------------------------------------------------------------
//! Возобновление выполнения потока.
/*!
 * Снимает флаг приостановки потока и запускет его.
 */

void TThreadEx::resume()
{
    m_PauseMutex.lock();
    m_Paused = false;
    m_PauseCondition.wakeAll();
    m_PauseMutex.unlock();
}

//------------------------------------------------------------------------------
/*!
 * Возвращает true, если выполнение заданий приостановлено и false в противном
 * случае.
 */

bool TThreadEx::isPaused() const
{
    QMutexLocker Locker(&m_PauseMutex);
    Q_UNUSED(Locker);

    return m_Paused;
}

//------------------------------------------------------------------------------
