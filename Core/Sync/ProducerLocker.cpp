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

#include "ProducerLocker.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                    T P r o d u c e r L o c k e r
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Конструктор.

TProducerLocker::TProducerLocker()
    : m_N(0)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TProducerLocker::~TProducerLocker()
{
}

//------------------------------------------------------------------------------
//! Инициализация блокировщика на n позиций.
/*!
   \sa release, wait.
 */

void TProducerLocker::init(int n)
{
    m_N = n;
    m_Released.clear();
    m_Semaphore.init(0);
}

//------------------------------------------------------------------------------
//! Освобождение n позиций.
/*!
   \sa init, wait.
 */

void TProducerLocker::release(void* pConsumer)
{
    if (m_Released.contains(pConsumer)) {
        qWarning("TProducerLocker::release. Consumer %p already released. "
                 "Ignored.", pConsumer);
    }
    else
        m_Semaphore.release();
}

//------------------------------------------------------------------------------
//! Ожидание освобождения всех позиций.
/*!
   \sa init, release.
 */

void TProducerLocker::wait()
{
    m_Semaphore.acquire(m_N);
    m_N = 0;
}

//------------------------------------------------------------------------------
//! Принудительная разблокировка.
/*!
   Метод принудительной разблокировки блокировщика. После его вызова и до
   следующего вызова \c init метод \c isUnlocked будет возвращать true.

   \remarks После unlock перед дальнейшим использованием обязательно вызывайте
     init.
 */

void TProducerLocker::unlock()
{
    m_N = 0;
    m_Semaphore.unlock();
}

//------------------------------------------------------------------------------
//! Признак принудительной разблокировки.
/*!
   \sa unlock
 */

bool TProducerLocker::isUnlocked() const
{
    return m_Semaphore.isUnlocked();
}

//------------------------------------------------------------------------------
//! Список блокирующих потребителей, завершивших работу.

QList<void*> TProducerLocker::releasedList() const
{
    return m_Released;
}

//------------------------------------------------------------------------------
//! Количество блокирующих потребителей, завершивших работу.

int TProducerLocker::releasedCount() const
{
    return m_Released.count();
}

//------------------------------------------------------------------------------
//! Проверка, завершил ли потребитель работу.

bool TProducerLocker::isReleased(void* pConsumer) const
{
    return m_Released.contains(pConsumer);
}

//------------------------------------------------------------------------------
