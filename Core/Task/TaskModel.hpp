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

#ifndef __TASKMODEL__HPP__
#define __TASKMODEL__HPP__

#include <QStandardItemModel>

#include "Task.hpp"

//------------------------------------------------------------------------------

class TTaskModel : public QStandardItemModel
{
    Q_OBJECT
    private :
        class QStandardItem2;
        typedef QMap<TSharedConstTask, QStandardItem2*> TTasks;

        TTasks           m_Tasks;        //!< Список задач.
        TSharedConstTask m_CurrentTask;  //!< Текущая выполняемая задача.
        QStandardItem*   m_pRoot;        //!< Корневой элемент модели.
        int              m_LastNumber;   //!< Порядковый номер задачи.

        Q_DISABLE_COPY(TTaskModel)

    public :
        explicit TTaskModel(QObject* Parent = NULL);
        virtual ~TTaskModel();

        void retranslate();

        void newTask(TSharedConstTask Task);
        void beginTask(TSharedConstTask Task);
        void endTask(TSharedConstTask Task);
        void deleteTask(TSharedConstTask Task);
        void deleteTask(int row);
        void deleteTasks(TTaskList TaskList);

        int taskNumberForIndex(const QModelIndex& Index) const;
        TSharedConstTask taskForNumber(int row) const;
        TSharedConstTask taskForIndex(const QModelIndex& Index) const;
        void moveTask(int row, int delta);

        //! Порядковый номер последней добавленной задачи.
        inline int lastNumber() const { return m_LastNumber; }
        //! Установка порядкового последней добавленной задачи.
        inline void setLastNumber(int Number) { m_LastNumber = Number; }
        //! Список указателей на задачи.
        inline TTaskList taskList() const { return m_Tasks.keys(); }
        inline int tasksCount() const { return m_Tasks.count(); }
};

//------------------------------------------------------------------------------

#endif // __TASKMODEL__HPP__
