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

#ifndef __TIMECOUNTER__HPP__3FDDD5C2_1BD9_4884_A4AB_B84F6321A5B6__
#define __TIMECOUNTER__HPP__3FDDD5C2_1BD9_4884_A4AB_B84F6321A5B6__

//------------------------------------------------------------------------------

#include <QtGlobal>

//------------------------------------------------------------------------------
//! Счётчик времени с возможностью приостановки.

class TTimeCounter
{
    private :
        qint64 m_StartTime;   //!< Время старта.
        qint64 m_PauseTime;   //!< Время постановки на паузу.
        bool   m_Started;     //!< Флаг запуска.
        bool   m_Paused;      //!< Флаг приостановки.
        qint64 m_msec;        //!< Время работы.
        qint64 m_Pause_msec;  //!< Общее время приостановки.

    public:
        TTimeCounter();
        virtual ~TTimeCounter();

        void start();
        void stop();
        void pause();
        void resume();
        void clear();
        qint64 msec() const;

        //! Возвращает true, если счётчик запущен.
        inline bool isStarted() const { return m_Started; }
        //! То же, что и \c isStarted.
        inline bool isRunning() const { return m_Started; }
        //! Возвращает true, если счётчик остановлен.
        inline bool isStopped() const { return !m_Started; }
        //! Возвращает true, если счётчик приостановлен.
        inline bool isPaused() const {return m_Paused; }
};

//------------------------------------------------------------------------------

#endif // __TIMECOUNTER__HPP__3FDDD5C2_1BD9_4884_A4AB_B84F6321A5B6__
