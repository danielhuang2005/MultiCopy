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

#include "ProgressForm.hpp"
#include "ui_ProgressForm.h"

#include <QResizeEvent>
#include <QTime>
#include <QMessageBox>
#include <QSettings>
#include <QDir>

#include "Core/Common/CommonFn.hpp"
#include "Core/Task/TaskStatus.hpp"
#include "Core/Task/TaskModel.hpp"
#include "Core/Threads/TaskManager.hpp"
#include "GUI/Settings.hpp"
#include "GUI/GUIErrorHandler.hpp"

//------------------------------------------------------------------------------
//! Конструктор.

TProgressForm::TProgressForm(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::TProgressForm()),
      m_ForcedHide(false),
      m_pTaskManager(new TTaskManager(NULL)),
      m_pSettings(TSettings::instance()),
      m_pTaskModel(new TTaskModel()),
      m_pGUIErrorHandler(new TGUIErrorHandler(this))
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    /* При других значениях этого параметра программа в некоторых случаях
       почему-то падает на QTreeViewPrivate::coordinateForItem, попадая на
       строку Q_ASSERT(false). Предполагается, что это ошибка в Qt. */
    // В версии 4.8.2 этой ошибки уже нет.
    #if QT_VERSION < 0x040802
        ui->TaskListTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    #endif

    // Менеджер задач.
    Q_ASSERT(m_pTaskManager != NULL);
    connect(m_pTaskManager, SIGNAL(begin()),
            SLOT(begin()));
    connect(m_pTaskManager, SIGNAL(beginTask(TSharedConstTask)),
            SLOT(beginTask(TSharedConstTask)));
    connect(m_pTaskManager, SIGNAL(beginCalculate()),
            SLOT(beginCalculate()));
    connect(m_pTaskManager, SIGNAL(endCalculate(TTaskSize)),
            SLOT(endCalculate(TTaskSize)));
    connect(m_pTaskManager, SIGNAL(beginCopy()),
            SLOT(beginCopy()));
    connect(m_pTaskManager, SIGNAL(endCopy()),
            SLOT(endCopy()));
    connect(m_pTaskManager, SIGNAL(endTask(TSharedConstTask)),
            SLOT(endTask(TSharedConstTask)));
    connect(m_pTaskManager, SIGNAL(end()),
            SLOT(end()));
    connect(m_pTaskManager, SIGNAL(cancelTasks(TTaskList)),
            SLOT(cancelTasks(TTaskList)));
    connect(m_pTaskManager, SIGNAL(outOfMemory()),
            SLOT(outOfMemory()));

    // Таймеры.
    m_TimeTimer.setInterval(1000);
    connect(&m_TimeTimer, SIGNAL(timeout()), SLOT(updateSpeedAndTime()));
    m_ProgressTimer.setInterval(250);
    connect(&m_ProgressTimer, SIGNAL(timeout()), SLOT(updateProgress()));

    // Модель.
    ui->TaskListTree->setModel(m_pTaskModel);

    // Обработчик ошибок.
    Q_ASSERT(m_pGUIErrorHandler != NULL);
    connect(m_pTaskManager, SIGNAL(error(TErrorData*)),
            m_pGUIErrorHandler, SLOT(errorReceiver(TErrorData*)));
    connect(m_pGUIErrorHandler, SIGNAL(errorProcessed(TErrorData*)),
            m_pTaskManager, SLOT(errorProcessed(TErrorData*)));

    restoreSession();
}

//------------------------------------------------------------------------------
//! Деструктор.

TProgressForm::~TProgressForm()
{
    delete m_pTaskManager;
    delete m_pTaskModel;
    saveSession();
    delete ui;
}

//------------------------------------------------------------------------------
//! "Вписывание" текста в QLabel.

void TProgressForm::elideText(QLabel* pLabel, const QString& Text)
{
    Q_ASSERT(pLabel != NULL);

    QFontMetrics FM = pLabel->fontMetrics();
    QString ElidedText = FM.elidedText(Text, Qt::ElideMiddle,
                                       pLabel->width() - 10);
    pLabel->setText(ElidedText);
}

//------------------------------------------------------------------------------
//! "Вписывание" текста сообщений о прогрессе операции.

void TProgressForm::elideProgressText()
{
    elideText(ui->SourceName, m_SrcText);
    elideText(ui->DestName,   m_DestText);
}

//------------------------------------------------------------------------------
//! Установка текста сообщений о прогрессе операции.

void TProgressForm::setProgressText(const QString& SrcText,
                                           const QString& DestText)
{
    m_SrcText  = SrcText;
    m_DestText = DestText;
    elideProgressText();
}

//------------------------------------------------------------------------------

void TProgressForm::clearView()
{
    ui->Speed->clear();
    ui->Elapsed->clear();
    ui->Remaining->clear();
    ui->ReadProgress_Label->setText(QString());
    ui->WriteProgress_Label->setText(QString());

    ui->CountProgress_Label->setEnabled(false);
    ui->CountProgress->setRange(0, 100);
    ui->CountProgress->setValue(0);
    ui->CountProgress->setFormat(QString());
    ui->CountProgress->update();

    ui->TotalProgress_Label->setEnabled(false);
    ui->TotalProgress->setRange(0, 100);
    ui->TotalProgress->setValue(0);
    ui->TotalProgress->setFormat(QString());
    ui->TotalProgress->update();
}

//------------------------------------------------------------------------------
//! Обработчик смены содержимого или выделенного элемента списка задач.

void TProgressForm::taskListChanged()
{
    QModelIndex Index = ui->TaskListTree->currentIndex();
    if (!Index.isValid() || Index.parent() != ui->TaskListTree->rootIndex()) {
        ui->Task_Up->setEnabled(false);
        ui->Task_Down->setEnabled(false);
        ui->Task_Delete->setEnabled(false);
        ui->Task_Edit->setEnabled(false);
    }
    else {
        int row = m_pTaskModel->taskNumberForIndex(Index);
        ui->Task_Up->setEnabled(row >= 2);
        ui->Task_Down->setEnabled((row >= 1) && (row < m_pTaskModel->tasksCount() - 1));
        ui->Task_Delete->setEnabled(true);
        ui->Task_Edit->setEnabled(row > 0);
    }
}

//------------------------------------------------------------------------------

void TProgressForm::pendingTasksChanged()
{
    int Pending = m_pTaskModel->tasksCount() - 1;
    bool Visible = Pending > 0;
    ui->Pending_Text->setVisible(Visible);
    ui->Pending_Num->setVisible(Visible);
    if (Visible)
        ui->Pending_Num->setText(QString::number(Pending));
}

//------------------------------------------------------------------------------

void TProgressForm::moveTask(int Delta)
{
    QModelIndex Index = ui->TaskListTree->currentIndex();
    int row = m_pTaskModel->taskNumberForIndex(Index);
    m_pTaskModel->moveTask(row, Delta);
    m_pTaskManager->moveTask(row, Delta);
    ui->TaskListTree->setCurrentIndex(m_pTaskModel->index(row + Delta, 0));
    taskListChanged();
}

//------------------------------------------------------------------------------
//! Строковое представление времени.

QString TProgressForm::time(qint64 msec)
{
    if (msec < 0)
        return tr("Unknown");
    QTime Time;
    Time = Time.addSecs(msec / 1000);
    return Time.toString();
}

//------------------------------------------------------------------------------
//! Обновление отображения счётчиков времени и скорости.

void TProgressForm::updateSpeedAndTime()
{
    const TTaskStatus* pTaskStatus = m_pTaskManager->taskStatus();
    if (!pTaskStatus) {
        qWarning("TProgressForm::updateSpeedAndTime. "
                 "TaskManager doesn't have TaskStatus.");
        return;
    }

    TTaskStatus::TSpeedAndTime SpeedAndTime;
    pTaskStatus->speedAndTime(&SpeedAndTime);
    ui->Elapsed->setText(time(SpeedAndTime.ElapsedTime));
    ui->Remaining->setText(time(SpeedAndTime.RemainingTime));
    ui->Speed->setText(speedToStr(SpeedAndTime.Speed));
}

//------------------------------------------------------------------------------
//! Обновление отображения прогресса операции.

void TProgressForm::updateProgress()
{
    Q_ASSERT(m_pTaskManager != NULL);

    if (m_pTaskManager->isCalculating()) {
        TTaskSize TaskSize = m_pTaskManager->taskSize();

        ui->CountProgress->setFormat(QString::number(TaskSize.FilesCount));
        ui->TotalProgress->setFormat(sizeToStr(TaskSize.TotalSize));
    }
    else {
        const TTaskStatus* pTaskStatus = m_pTaskManager->taskStatus();
        if (!pTaskStatus) {
            qWarning("TProgressForm::updateProgress. "
                     "TaskManager doesn't have TaskStatus.");
            return;
        }

        // Статистика чтения.
        TTaskStatus::TStatus Status = pTaskStatus->readingStatus();
        QString SrcText = AddWithSeparator(Status.DirName, Status.RelName);
        ui->ReadProgress->setValue(Status.Percent);

        // Статистика записи.
        Status = pTaskStatus->writingStatus();
        QString DestText = Status.RelName;
        ui->WriteProgress->setValue(Status.Percent);

        QString S;
        if (m_TaskSize.FilesCount > 0) {
            S = tr("%1 of %2").arg(Status.Files)
                              .arg(m_TaskSize.FilesCount);
            ui->CountProgress->setValue(Status.Files);
        }
        else {
            S = QString::number(Status.Files);
        }
        ui->CountProgress->setFormat(S);

        if (m_TaskSize.TotalSize > 0) {
            S = tr("%1 of %2").arg(sizeToStr(Status.TotalBytes),
                                   sizeToStr(m_TaskSize.TotalSize));
            ui->TotalProgress->setValue(Status.TotalPercent);
        }
        else {
            S = sizeToStr(Status.TotalBytes);
        }
        ui->TotalProgress->setFormat(S);


        setProgressText(SrcText, DestText);
    }
}

//------------------------------------------------------------------------------
//! Сохранение настроек формы.

void TProgressForm::saveSession()
{
    QSettings* pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    pS->setValue("geometry", geometry());
    pS->setValue("DoNotClose", ui->DoNotClose->isChecked());
    pS->setValue("ShowTaskList", ui->ShowTaskList->isChecked());
    pS->setValue("TaskListHeight", ui->TaskList->height());
    pS->endGroup();
    m_pSettings->write();
}

//------------------------------------------------------------------------------
//! Восстановление настроек формы.

void TProgressForm::restoreSession()
{
    QSettings* pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    QRect Geometry = pS->value("geometry").toRect();
    if (!Geometry.isEmpty())
        setGeometry(Geometry);
    ui->DoNotClose->setChecked(pS->value("DoNotClose").toBool());
    ui->ShowTaskList->setChecked(pS->value("ShowTaskList").toBool());
    m_TaskListHeight = pS->value("TaskListHeight").toInt();
    ui->TaskList->resize(width(), m_TaskListHeight);
    pS->endGroup();
}

//------------------------------------------------------------------------------
//! Вывод диалога отмены копирования.
/*!
   \param Exit Если true, предполагается закрытие окна. В этом случае диалог
     запросит отмену всех заданий. Если false, предполагается нажатие кнопки
     "Отмена". В этом случае диалог также запросит отмену текущего задания.
 */

bool TProgressForm::cancelDialog(TCancelReason CancelReason)
{
    Q_ASSERT(m_pTaskManager != NULL);

    if (!m_pTaskManager->isRunning())
        return true;

    if (m_pTaskModel->tasksCount() <= 0) {
        qWarning("TProgressForm::cancelDialog. Tasks count is nonpositive.");
        return true;
    }

    bool Cancel = false;
    m_pTaskManager->lockProcessErrors();
    m_pTaskManager->pause();


    if (CancelReason == crCloseForm || CancelReason == crCloseApplication) {
        QString Message;
        if (CancelReason == crCloseForm)
            Message = m_pTaskModel->tasksCount() > 1 ?
                          tr("Cancel all tasks and close window?") :
                          tr("Cancel task and close window?");
        else // crCloseApplication
            Message = m_pTaskModel->tasksCount() > 1 ?
                          tr("Cancel all tasks and close application?") :
                          tr("Cancel copy and close application?");

        if (QMessageBox::question(this,
                                  QApplication::applicationName(),
                                  Message,
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
        {
            m_pTaskManager->cancelAllTasks();
            Cancel = true;
        }
    }
    else {
        QMessageBox::StandardButtons Buttons = QMessageBox::Yes | QMessageBox::No;
        if (m_pTaskModel->tasksCount() > 1 && CancelReason != crDeleteButton)
            Buttons |= QMessageBox::YesToAll;

        QMessageBox::StandardButton Btn = QMessageBox::question(
                                              this,
                                              QApplication::applicationName(),
                                              tr("Do you really want to cancel a copy?"),
                                              Buttons,
                                              QMessageBox::No
                                          );
        switch (Btn) {
            case QMessageBox::YesToAll :
                m_pTaskManager->cancelAllTasks();
                Cancel = true;
                break;
            case QMessageBox::Yes :
                m_pTaskManager->cancelCurrentTask();
                Cancel = true;
                break;
            default :
                ;
        }
    }

    m_pTaskManager->resume();
    m_pTaskManager->unlockProcessErrors();

    if (Cancel && ui->Pause->isChecked())
        ui->Pause->click();

    return Cancel;
}

//------------------------------------------------------------------------------
//! Обработчик смены состояний.

void TProgressForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            m_pTaskModel->retranslate();
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------
//! Обработчик изменения размера.

void TProgressForm::resizeEvent(QResizeEvent *Event)
{
    if (Event->size().width() != Event->oldSize().width())
        elideProgressText();
}

//------------------------------------------------------------------------------
//! Обработчик события закрытия формы.

void TProgressForm::closeEvent(QCloseEvent *Event)
{
    if (m_pTaskManager->isRunning()) {
        Event->ignore();
        m_ForcedHide = cancelDialog(crCloseForm);
    }
}

//------------------------------------------------------------------------------
//! Обработчик события скрытия формы.

void TProgressForm::hideEvent(QHideEvent*)
{
    emit hidden();
}

//------------------------------------------------------------------------------
//! Обработчик сигнала о недостатке памяти.

void TProgressForm::outOfMemory()
{
    QMessageBox::critical(this,
                          QApplication::applicationName(),
                          tr("Out of Memory!"));
}

//------------------------------------------------------------------------------

void TProgressForm::begin()
{
    ui->Cancel->setEnabled(true);
    ui->Pause->setEnabled(true);
    clearView();
    m_ForcedHide = false;

    show();
    setWindowState(windowState() & ~Qt::WindowMinimized);
    raise();
    activateWindow();
}

//------------------------------------------------------------------------------

void TProgressForm::beginTask(TSharedConstTask Task)
{
    m_TaskSize.clear();
    m_pTaskModel->beginTask(Task);
    m_ProgressTimer.start();

    clearView();
}

//------------------------------------------------------------------------------

void TProgressForm::beginCalculate()
{
    ui->ReadProgress->setEnabled(false);
    ui->ReadProgress_Label->setEnabled(false);
    ui->ReadProgress->setRange(0, 0);
    ui->ReadProgress->setValue(0);
    ui->ReadProgress->update();

    ui->WriteProgress->setEnabled(false);
    ui->WriteProgress_Label->setEnabled(false);
    ui->WriteProgress->setRange(0, 0);
    ui->WriteProgress->setValue(0);
    ui->WriteProgress->update();

    setProgressText(tr("Calculate task size..."), QString());
}

//------------------------------------------------------------------------------

void TProgressForm::endCalculate(TTaskSize TaskSize)
{
    m_TaskSize = TaskSize;

    ui->ReadProgress->setEnabled(true);
    ui->ReadProgress_Label->setEnabled(true);
    ui->ReadProgress->setRange(0, 100);
    ui->ReadProgress->setValue(0);
    ui->ReadProgress->update();

    ui->WriteProgress->setEnabled(true);
    ui->WriteProgress_Label->setEnabled(true);
    ui->WriteProgress->setRange(0, 100);
    ui->WriteProgress->setValue(0);
    ui->WriteProgress->update();

    ui->CountProgress_Label->setEnabled(true);
    ui->CountProgress->setRange(0, m_TaskSize.FilesCount);
    ui->CountProgress->setValue(0);
    ui->CountProgress->update();

    ui->TotalProgress_Label->setEnabled(true);
    ui->TotalProgress->setRange(0, 100);
    ui->TotalProgress->setValue(0);
    ui->TotalProgress->update();
}

//------------------------------------------------------------------------------

void TProgressForm::beginCopy()
{
    setProgressText(tr("Preparing..."), QString());

    m_TimeTimer.start();
}

//------------------------------------------------------------------------------

void TProgressForm::endCopy()
{
    m_ProgressTimer.stop();
    m_TimeTimer.stop();

    updateProgress();
}

//------------------------------------------------------------------------------

void TProgressForm::endTask(TSharedConstTask Task)
{
    m_pTaskModel->endTask(Task);
    taskListChanged();
    pendingTasksChanged();
}

//------------------------------------------------------------------------------

void TProgressForm::end()
{
    if (m_ForcedHide || !ui->DoNotClose->isChecked())
        hide();
    else
        ui->SourceName->setText(tr("All tasks are completed!"));
    ui->DestName->clear();
    ui->ReadProgress->setRange(0, 100);
    ui->ReadProgress->setValue(100);
    ui->WriteProgress->setRange(0, 100);
    ui->WriteProgress->setValue(100);
    ui->CountProgress->setValue(-1);
    ui->TotalProgress->setValue(-1);
    ui->Cancel->setEnabled(false);
    ui->Pause->setEnabled(false);
    ui->Remaining->clear();
}

//------------------------------------------------------------------------------

void TProgressForm::cancelTasks(TTaskList TaskList)
{
    m_pTaskModel->deleteTasks(TaskList);
}

//------------------------------------------------------------------------------

void TProgressForm::addTask(TSharedConstTask Task)
{
    m_pTaskModel->newTask(Task);
    m_pTaskManager->addTask(Task);
    m_pTaskManager->start();
    taskListChanged();
    pendingTasksChanged();
}

//------------------------------------------------------------------------------

bool TProgressForm::exit()
{
    bool Exited = cancelDialog(crCloseApplication);
    if (Exited && m_pTaskManager != NULL) {
        // TODO: Возможно "замерзание" графического интерфейса.
        m_pTaskManager->wait();
        close();
    }
    return Exited;
}

//------------------------------------------------------------------------------
//! Обработчик нажатия кнопки отмена.

void TProgressForm::on_Cancel_clicked()
{
    cancelDialog(crCancelButton);
}

//------------------------------------------------------------------------------
//! Обработчик нажатия кноки паузы.

void TProgressForm::on_Pause_clicked()
{
    if (ui->Pause->isChecked())
    {
        m_pTaskManager->pause();
        ui->Pause->setText(tr("&Resume"));
    }
    else {
        m_pTaskManager->resume();
        ui->Pause->setText(tr("&Pause"));
    }
}

//------------------------------------------------------------------------------

void TProgressForm::reject()
{
    cancelDialog(crCancelButton);
}

//------------------------------------------------------------------------------
//! Обработчик переключения состояния видимости списка заданий.

void TProgressForm::on_ShowTaskList_toggled(bool checked)
{
    QSize Size = size();
    if (!checked)
        m_TaskListHeight = ui->TaskList->height();
    ui->TaskList->setVisible(checked);
    if (!checked) {
        QSize SizeHint = sizeHint();
        if (SizeHint.isValid()) {
            SizeHint.setWidth(size().width());
            if (layout())
                layout()->activate();
            resize(SizeHint);
            setMaximumHeight(height());
        }
    }
    else {
        int newHeight = Size.height() + m_TaskListHeight;
        if (layout()) {
            newHeight += layout()->spacing();
            layout()->activate();
        }
        Size.setHeight(newHeight);
        setMaximumHeight(16777215);
        resize(Size);
    }
}

//------------------------------------------------------------------------------
//! Обработчик щелчка мышью внутри списка задач.

void TProgressForm::on_TaskListTree_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    taskListChanged();
}

//------------------------------------------------------------------------------
//! Обработчик нажатия кнопки удаления задания.

void TProgressForm::on_Task_Delete_clicked()
{
    int row = m_pTaskModel->taskNumberForIndex(ui->TaskListTree->currentIndex());
    if (row == 0)
        cancelDialog(crDeleteButton);
    else {
        TSharedConstTask Task = m_pTaskModel->taskForNumber(row);
        m_pTaskModel->deleteTask(row);
        m_pTaskManager->deleteTask(Task);
        taskListChanged();
        pendingTasksChanged();
    }
}

//------------------------------------------------------------------------------

void TProgressForm::on_Task_Edit_clicked()
{
    int row = m_pTaskModel->taskNumberForIndex(ui->TaskListTree->currentIndex());
    TSharedConstTask Task = m_pTaskModel->taskForNumber(row);

    if (receivers(SIGNAL(taskNeedEditing(TSharedConstTask))) <= 0) {
        qWarning("TProgressForm::on_Task_Edit_clicked. "
                 "No receivers for signal taskNeedEditing.");
    }

    emit taskNeedEditing(Task);

    if (row > 0) {
        m_pTaskModel->deleteTask(row);
        m_pTaskManager->deleteTask(Task);
        taskListChanged();
        pendingTasksChanged();
    }
}

//------------------------------------------------------------------------------

void TProgressForm::on_Task_Up_clicked()
{
    moveTask(-1);
}

//------------------------------------------------------------------------------

void TProgressForm::on_Task_Down_clicked()
{
    moveTask(+1);
}

//------------------------------------------------------------------------------
