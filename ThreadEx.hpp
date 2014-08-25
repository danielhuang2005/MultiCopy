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

#ifndef __THREADEX__HPP__
#define __THREADEX__HPP__

//------------------------------------------------------------------------------

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

//------------------------------------------------------------------------------
//! Потомок класса QThread с расширенной функциональностью.
/*!
 * \remarks Все функции класса потокобезопасны (thread-safe).
 */

class TThreadEx : public QThread
{
    private :
        mutable QMutex m_PauseMutex; //!< Мьютекс для приостановки потока.
        QWaitCondition m_PauseCondition;
        QAtomicInt     m_Paused;     //!< Флаг приостановки потока.
    protected :
        void pausePoint();
    public :
        explicit TThreadEx(QObject* Parent = NULL);
        virtual ~TThreadEx();

        void pause();
        void resume();
        bool isPaused() const;

        //! То же, что и pause.
        inline void suspend() { pause(); }
        //! То же, что и isPaused.
        inline bool isSuspended() const { return isPaused(); }
};

//------------------------------------------------------------------------------

#endif // THREADEX_HPP
