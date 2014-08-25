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

#ifndef __PROGRESSFORM__HPP__
#define __PROGRESSFORM__HPP__

#include <QTimer>
#include <QDialog>
#include <QModelIndex>

#include "Core/Task/Task.hpp"

//------------------------------------------------------------------------------

class QResizeEvent;
class QCloseEvent;
class QLabel;

class TErrorHandler;
class TSettings;
class TGUIErrorHandler;
class TTaskModel;
class TTaskManager;
namespace Ui {
    class TProgressForm;
}

//------------------------------------------------------------------------------

class TProgressForm : public QDialog
{
    Q_OBJECT
    public :
        //! Причина отмены операции.
        enum TCancelReason {
            crCancelButton,      //!< Нажата кнопка отмены.
            crDeleteButton,      //!< Нажата кнопка удаления задания.
            crCloseForm,         //!< Закрытие текущей формы.
            crCloseApplication   //!< Выход из приложения.
        };

    private:
        Ui::TProgressForm* ui;

        bool              m_ForcedHide;        //!< Флаг форсированного скрытия формы.
        TTaskManager*     m_pTaskManager;      //!< Диспетчер заданий.
        TSettings*        m_pSettings;         //!< Настройки.
        TTaskModel*       m_pTaskModel;        //!< Модель с заданиями.
        TGUIErrorHandler* m_pGUIErrorHandler;  //!< Обработчик ошибок.
        QTimer            m_TimeTimer;         //!< Таймер для обновления времени.
        QTimer            m_ProgressTimer;     //!< Таймер для обновления прогресса.
        QString           m_SrcText;           //!< Наименование источника.
        QString           m_DestText;          //!< Наименование назначения.
        TTaskSize         m_TaskSize;          //!< Размер задания.
        int               m_TaskListHeight;    //!< Высота виджета со списком заданий.

        Q_DISABLE_COPY(TProgressForm)

        void elideText(QLabel* pLabel, const QString& Text);
        void elideProgressText();
        void setProgressText(const QString& SrcText, const QString& DestText);
        void taskListChanged();
        void pendingTasksChanged();
        void moveTask(int Delta);

        static QString time(qint64 msec);

        void saveSession();
        void restoreSession();

        bool cancelDialog(TCancelReason CancelReason);

    protected :
        virtual void changeEvent(QEvent *e);
        virtual void resizeEvent(QResizeEvent *Event);
        virtual void closeEvent(QCloseEvent *Event);
        virtual void hideEvent(QHideEvent*);

    private slots :
        void updateProgress();
        void updateSpeedAndTime();

        void begin();
        void beginTask(TSharedConstTask Task);
        void beginCalculate();
        void endCalculate(TTaskSize TaskSize);
        void beginCopy();
        void endCopy();
        void endTask(TSharedConstTask Task);
        void end();
        void cancelTasks(TTaskList TaskList);

        void outOfMemory();

        void on_Cancel_clicked();
        void on_Pause_clicked();
        void on_ShowTaskList_toggled(bool checked);
        void on_TaskListTree_clicked(const QModelIndex &index);
        void on_Task_Delete_clicked();
        void on_Task_Edit_clicked();
        void on_Task_Up_clicked();
        void on_Task_Down_clicked();

    public:
        explicit TProgressForm(QWidget* parent = NULL);
        virtual ~TProgressForm();
        void addTask(TSharedConstTask Task);
        bool exit();

    signals :
        void hidden();
        void taskNeedEditing(TSharedConstTask Task);

    public slots :
        virtual void reject();
};

//------------------------------------------------------------------------------

#endif // __PROGRESSFORM__HPP__
