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

#include "TaskModel.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Класс элемента модели с дополнительным функционалом.

class TTaskModel::QStandardItem2 : public QStandardItem
{
    public :
        //! Тип элемента.
        enum TItemType {
            TaskHeader,   //!< Заголовок задания.
            Sources,      //!< Список источников.
            Destinations  //!< Список назначений.
        };

    private :
        int       m_TaskNumber;  //!< Номер задания.
        TItemType m_ItemType;    //!< Тип элемента.
        bool      m_Processing;  //!< Флаг обработки задания.

    public :
        QStandardItem2(int TaskNumber);
        QStandardItem2(TItemType ItemType);

        void retranslate(bool WithChildren = true);

        int taskNumber() const { return m_TaskNumber; }
        TItemType itemType() const { return m_ItemType; }
        bool isProcessing() const { return m_Processing; }
        void setProcessing(bool Processing);
};

//------------------------------------------------------------------------------
//! Конструктор элемента с типом "заголовок задания".

TTaskModel::QStandardItem2::QStandardItem2(int TaskNumber)
    : m_TaskNumber(TaskNumber), m_ItemType(TaskHeader), m_Processing(false)
{
}

//------------------------------------------------------------------------------
//! Конструктор элемента указанного типа.

TTaskModel::QStandardItem2::QStandardItem2(TItemType ItemType)
    : m_TaskNumber(-1), m_ItemType(ItemType), m_Processing(false)
{
}

//------------------------------------------------------------------------------
//! Переводчик элемента.
/*!
  \arg WithChildren Флаг перевода дочерних элементов.
 */

void TTaskModel::QStandardItem2::retranslate(bool WithChildren)
{
    switch (m_ItemType)
    {
        case TaskHeader :
            Q_ASSERT(m_TaskNumber > 0);
            setText(tr("Task No.%1").arg(m_TaskNumber));
            if (m_Processing) {
                setText(text() + " " + tr("(processing)"));
                QFont Font = font();
                Font.setBold(true);
                setFont(Font);
            }
            break;
        case Sources :
            setText(tr("Source(s)"));
            break;
        case Destinations :
            setText(tr("Destination(s)"));
            break;
        default :
            qWarning("TTaskModel::QStandardItem2::retranslate. "
                     "Nnknown item type (%i).", m_ItemType);
    }

    if (WithChildren) {
        for (int row = 0; row < rowCount(); ++row) {
            QStandardItem2* pItem2 = dynamic_cast<QStandardItem2*>(child(row));
            if (pItem2 != NULL)
                pItem2->retranslate();
        }
    }
}

//------------------------------------------------------------------------------
//! Установка флага обработки задания.

void TTaskModel::QStandardItem2::setProcessing(bool Processing)
{
    Q_ASSERT(m_ItemType == TaskHeader);

    m_Processing = Processing;
    retranslate(false);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TTaskModel::TTaskModel(QObject* Parent)
    : QStandardItemModel(Parent),
      m_pRoot(invisibleRootItem()),
      m_LastNumber(0)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TTaskModel::~TTaskModel()
{
}

//------------------------------------------------------------------------------
//! Перевод.

void TTaskModel::retranslate()
{
    for (TTasks::iterator I = m_Tasks.begin(); I != m_Tasks.end(); ++I)
        (*I)->retranslate();
}

//------------------------------------------------------------------------------
//! Добавление задачи.

void TTaskModel::newTask(TSharedConstTask Task)
{
    Q_ASSERT(!Task.isNull());

    ++m_LastNumber;
    QStandardItem2* pItem = new QStandardItem2(m_LastNumber);
    m_Tasks.insert(Task, pItem);

    QStandardItem2* pSrcs  = new QStandardItem2(QStandardItem2::Sources);
    for (int i = 0; i < Task->SrcList.count(); ++i) {
        QStandardItem* pItem = new QStandardItem(Task->SrcList[i]);
        pSrcs->appendRow(pItem);
    }
    pItem->appendRow(pSrcs);

    QStandardItem2* pDests = new QStandardItem2(QStandardItem2::Destinations);
    for (int i = 0; i < Task->DestList.count(); ++i) {
        pDests->appendRow(new QStandardItem(Task->DestList[i]));
    }
    pItem->appendRow(pDests);

    pItem->retranslate();
    m_pRoot->appendRow(pItem);
}

//------------------------------------------------------------------------------

void TTaskModel::beginTask(TSharedConstTask Task)
{
    if (m_Tasks.contains(Task)) {
        if (!m_CurrentTask.isNull()) {
            qWarning("TTaskModel::beginTask. Current task is not empty.");
        }

        m_CurrentTask = Task;

        QStandardItem2* pItem = m_Tasks[Task];
        pItem->setProcessing(true);
    }
    else {
        qWarning("TTaskModel::beginTask. Task not found.");
    }
}

//------------------------------------------------------------------------------

void TTaskModel::endTask(TSharedConstTask Task)
{
    deleteTask(Task);
}

//------------------------------------------------------------------------------

void TTaskModel::deleteTask(TSharedConstTask Task)
{
    if (m_Tasks.contains(Task)) {
        QStandardItem* pItem = m_Tasks.take(Task);
        m_pRoot->removeRow(pItem->index().row());
        if (Task == m_CurrentTask)
            m_CurrentTask.clear();
    }
    else {
        qWarning("TTaskModel::deleteTask. Task not found.");
    }
}

//------------------------------------------------------------------------------

void TTaskModel::deleteTask(int row)
{
    if (0 <= row && row < m_Tasks.count()) {

        for (TTasks::const_iterator I = m_Tasks.constBegin();
             I != m_Tasks.constEnd(); ++I)
        {
            if (I.value()->row() == row) {
                deleteTask(I.key());
                break;
            }
        }
    }
    else {
        qWarning("TTaskModel::deleteTask. Index is out of range "
                 "(index = %i, count = %i)", row, m_Tasks.count());
    }
}

//------------------------------------------------------------------------------

void TTaskModel::deleteTasks(TTaskList TaskList)
{
    for (int i = TaskList.count() - 1; i >= 0; --i)
        deleteTask(TaskList[i]);
}

//------------------------------------------------------------------------------

int TTaskModel::taskNumberForIndex(const QModelIndex& Index) const
{
    if (!Index.isValid() || Index.model() != this)
        return -1;

    QModelIndex Parent = Index.parent();
    QModelIndex Index2 = Index;
    QModelIndex RootIndex = m_pRoot->index();
    while (Parent != RootIndex) {
        Index2 = Parent;
        Parent = Index2.parent();
    }
    return Index2.row();
}

//------------------------------------------------------------------------------

TSharedConstTask TTaskModel::taskForNumber(int row) const
{
    if (0 <= row && row < m_Tasks.count()) {
        for (TTasks::const_iterator I = m_Tasks.constBegin();
             I != m_Tasks.constEnd(); ++I)
        {
            if (I.value()->row() == row)
                return I.key();
        }
    }
    return TSharedConstTask();
}

//------------------------------------------------------------------------------

TSharedConstTask TTaskModel::taskForIndex(const QModelIndex& Index) const
{
    return taskForNumber(taskNumberForIndex(Index));
}

//------------------------------------------------------------------------------

void TTaskModel::moveTask(int row, int delta)
{
    if (delta == 0)
        return;

    if (0 <= row && row < m_Tasks.count()) {
        int newRow = row + delta;
        if (newRow < 0) newRow = 0;
        if (newRow >= m_Tasks.count()) newRow = m_Tasks.count() - 1;
        if (row != newRow) {
            QList<QStandardItem*> Items = m_pRoot->takeRow(row);
            Q_ASSERT(Items.count() == 1);
            m_pRoot->insertRow(newRow, Items);
        }
    }
    else {
        qWarning("TTaskModel::moveTask. Index is out of range "
                 "(index = %i, count = %i)", row, m_Tasks.count());
    }
}

//------------------------------------------------------------------------------
