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

#ifndef __SEMAPHOREEX__HPP__E1E8D9A3_2274_4393_98E0_FD1F4AE26A61__
#define __SEMAPHOREEX__HPP__E1E8D9A3_2274_4393_98E0_FD1F4AE26A61__

//------------------------------------------------------------------------------

#include <QMutex>
#include <QWaitCondition>

//------------------------------------------------------------------------------
//! Семафор с расширенной функциональностью.
/*!
   Класс семафора с расширенной функциональностью во многом аналогичен классу
   QSemaphore. В принципе, этот класс можно реализовать как простую надстройку
   над QSemaphore, но тот сам реализован на QMutex и QWaitCondition, поэтому для
   увеличения быстродействия класс реализован заново.
 */

class TSemaphoreEx
{
    private :
        mutable QMutex m_Mutex;          //!< Мьютекс.
        QWaitCondition m_WaitCondition;  //!< Wait condition.
        int            m_Available;      //!< Счётчик семафора.
        int m_Acquired;  //!< Последнее число позиций, затребованное для
                         //!< захвата. Отрицательное значение - признак
                         //!< принудительной разблокировки.
    public:
        TSemaphoreEx(int n = 0);
        ~TSemaphoreEx();

        void acquire(int n = 1);
        void release(int n = 1);
        int  available() const;
        void init(int n);
        void unlock();
        bool isUnlocked() const;
};

//------------------------------------------------------------------------------

#endif // __SEMAPHOREEX__HPP__E1E8D9A3_2274_4393_98E0_FD1F4AE26A61__
