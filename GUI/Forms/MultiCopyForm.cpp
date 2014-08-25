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

#include "MultiCopyForm.hpp"
#include "ui_MultiCopyForm.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QCloseEvent>
#include <QTime>

#include "Core/Resources.hpp"
#include "Core/Common/CommonFn.hpp"
#include "Core/Task/GlobalStatistics.hpp"
#include "GUI/FileDialogs/FileDialogs.hpp"
#include "GUI/Forms/SettingsForm.hpp"
#include "GUI/Forms/TaskSettingsForm.hpp"
#include "GUI/Forms/ProgressForm.hpp"
#include "GUI/Translator.hpp"
#include "GUI/Settings.hpp"

//------------------------------------------------------------------------------
//! Перемещение элемента в списке типа QListWidget.
/*!
   Метод перемещает элемент списка ListWidget, находящийся в позиции с индексом
   Row, на Delta позиций. Delta может быть как положительной, так и
   отрицательной. Если индекс Row неверный, ничего не происходит. Если новая
   позиция элемента выходит за пределы индексов списка, производится коррекция.

   \remarks Подсветка устанавливается на перемещённый элемент.
 */

void MoveListWigetItem(QListWidget* ListWidget, int Row, int Delta)
{
    int count = ListWidget->count();
    if ((Row < 0) || (Row >= count) || (Delta == 0) || (count == 0))
        return;
    int NewRow = Row + Delta;
    if (NewRow < 0) NewRow = 0;
    if (NewRow >= count) NewRow = count - 1;
    if (NewRow == Row) return;

    ListWidget->insertItem(NewRow, ListWidget->takeItem(Row));
    ListWidget->setCurrentRow(NewRow);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TMultiCopy::TMultiCopy(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TMultiCopyForm),
    m_pSettings(TSettings::instance()),
    m_pProgressForm(new TProgressForm())
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName());
    ui->SrcList->setDirsOnly(false);
    ui->DestList->setDirsOnly(true);

    TGeneralSettings* pGS = &m_pSettings->GeneralSettings;
    ui->DestList->setCheckDirs(pGS->CheckDestDirs);
    ui->DestList->setCheckNetworkDirs(pGS->CheckNetworkDestDirs);

    viewChange();

    // "Сложные" соединения (дизайнер сделать не позволяет).
    connect(ui->actionAbout_Qt, SIGNAL(triggered()),
            qApp, SLOT(aboutQt()));
    connect(m_pProgressForm, SIGNAL(closed()),
            SLOT(progressFormClosed()));
    connect(m_pProgressForm, SIGNAL(taskNeedEditing(TSharedConstTask)),
            SLOT(loadTask(TSharedConstTask)));

    restoreSession();

    ui->SrcList->setCurrentRow(0);
    ui->DestList->setCurrentRow(0);
}

//------------------------------------------------------------------------------
//! Деструктор.

TMultiCopy::~TMultiCopy()
{
    delete m_pProgressForm;
    delete ui;
}

//------------------------------------------------------------------------------

void TMultiCopy::srcChanged()
{
    // Кнопка "Remove".
    ui->SrcRemove->setEnabled(!ui->SrcList->selectedItems().isEmpty());

    //Кнопка "Clear".
    ui->SrcClear->setEnabled(ui->SrcList->count() > 0);

    // Кнопка "Up".
    ui->SrcUp->setEnabled(ui->SrcList->canMoveUpSelected());

    // Кнопка "Down".
    ui->SrcDown->setEnabled(ui->SrcList->canMoveDownSelected());

    // Кнопка "Start".
    ui->Start->setEnabled((ui->SrcList->count() > 0) &&
                          (ui->DestList->count() > 0));
}

//------------------------------------------------------------------------------

void TMultiCopy::destChanged()
{
    // Кнопка "Clear".
    ui->DestClear->setEnabled(ui->DestList->count() > 0);

    // Кнопка "Remove".
    ui->DestRemove->setEnabled(!ui->DestList->selectedItems().isEmpty());

    // Кнопка "Start".
    ui->Start->setEnabled((ui->SrcList->count() > 0) &&
                          (ui->DestList->count() > 0));
}

//------------------------------------------------------------------------------

void TMultiCopy::saveSession()
{
    QSettings* pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    pS->setValue(ui->SrcList->objectName(), ui->SrcList->toStringList());
    pS->setValue(ui->DestList->objectName(), ui->DestList->toStringList());
    pS->setValue("geometry", geometry());
    pS->setValue(ui->splitter->objectName(), ui->splitter->saveState());
    pS->endGroup();

    TGlobalStatistics::instance()->write(pS);

    m_pSettings->write();
}

//------------------------------------------------------------------------------

void TMultiCopy::restoreSession()
{
    QSettings* pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    QRect Geometry = pS->value("geometry").toRect();
    if (!Geometry.isEmpty())
        setGeometry(Geometry);
    ui->splitter->restoreState(pS->value(ui->splitter->objectName()).toByteArray());
    pS->endGroup();
}

//------------------------------------------------------------------------------

bool TMultiCopy::loadStringListFromFile(const QString& FileName,
                                        QStringList& List)
{
    // TODO: Обрабатывать ошибки.
    QFile File(FileName);
    if (File.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream Stream(&File);
        QString S;
        while (!(S = Stream.readLine()).isEmpty())
            List.append(S);
        File.close();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool TMultiCopy::saveStringListToFile(const QString& FileName,
                                      const QStringList& List)
{
    // TODO: Обрабатывать ошибки.
    QFile File(FileName);
    if (File.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
        QTextStream Stream(&File);
        for (QStringList::const_iterator I = List.begin(); I != List.end(); ++I)
            Stream << *I << "\n";
        File.close();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool TMultiCopy::loadTaskFromFile(const QString& FileName)
{
    // TODO: Обрабатывать ошибки.
    QFile File(FileName);
    bool Result = File.open(QFile::ReadOnly | QFile::Text);

    if (Result) {
        QString S;
        QTextStream Stream(&File);
        QStringList List,
                    OldSrcList  = ui->SrcList->toStringList(),
                    OldDestList = ui->DestList->toStringList();

        ui->SrcList->clear();
        ui->DestList->clear();

        // Загрузка списка источников.
        while(!(S = Stream.readLine()).isEmpty())
            List.append(S);
        Result = ui->SrcList->checkAndAddItems(&List);

        if (Result) {
            // Загрузка списка назначений.
            List.clear();
            while(!(S = Stream.readLine()).isEmpty())
                List.append(S);
            Result = ui->DestList->checkAndAddItems(&List);
        }

        File.close();

        if (!Result) {
            ui->SrcList->clear();
            ui->SrcList->checkAndAddItems(&OldSrcList);
            ui->DestList->clear();
            ui->DestList->checkAndAddItems(&OldDestList);
        }
    }

    return Result;
}

//------------------------------------------------------------------------------

bool TMultiCopy::saveTaskToFile(const QString& FileName)
{
    // TODO: Обрабатывать ошибки.
    QFile File(FileName);
    if (File.open(QFile::ReadWrite | QFile::Truncate | QFile::Text))
    {
        QTextStream Stream(&File);
        QStringList List;

        // Запись списка источников.
        List = ui->SrcList->toStringList();
        for (QStringList::const_iterator I = List.begin(); I != List.end(); ++I)
            Stream << *I << "\n";

        // Разделитель (пустая строка).
        Stream << "\n";

        // Запись списка назначений.
        List = ui->DestList->toStringList();
        for (QStringList::const_iterator I = List.begin(); I != List.end(); ++I)
            Stream << *I << "\n";

        File.close();

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

void TMultiCopy::setShowNameEditors(bool Show)
{
    ui->SrcCustom->setVisible(Show);
    ui->SrcAddCustom->setVisible(Show);
    ui->DestCustom->setVisible(Show);
    ui->DestAddCustom->setVisible(Show);
}

//------------------------------------------------------------------------------

void TMultiCopy::viewChange()
{
    TGeneralSettings* pGS = &m_pSettings->GeneralSettings;

    bool ShowIcons = pGS->ShowFileIcons;
    ui->SrcList->setShowIcons(ShowIcons);
    ui->DestList->setShowIcons(ShowIcons);

    ShowIcons = pGS->ShowNetworkIcons;
    ui->SrcList->setShowNetworkIcons(ShowIcons);
    ui->DestList->setShowNetworkIcons(ShowIcons);

    setToolButtonsSettings(this);
    setShowNameEditors(pGS->ShowNameEditors);
}

//------------------------------------------------------------------------------
//! Проверка возможности запуска задания.

bool TMultiCopy::checkStart(TTask* pTask)
{
    #ifdef Q_OS_WIN
        // Проверка корневых каталогов дисков.
        if (!pTask->TaskSettings.NoCreateRootDir)
        {
            const QStringList* pList = &pTask->SrcList;
            QStringList RootDirs;
            for (int i = 0; i < pList->count(); ++i)
            {
                if (isRootDir(pList->at(i))) {
                    RootDirs.append(pList->at(i));
                }
            }

            if (!RootDirs.isEmpty())
            {
                QString Message = tr("The next source folders are root folders:\n"
                                     "\n"
                                     "%1\n"
                                     "\n"
                                     "Is turn on the option \"Don't create root folders\" and continue?");
                Message = Message.arg(RootDirs.join("\n"));
                if (QMessageBox::question(this,
                                          QApplication::applicationName(),
                                          Message,
                                          QMessageBox::Yes | QMessageBox::No)
                        == QMessageBox::Yes)
                {
                    pTask->TaskSettings.NoCreateRootDir = true;
                }
                else {
                    return false;
                }
            }
        }
    #endif

    return true;
}

//------------------------------------------------------------------------------

void TMultiCopy::loadListsFromSettings(QSettings* pS)
{
    if (pS == NULL)
        pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    ui->SrcList->addItems(pS->value(ui->SrcList->objectName()).toStringList());
    ui->DestList->addItems(pS->value(ui->DestList->objectName()).toStringList());
    pS->endGroup();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcAddFile_clicked()
{
    static const char* key = "SrcAddFile";
    QSettings* pS = m_pSettings->getQSettings();
    QStringList FileNames = QFileDialog::getOpenFileNames(this,
                                tr("Add source file(s)"),
                                pS->value(key).toString());
    if (FileNames.count() > 0)
    {
        QFileInfo FileInfo(FileNames[0]);
        pS->setValue(key, FileInfo.absoluteDir().absolutePath());
        ui->SrcList->checkAndAddItems(&FileNames);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcAddFolder_clicked()
{
    static const char* key = "SrcAddFolder";
    QSettings* pS = m_pSettings->getQSettings();
    QStringList Dirs = getExistingDirectories(this,
                           tr("Add source folder"),
                           pS->value(key).toString());
    if (!Dirs.isEmpty())
    {
        pS->setValue(key, ParentDir(Dirs.first()));
        if (ui->SrcList->checkAndAddItems(&Dirs))
            srcChanged();
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_DestAddFolder_clicked()
{
    static const char* key = "DestAddFolder";
    QSettings* pS = m_pSettings->getQSettings();
    QStringList Dirs = getExistingDirectories(this,
                           tr("Add destination folder"),
                           pS->value(key).toString());
    if (!Dirs.isEmpty())
    {
        pS->setValue(key, ParentDir(Dirs.first()));
        ui->DestList->checkAndAddItems(&Dirs);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcRemove_clicked()
{
    ui->SrcList->deleteSelected();
    srcChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcClear_clicked()
{
    ui->SrcList->clear();
    srcChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcList_currentRowChanged(int currentRow)
{
    Q_UNUSED(currentRow);
    srcChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_Start_clicked()
{
    TTask* pTask = new TTask();
    pTask->SrcList = ui->SrcList->toStringList();
    pTask->DestList = ui->DestList->toStringList();
    pTask->TaskSettings = m_pSettings->TaskSettings;

    if (checkStart(pTask)) {
        m_pProgressForm->addTask(TSharedConstTask(pTask));
        TGeneralSettings* pGS = &m_pSettings->GeneralSettings;
        if (pGS->AfterStartClearSrcList)
            on_SrcClear_clicked();
        if (pGS->AfterStartClearDestList)
            on_DestClear_clicked();
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcUp_clicked()
{
    ui->SrcList->moveUpSelected();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcDown_clicked()
{
    ui->SrcList->moveDownSelected();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_DestClear_clicked()
{
    ui->DestList->clear();
    destChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_DestRemove_clicked()
{
    ui->DestList->deleteSelected();
    destChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_DestList_currentRowChanged(int currentRow)
{
    Q_UNUSED(currentRow);
    destChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionAbout_triggered()
{
    const int Bits = sizeof(void*) * 8;
    QString Text = tr(
        "<p align='center'>MultiCopy, version %1 (%2-bit)</p>"
    ).arg(version()).arg(Bits);
    QString InformativeText = tr(
        "<p align='center'>This product licensed under GNU GPL version 3. For details see "
        "<a href='http://www.gnu.org/copyleft/gpl.html'>http://www.gnu.org/copyleft/gpl.html</a>.</p>"
        "<p align='center'>Copyright &copy; 2011-2012 Yuri&nbsp;Krugloff.<br> "
        "All rights reserved.</p>"
        "<p align='center'><a href='http://www.tver-soft.org'>http://www.tver-soft.org</a></p>"
    );
    QMessageBox *pBox = new QMessageBox(this);
    pBox->setAttribute(Qt::WA_DeleteOnClose);;
    pBox->setWindowTitle(tr("%1 - About").arg(QApplication::applicationName()));
    pBox->setInformativeText(InformativeText);
    pBox->setText(Text);
    QPixmap Pixmap(":/MainIcon");
    pBox->setIconPixmap(Pixmap);
    pBox->exec();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionLoadSourcesList_triggered()
{
    static const char* key = "LoadSourcesListPath";
    QSettings* pS = m_pSettings->getQSettings();
    QString FileName = QFileDialog::getOpenFileName(this,
                           tr("Load sources list"),
                           pS->value(key).toString(),
                           tr("Lists (*.lst);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        QStringList List;
        loadStringListFromFile(FileName, List);

        ui->SrcList->checkAndAddItems(&List);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionLoadDestinationsList_triggered()
{
    static const char* key = "LoadDestsListPath";
    QSettings* pS = m_pSettings->getQSettings();
    QString FileName = QFileDialog::getOpenFileName(this,
                           tr("Load destinations list"),
                           pS->value(key).toString(),
                           tr("Lists (*.lst);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        QStringList List;
        loadStringListFromFile(FileName, List);
        ui->DestList->checkAndAddItems(&List);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionSaveSourcesList_triggered()
{
    static const char* key = "SaveSourcesListPath";
    QSettings* pS = m_pSettings->getQSettings();
    QString FileName = QFileDialog::getSaveFileName(this,
                           tr("Save sources list"),
                           pS->value(key).toString(),
                           tr("Lists (*.lst);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        saveStringListToFile(FileName, ui->SrcList->toStringList());
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionSaveDestinationsList_triggered()
{
    static const char* key = "SaveDestsListPath";
    QSettings* pS = m_pSettings->getQSettings();
    QString FileName = QFileDialog::getSaveFileName(this,
                           tr("Save destinations list"),
                           pS->value(key).toString(),
                           tr("Lists (*.lst);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        saveStringListToFile(FileName, ui->DestList->toStringList());
    }
}

//------------------------------------------------------------------------------
//! Добавление введённого вручную источника.

void TMultiCopy::addCustomSrc()
{
    if (!ui->SrcCustom->text().isEmpty()) {
        if (ui->SrcList->checkAndAddItem(ui->SrcCustom->text())) {
            ui->SrcCustom->clear();
        }
    }
    else {
        QApplication::beep();
    }
    ui->SrcCustom->setFocus();
}

//------------------------------------------------------------------------------
//! Добавление введённого вручную назначения.

void TMultiCopy::addCustomDest()
{
    if (!ui->DestCustom->text().isEmpty()) {
        if (ui->DestList->checkAndAddItem(ui->DestCustom->text())) {
            ui->DestCustom->clear();
        }
    }
    else {
        QApplication::beep();
    }
    ui->DestCustom->setFocus();
}

//------------------------------------------------------------------------------
//! Обработчик сигнала скрытия формы прогресса.

void TMultiCopy::progressFormClosed()
{
    // TODO : В отдельный метод.
    setWindowState(windowState() & ~Qt::WindowMinimized);
    raise();
    activateWindow();
}

//------------------------------------------------------------------------------
//! Загрузка задания в форму.

void TMultiCopy::loadTask(TSharedConstTask Task)
{
    Q_ASSERT(m_pSettings);
    Q_ASSERT(!Task.isNull());

    ui->SrcList->clear();
    ui->SrcList->addItems(Task->SrcList);
    ui->DestList->clear();
    ui->DestList->addItems(Task->DestList);

    TTaskSettings* pTS = &m_pSettings->TaskSettings;
    *pTS = Task->TaskSettings;

    progressFormClosed();
}

//------------------------------------------------------------------------------
//! Обработчик сигнала сохранения задания.

void TMultiCopy::on_actionSaveTask_triggered()
{
    static const char* key = "SaveTaskPath";
    QSettings* pS = m_pSettings->getQSettings();
    QString FileName = QFileDialog::getSaveFileName(this,
                           tr("Save task"),
                           pS->value(key).toString(),
                           tr("Copy tasks (*.copy-task);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        saveTaskToFile(FileName);
    }
}

//------------------------------------------------------------------------------
//! Обработчик сигнала загрузки задания.

void TMultiCopy::on_actionLoadTask_triggered()
{
    static const char* key = "LoadTaskPath";
    QSettings* pS = m_pSettings->getQSettings();
    QString FileName = QFileDialog::getOpenFileName(this,
                           tr("Load task"),
                           pS->value(key).toString(),
                           tr("Copy tasks (*.copy-task);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        loadTaskFromFile(FileName);
    }
}

//------------------------------------------------------------------------------
//! Обработчик сигнала вызова формы общих настроек приложения.

void TMultiCopy::on_actionGeneralSettings_triggered()
{
    // Вызываем форму общих настроек.
    TSettingsForm* Form = new TSettingsForm(this);
    if (Form->exec() == QDialog::Accepted)
    {
        TGeneralSettings* pGS = &m_pSettings->GeneralSettings;
        ui->DestList->setCheckDirs(pGS->CheckDestDirs);
        ui->DestList->setCheckNetworkDirs(pGS->CheckNetworkDestDirs);

        // Обновление внешнего вида формы.
        viewChange();

        m_pProgressForm->viewChange();

        // Изменение языка форм выполнят обработчики события
        // QEvent::LanguageChange.
    }
}

//------------------------------------------------------------------------------
//! Обработчик сигнала вызова формы настроек задания.

void TMultiCopy::on_actionTaskSettings_triggered()
{
    // Вызываем форму настроек задания.
    TTaskSettingsForm* Form = new TTaskSettingsForm(this);
    Form->exec();
}

//------------------------------------------------------------------------------
//! Обработчик сигнала вызова окна статистики работы программы.

void TMultiCopy::on_actionStatistics_triggered()
{
    TGlobalStatistics* pGS = TGlobalStatistics::instance();
    TGlobalStatistics::TStat Stat = pGS->get();
    QString Text = tr("Readed: %1 from %n file(s)", "", Stat.FilesReaded)
                     .arg(sizeToStr(Stat.BytesReaded)) + "\n\n" +
                   tr("Writed: %1 to %n file(s)", "", Stat.FilesWrited)
                     .arg(sizeToStr(Stat.BytesWrited)) + "\n\n" +
                   tr("Working time: %1")
                     .arg(QTime().addSecs(Stat.WorkTime).toString("H:mm:ss")) + "\n\n" +
                   tr("Tasks completed: %1")
                     .arg(Stat.TasksCompleted);
    QMessageBox *pBox = new QMessageBox(this);
    pBox->setAttribute(Qt::WA_DeleteOnClose);;
    pBox->setWindowTitle(tr("%1 - Statistics").arg(QApplication::applicationName()));
    pBox->setText(Text);
    QPixmap Pixmap(":/MainIcon");
    pBox->setIconPixmap(Pixmap);
    pBox->exec();
}

//------------------------------------------------------------------------------
//! Обработчик события закрытия формы.

void TMultiCopy::closeEvent(QCloseEvent *Event)
{
    if (m_pProgressForm->exit()) {
        saveSession();
        Event->accept();
    }
    else {
        Event->ignore();
    }
}

//------------------------------------------------------------------------------
//! Обработчик смены состояний.

void TMultiCopy::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------
