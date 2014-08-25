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
#include <QFileIconProvider>
#include <QMessageBox>
#include <QMenu>
#include <QStringBuilder>

#include "ControlThread.hpp"
#include "SettingsForm.hpp"
#include "ProgressForm.hpp"
#include "ErrorHandler.hpp"

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

QStringList ListWidgetToStrings(QListWidget* List)
{
    QStringList Strings;
    for (int i =  0; i < List->count(); ++i)
        Strings.append(List->item(i)->text());
    return Strings;
}

//------------------------------------------------------------------------------
//! "Укорачивание" длинных строк.
/*!
 * Если строка Str длиннее чем MaxSymbols, возвращает строку длиной не более
 * MaxSymbols, полученную из Str заменой лишних символов в середине на
 * многоточие ("..."). Если MaxSymbols <= 3, вернёт только многоточие.
 */

QString ElideText(const QString& Str, int MaxSymbols)
{
    if (MaxSymbols <= 3)
        return "...";

    if (Str.length() <= MaxSymbols)
        return Str;

    int n = (MaxSymbols - 3) / 2;
    return Str.left(n) % QString("...") % Str.right(n);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TMultiCopy::TMultiCopy(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TMultiCopyForm)
{
    ui->setupUi(this);

    QMenu* pMenu = new QMenu(this);
    pMenu->addAction(tr("About"), this, SLOT(about()));
    pMenu->addAction(tr("About Qt"), qApp, SLOT(aboutQt()));
    ui->About->setMenu(pMenu);

    restoreSession();
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

void TMultiCopy::addItemWithIcon(QListWidget* List, const QString& FileName)
{
    QFileInfo FileInfo(FileName);
    QFileIconProvider FileIconProvider;
    List->addItem(new QListWidgetItem(FileIconProvider.icon(FileInfo),
                          QDir::toNativeSeparators(FileName)));
}

//------------------------------------------------------------------------------

void TMultiCopy::addItemsWithIcon(QListWidget* List,
                                  const QStringList& FileNames)
{
    for (int i = 0; i < FileNames.count(); ++i)
        addItemWithIcon(List, FileNames[i]);
}

//------------------------------------------------------------------------------

void TMultiCopy::saveSession()
{
    QSettings* pS = Settings.getQSettings();
    pS->beginGroup(objectName());
    pS->setValue(ui->SrcList->objectName(), ListWidgetToStrings(ui->SrcList));
    pS->setValue(ui->DestList->objectName(), ListWidgetToStrings(ui->DestList));
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
    addItemsWithIcon(ui->SrcList, pS->value(ui->SrcList->objectName()).toStringList());
    addItemsWithIcon(ui->DestList, pS->value(ui->DestList->objectName()).toStringList());
    QRect Geometry = pS->value("geometry").toRect();
    if (!Geometry.isEmpty())
        setGeometry(Geometry);
    ui->splitter->restoreState(pS->value(ui->splitter->objectName()).toByteArray());
    pS->endGroup();

    srcChanged();
    destChanged();
}

//------------------------------------------------------------------------------

bool TMultiCopy::testNewSource(QStringList* pSrc)
{
    /*QMessageBox::StandardButtons Buttons = QMessageBox::Ok;
    if (pSrc->count() > 1)
        Buttons |= QMessageBox::Cancel;*/

    QStringList Dup;
    for (QStringList::iterator I = pSrc->begin(); I != pSrc->end(); ++I)
    {
        for (int j = 0; j < ui->SrcList->count(); ++j)
        {
            if (ui->SrcList->item(j)->text() == *I)
            {
                /*QMessageBox::StandardButton B = QMessageBox::warning(
                    this,
                    QApplication::applicationName(),
                    tr("This source already added.") + "\n\n" + *I,
                    Buttons,
                    QMessageBox::Ok);
                if (B == QMessageBox::Ok)
                {
                    pSrc->removeAll(*I);
                    break;
                }
                else
                    return false;*/
                Dup << *I;
                break;
            }
        }
    }

    if (Dup.count() > 0)
    {
        // Есть дублируемые элементы.
        QString Names;
        const int MaxOutputNames = 4;  //!< Максимальное число выводимых строк.
        for (int i = 0; i < qMin(Dup.count(), MaxOutputNames); ++i)
        {
            Names += ElideText(Dup[i], 40) + "\n";
        }
        if (Dup.count() > MaxOutputNames)
            Names += "..........";

        // Выводим сообщение пользователю.
        QMessageBox::StandardButton  DefButton = QMessageBox::NoButton;
        QMessageBox::StandardButtons Buttons = QMessageBox::NoButton;
        QString Message = tr("Some sources already added:") + "\n\n" + Names;
        if ((pSrc->count() > 1) && (Dup.count() < pSrc->count()))
        {
            DefButton = QMessageBox::Yes;
            Buttons   = QMessageBox::Yes | QMessageBox::No;
            Message  += "\n\n" + tr("They won't be added again. Continue?");
        }
        else {
            DefButton = QMessageBox::Ok;
            Buttons   = DefButton;
        }
        QMessageBox::StandardButton B = QMessageBox::warning(
                    this,
                    QApplication::applicationName(),
                    Message,
                    Buttons,
                    DefButton);
        if (B == QMessageBox::No)
            return false;

        for (int i = Dup.count() - 1; i >= 0; --i)
            pSrc->removeAll(Dup[i]);
    }

    return true;
}

//------------------------------------------------------------------------------

bool TMultiCopy::testNewSource(const QString& Src)
{
    QStringList List(Src);
    return testNewSource(&List) && !List.isEmpty();
}

//------------------------------------------------------------------------------

void TMultiCopy::on_SrcAddFile_clicked()
{
    static const char* key = "SrcAddFile";
    QSettings* pS = Settings.getQSettings();
    QStringList FileNames = QFileDialog::getOpenFileNames(this,
                                tr("Add source file(s)"),
                                pS->value(key).toString());
    if (testNewSource(&FileNames) && (FileNames.count() > 0))
    {
        QFileInfo FileInfo(FileNames[0]);
        pS->setValue(key, FileInfo.absoluteDir().absolutePath());
        addItemsWithIcon(ui->SrcList, FileNames);
        srcChanged();
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
    if (!Dir.isEmpty() && testNewSource(Dir))
    {
        pS->setValue(key, Dir);
        addItemWithIcon(ui->SrcList, Dir);
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

        // Проверка дублирования элементов.
        for (int i = ui->DestList->count() - 1; i >= 0; --i)
            if (ui->DestList->item(i)->text() == Dir)
            {
                QMessageBox::warning(this, QApplication::applicationName(),
                                     tr("Folder already added."));
                return;
            }

        addItemWithIcon(ui->DestList, Dir);
        destChanged();
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
    Job.Srcs = ListWidgetToStrings(ui->SrcList);
    Job.Dests = ListWidgetToStrings(ui->DestList);
    Job.SettingsData = *Settings.data();
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
    if ((Index <= 0) || (Index >= ui->SrcList->count() - 1))
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
    TSettingsForm* Form = new TSettingsForm(this);
    Form->show();
}

//------------------------------------------------------------------------------

void TMultiCopy::about()
{
    QString Text = tr(
        "<p align='center'>MultiCopy, version 1.0.0</p>"
    );
    QString InformativeText = tr(
        "<p>This product licensed under GNU GPL version 3. For details see "
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
