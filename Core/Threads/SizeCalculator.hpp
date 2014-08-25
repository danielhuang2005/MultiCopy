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

#ifndef __SIZECALCULATOR__HPP__
#define __SIZECALCULATOR__HPP__

//------------------------------------------------------------------------------

#include <QString>
#include <QStringList>

#include "ThreadEx.hpp"
#include "Core/IO/DirEnumerator.hpp"
#include "Core/Task/Task.hpp"

//------------------------------------------------------------------------------
//! Класс для подсчёта размера задания.

class TSizeCalculator : public TThreadEx
{
    Q_OBJECT
    private :
        TSharedConstTask m_Task;      //!< Задание.
        TDirEnumerator*  m_pDirEnum;  //!< Перечислитель каталогов.
        TTaskSize        m_TaskSize;  //!< Результат подсчёта размера.

        void calculateOne(const TDirEnumerator::TParams& Params);
        void calculateAll();

        // Скрываем копирующий конструктор и оператор присваивания.
        Q_DISABLE_COPY(TSizeCalculator)

    protected :
        virtual void run();

    public:
        explicit TSizeCalculator();
        ~TSizeCalculator();

        void setTask(const TSharedConstTask& Task);

        //! Задание для подсчёта размера.
        inline TSharedConstTask task() const { return m_Task; }

        //! Результат подсчёта размера.
        inline TTaskSize taskSize() const { return m_TaskSize; }

    signals :
        //! Начало подсчёта размера.
        void begin();

        //! Завершение подсчёта размера и результат подсчёта.
        /*!
           \remarks Если подсчёт размера задания был прерван методом
             \c TThreadEx::cancel(), этот сигнал будет сгенерирован, а в
             параметре сигнала будет передан размер подсчитанных до момента
             прерывания элементов файловой системы.
         */
        void end(TTaskSize TaskSize);

        //! Отмена подсчёта размера.
        /*!
           \remarks Данный сигнал будет сгенерирован в случае, если подсчёт
             размера задания был прерван методом \c TThreadEx::cancel().
             Сигнал \c end будет сгенерирован сразу после cancelled.
         */
        void cancelled();
};

//------------------------------------------------------------------------------

#endif // __SIZECALCULATOR__HPP__
