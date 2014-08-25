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

#include "MultiCopyForm.hpp"
#include "ui_MultiCopyForm.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "ControlThread.hpp"
#include "SettingsForm.hpp"
#include "ProgressForm.hpp"
#include "ErrorHandler.hpp"
#include "Translator.hpp"
#include "CommonFn.hpp"

//------------------------------------------------------------------------------
//! Перемещение элемента в списке типа QListWidget.
/*!
 * Метод перемещает элемент списка ListWidget, находящийся в позиции с индексом
 * Row, на Delta позиций. Delta может быть как положительной, так и
 * отрицательной. Если индекс Row неверный, ничего не происходит. Если новая
 * позиция элемента выходит за пределы индексов списка, производится коррекция.
 *
 * \remarks Подсветка устанавливается на перемещённый элемент.
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

TMultiCopy::TMultiCopy(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TMultiCopyForm)
{
    ui->setupUi(this);
    ui->SrcList->setDirsOnly(false);
    ui->DestList->setDirsOnly(true);
    ui->DestList->setCheckDirs(Settings.systemData()->CheckDirs);
    ui->DestList->setCheckNetworkDirs(Settings.systemData()->CheckNetworkDirs);

    setShowNameEditors(Settings.viewData()->ShowNameEditors);

    // "Сложные" соединения (дизайнер сделать не позволяет).
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    restoreSession();

    bool ShowIcons = Settings.viewData()->ShowFileIcons;
    ui->SrcList->setShowIcons(ShowIcons);
    ui->DestList->setShowIcons(ShowIcons);

    ShowIcons = Settings.viewData()->ShowNetworkIcons;
    ui->SrcList->setShowNetworkIcons(ShowIcons);
    ui->DestList->setShowNetworkIcons(ShowIcons);

    ui->SrcList->setCurrentRow(0);
    ui->DestList->setCurrentRow(0);
}

//------------------------------------------------------------------------------

TMultiCopy::~TMultiCopy()
{
    saveSession();
    delete ui;
}

//------------------------------------------------------------------------------

void TMultiCopy::srcChanged()
{
    // Кнопка "Remove".
    ui->SrcRemove->setEnabled(ui->SrcList->currentRow() >= 0);

    //Кнопка "Clear".
    ui->SrcClear->setEnabled(ui->SrcList->count() > 0);

    // Кнопка "Up".
    ui->SrcUp->setEnabled(ui->SrcList->currentRow() > 0);

    // Кнопка "Down".
    bool Enabled = ui->SrcList->currentRow() >= 0;
    if (Enabled)
        Enabled = ui->SrcList->currentRow() <
                  ui->SrcList->count() - 1;
    ui->SrcDown->setEnabled(Enabled);

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
    ui->DestRemove->setEnabled(ui->DestList->currentRow() >= 0);

    // Кнопка "Start".
    ui->Start->setEnabled((ui->SrcList->count() > 0) &&
                          (ui->DestList->count() > 0));
}

//------------------------------------------------------------------------------

void TMultiCopy::saveSession()
{
    QSettings* pS = Settings.getQSettings();
    //saveListsToSettings(pS);
    pS->beginGroup(objectName());
    pS->setValue(ui->SrcList->objectName(), ui->SrcList->toStringList());
    pS->setValue(ui->DestList->objectName(), ui->DestList->toStringList());
    pS->setValue("geometry", geometry());
    pS->setValue(ui->splitter->objectName(), ui->splitter->saveState());
    pS->endGroup();
    Settings.write();
}

//------------------------------------------------------------------------------

void TMultiCopy::restoreSession()
{
    QSettings* pS = Settings.getQSettings();
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
    // TODO : Обрабатывать ошибки.
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
    // TODO : Обрабатывать ошибки.
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

bool TMultiCopy::loadJobFromFile(const QString& FileName)
{
    // TODO : Обрабатывать ошибки.
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

bool TMultiCopy::saveJobToFile(const QString& FileName)
{
    // TODO : Обрабатывать ошибки.
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
/*!
 *
 * \remarks Форма настроек должна сама вызвать свой метод retranslateUi!
 */

void TMultiCopy::retranslateAllUi(QString LangID)
{
    loadTranslators(LangID);
    ui->retranslateUi(this);
    TProgressForm::retranslateUi();
}

//------------------------------------------------------------------------------

void TMultiCopy::loadListsFromSettings(QSettings* pS)
{
    if (pS == NULL)
        pS = Settings.getQSettings();
    pS->beginGroup(objectName());
    ui->SrcList->addItems(pS->value(ui->SrcList->objectName()).toStringList());
    ui->DestList->addItems(pS->value(ui->DestList->objectName()).toStringList());
    pS->endGroup();
}

//------------------------------------------------------------------------------
/*
void TMultiCopy::saveListsToSettings(QSettings* pS)
{
    if (pS == NULL)
        pS = Settings.getQSettings();
    pS->beginGroup(objectName());
    pS->setValue(ui->SrcList->objectName(), ui->SrcList->toStringList());
    pS->setValue(ui->DestList->objectName(), ui->DestList->toStringList());
    pS->endGroup();
}
*/
//------------------------------------------------------------------------------

void TMultiCopy::on_SrcAddFile_clicked()
{
    static const char* key = "SrcAddFile";
    QSettings* pS = Settings.getQSettings();
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
    QSettings* pS = Settings.getQSettings();
    QString Dir = QFileDialog::getExistingDirectory(this,
                      tr("Add source folder"),
                      pS->value(key).toString());
    if (!Dir.isEmpty())
    {
        pS->setValue(key, Dir);
        if (ui->SrcList->checkAndAddItem(Dir))
            srcChanged();
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_DestAddFolder_clicked()
{
    static const char* key = "DestAddFolder";
    QSettings* pS = Settings.getQSettings();
    QString Dir = QFileDialog::getExistingDirectory(this,
                      tr("Add destination folder"),
                      pS->value(key).toString());
    if (!Dir.isEmpty())
    {
        pS->setValue(key, Dir);
        ui->DestList->checkAndAddItem(Dir);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcRemove_clicked()
{
    delete ui->SrcList->item(ui->SrcList->currentRow());
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
    srcChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_Start_clicked()
{
    TProgressForm ProgressForm(this);
    TJob Job;
    Job.Srcs = ui->SrcList->toStringList();
    Job.Dests = ui->DestList->toStringList();
    Job.SettingsData = *Settings.copyData();
    ProgressForm.addJob(Job);
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcUp_clicked()
{
    int Index = ui->SrcList->currentRow();
    if (Index <= 0) return;

    QListWidgetItem* Item = ui->SrcList->takeItem(Index);
    --Index;
    ui->SrcList->insertItem(Index, Item);
    ui->SrcList->setCurrentRow(Index);
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcDown_clicked()
{
    int Index = ui->SrcList->currentRow();
    if ((Index < 0) || (Index >= ui->SrcList->count() - 1))
        return;

    QListWidgetItem* Item = ui->SrcList->takeItem(Index);
    ++Index;
    ui->SrcList->insertItem(Index, Item);
    ui->SrcList->setCurrentRow(Index);
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
    delete ui->DestList->item(ui->DestList->currentRow());
    destChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_DestList_currentRowChanged(int currentRow)
{
    destChanged();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_Settings_clicked()
{
    // Вызываем форму настроек.
    TSettingsForm* Form = new TSettingsForm(this);
    if (Form->exec() == QDialog::Accepted)
    {
        ui->DestList->setCheckDirs(Settings.systemData()->CheckDirs);
        ui->DestList->setCheckNetworkDirs(Settings.systemData()->CheckNetworkDirs);

        // Обновление внешнего вида формы.
        bool ShowIcons = Settings.viewData()->ShowNetworkIcons;
        ui->SrcList->setShowNetworkIcons(ShowIcons);
        ui->DestList->setShowNetworkIcons(ShowIcons);

        ShowIcons = Settings.viewData()->ShowFileIcons;
        ui->SrcList->setShowIcons(ShowIcons);
        ui->DestList->setShowIcons(ShowIcons);

        setShowNameEditors(Settings.viewData()->ShowNameEditors);
        QString LangID = Settings.langID();
        if (LangID.isEmpty())
            LangID = QLocale::system().name();
        loadTranslators(LangID);
        ui->retranslateUi(this);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionAbout_triggered()
{
    QString Text = tr(
        "<p align='center'>MultiCopy, version 1.2.0</p>"
    );
    QString InformativeText = tr(
        "<p align='center'>This product licensed under GNU GPL version 3. For details see "
        "<a href='http://www.gnu.org/copyleft/gpl.html'>http://www.gnu.org/copyleft/gpl.html</a>.</p>"
        "<p align='center'>Copyright &copy; 2011 Yuri&nbsp;Krugloff. "
        "All rights reserved.</p>"
        "<p align='center'><a href='http://www.tver-soft.ru'>http://www.tver-soft.ru</a></p>"
    );
    QMessageBox *pBox = new QMessageBox(this);
    pBox->setAttribute(Qt::WA_DeleteOnClose);;
    pBox->setWindowTitle(tr("MultiCopy - About"));
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
    QSettings* pS = Settings.getQSettings();
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
    QSettings* pS = Settings.getQSettings();
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
    QSettings* pS = Settings.getQSettings();
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
    QSettings* pS = Settings.getQSettings();
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

void TMultiCopy::on_actionSaveJob_triggered()
{
    static const char* key = "SaveJobPath";
    QSettings* pS = Settings.getQSettings();
    QString FileName = QFileDialog::getSaveFileName(this,
                           tr("Save job"),
                           pS->value(key).toString(),
                           tr("Jobs (*.job);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        //QSettings Ini(FileName, QSettings::IniFormat, this);
        //saveListsToSettings(&Ini);
        saveJobToFile(FileName);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionLoadJob_triggered()
{
    static const char* key = "LoadJobPath";
    QSettings* pS = Settings.getQSettings();
    QString FileName = QFileDialog::getOpenFileName(this,
                           tr("Load job"),
                           pS->value(key).toString(),
                           tr("Jobs (*.job);;All files (*.*)"));
    if (!FileName.isEmpty())
    {
        QFileInfo FileInfo(FileName);
        pS->setValue(key, FileInfo.absoluteFilePath());
        /*QSettings Ini(FileName, QSettings::IniFormat, this);
        ui->SrcList->clear();
        ui->DestList->clear();
        loadListsFromSettings(&Ini);*/
        loadJobFromFile(FileName);
    }
}

//------------------------------------------------------------------------------

void TMultiCopy::on_actionAddSubfolders_triggered()
{
    static const char* key = "AddSubfoldersPath";
    QSettings* pS = Settings.getQSettings();
    QString Dir = QFileDialog::getExistingDirectory(this,
                      tr ("Select folder"),
                      pS->value(key).toString());
    if (!Dir.isEmpty())
    {
        pS->setValue(key, Dir);
        QStringList List = SubfoldersList(Dir);
        if (List.count() > 0)
            ui->DestList->checkAndAddItems(&List);
    }
}

//------------------------------------------------------------------------------
