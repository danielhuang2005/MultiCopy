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

#include "SemaphoreEx.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                      T S e m a p h o r e E x
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Конструктор семафора с начальным значением счётчика n.

TSemaphoreEx::TSemaphoreEx(int n)
    : m_Available(n), m_Acquired(0)
{
    if (m_Available < 0) {
        m_Available = 0;
        qWarning("TSemaphoreEx::TSemaphoreEx. "
                 "Parameter must be non-negative. Reduced to zero.");
    }
}

//------------------------------------------------------------------------------
//! Деструктор.

TSemaphoreEx::~TSemaphoreEx()
{
}

//------------------------------------------------------------------------------
//! Захват n позиций семафора.

void TSemaphoreEx::acquire(int n)
{
    if (n < 0) {
        qWarning("TSemaphoreEx::acquire. "
                 "Parameter must be non-negative. Ignored.");
        return;
    }

    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_Acquired = n;

    while (n > m_Available)
        m_WaitCondition.wait(&m_Mutex);
    m_Available -= n;

    if (!isUnlocked()) m_Acquired = 0;
}

//------------------------------------------------------------------------------
//! Освобождение n позиций семафора.

void TSemaphoreEx::release(int n)
{
    if (n < 0) {
        qWarning("TSemaphoreEx::release. "
                 "Parameter must be non-negative. Ignored.");
        return;
    }

    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_Available += n;
    m_WaitCondition.wakeAll();
}

//------------------------------------------------------------------------------
//! Число свободных позиций семафора.

int TSemaphoreEx::available() const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_Available;
}

//------------------------------------------------------------------------------
//! (Ре)инициализация семафора на значение счётчика n.

void TSemaphoreEx::init(int n)
{
    if (n < 0) {
        qWarning("TSemaphoreEx::init. "
                 "Parameter must be non-negative. Reduced to zero.");
        n = 0;
    }

    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_Available = n;
    m_Acquired  = 0;
    m_WaitCondition.wakeAll();
}

//------------------------------------------------------------------------------
//! Разблокировка семафора.
/*!
   Принудительная разблокировка семафора. Устанавливает счётчик семафора в
   последнее затребованное для захвата методом \c acquire число. После вызова
   этого метода \c isUnlocked вернёт true. Перед дальнейшей работой с семафором
   обязательно вызовите init, в противном случае поведение семафора может быть
   непередсказуемым.

   \remarks Данный метод нестандартен. Используйте его только если точно
     уверены, что делаете.
 */

void TSemaphoreEx::unlock()
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    m_Available = m_Acquired;
    m_Acquired  = -1;
}

//------------------------------------------------------------------------------
//! Признак принудительной разблокировки семафора.
/*!
   \sa unlock
 */

bool TSemaphoreEx::isUnlocked() const
{
    return m_Available < 0;
}

//------------------------------------------------------------------------------
