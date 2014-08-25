﻿/*******************************************************************************

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

#include "TaskSettingsForm.hpp"
#include "ui_TaskSettingsForm.h"

#include <QMessageBox>
#include <QSettings>

#include "GUI/Settings.hpp"

//------------------------------------------------------------------------------

TTaskSettingsForm::TTaskSettingsForm(QWidget *Parent)
    : QDialog(Parent),
      ui(new Ui::TTaskSettingsForm),
      m_pSettings(TSettings::instance())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    if (sizeof(void*) >= 8) {
        ui->CellSize->setMaximum(256);
        ui->CellsCount->setMaximum(256);
    }

    #ifndef Q_OS_WIN
        ui->FollowShortcuts->setVisible(false);
        ui->CopySystem->setVisible(false);

        // TODO: Реализовать для UNIX.
        ui->LockMemory->setVisible(false);
        ui->NoUseCache->setVisible(false);
    #endif

    QIcon Icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
    QPixmap Pixmap = Icon.pixmap(128);
    ui->WarningIcon->setPixmap(Pixmap);

    readData(&m_pSettings->TaskSettings);
    restoreSession();
}

//------------------------------------------------------------------------------

TTaskSettingsForm::~TTaskSettingsForm()
{
    saveSession();
    delete ui;
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::saveSession()
{
    if (result() == Accepted)
    {
        QSettings* pS = m_pSettings->getQSettings();
        pS->beginGroup(objectName());
        pS->setValue("currentIndex", ui->Tabs->currentIndex());
        pS->endGroup();
        m_pSettings->write();
    }
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::restoreSession()
{
    QSettings* pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    ui->Tabs->setCurrentIndex(pS->value("currentIndex").toInt());
    pS->endGroup();
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::readData_GeneralParams(const TTaskSettings* pTS)
{
    ui->TotalCalc->setChecked(pTS->TotalCalc);
    ui->CheckFreeSpace->setChecked(pTS->CheckFreeSpace);
    ui->SubDirsDepth->setValue(pTS->SubDirsDepth);
    ui->CopyDateTime->setChecked(pTS->CopyDateTime);
    ui->CopyAttr->setChecked(pTS->CopyAttr);
    ui->NoCreateRootDir->setChecked(pTS->NoCreateRootDir);
    ui->CopyEmptyDirs->setChecked(pTS->CopyEmptyDirs);
    ui->CopyHidden->setChecked(pTS->CopyHidden);
    ui->CopySystem->setChecked(pTS->CopySystem);
    ui->FollowShortcuts->setChecked(pTS->FollowShortcuts);
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::readData_RAMParams(const TTaskSettings *pTS)
{
    ui->AutodetectRAM->setChecked(pTS->BufferSizeAutoselect);
    ui->CellSize->setValue(pTS->RAMCellSize / (1024 * 1024));
    ui->CellsCount->setValue(pTS->RAMCellCount);
    ui->LockMemory->setChecked(pTS->LockMemory);
    ui->NoUseCache->setChecked(pTS->NoUseCache);
    calculateRAM();
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::readData(const TTaskSettings* pTS)
{
    readData_GeneralParams(pTS);
    readData_RAMParams(pTS);
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::writeData_GeneralParams(TTaskSettings* pTS)
{
    pTS->TotalCalc       = ui->TotalCalc->isChecked();
    pTS->CheckFreeSpace  = ui->CheckFreeSpace->isChecked();
    pTS->SubDirsDepth    = ui->SubDirsDepth->value();
    pTS->CopyDateTime    = ui->CopyDateTime->isChecked();
    pTS->CopyAttr        = ui->CopyAttr->isChecked();
    pTS->NoCreateRootDir = ui->NoCreateRootDir->isChecked();
    pTS->CopyEmptyDirs   = ui->CopyEmptyDirs->isChecked();
    pTS->CopyHidden      = ui->CopyHidden->isChecked();
    pTS->CopySystem      = ui->CopySystem->isChecked();
    pTS->FollowShortcuts = ui->FollowShortcuts->isChecked();
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::writeData_RAMParams(TTaskSettings* pTS)
{
    pTS->BufferSizeAutoselect = ui->AutodetectRAM->isChecked();
    pTS->RAMCellCount         = ui->CellsCount->value();
    pTS->RAMCellSize          = ui->CellSize->value() * 1024 * 1024;
    pTS->LockMemory           = ui->LockMemory->isChecked();
    pTS->NoUseCache           = ui->NoUseCache->isChecked();
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::writeData(TTaskSettings* pTS)
{
    writeData_GeneralParams(pTS);
    writeData_RAMParams(pTS);
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::calculateRAM()
{
    int mb = ui->CellSize->value() * ui->CellsCount->value();
    if (mb < 1024) {
        ui->RequiredRAM->setText(QString::number(mb) + tr(" MB"));
    }
    else {
        float gb = mb / 1024.0;
        ui->RequiredRAM->setText(QString::number(gb, 'g', 3) + tr(" GB"));
    }
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::on_OK_clicked()
{
    writeData(&m_pSettings->TaskSettings);
    m_pSettings->write();
    accept();
}

//------------------------------------------------------------------------------
//! Установка значений по умолчанию.

void TTaskSettingsForm::on_Default_clicked()
{
    if (QMessageBox::question(this,
            tr("Task Settings"),
            tr("Do set parameters on this tab to their default values?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No)
        == QMessageBox::Yes)
    {
        TTaskSettings TaskSettings;
        switch (ui->Tabs->currentIndex()) {
            case 0 :
                readData_GeneralParams(&TaskSettings);
                break;
            case 1 :
                readData_RAMParams(&TaskSettings);
                break;
            default :
                qWarning("TTaskSettingsForm::on_Default_clicked. "
                         "Unhandled tab index (%i).",
                         ui->Tabs->currentIndex());
        }
    }
}

//------------------------------------------------------------------------------

void TTaskSettingsForm::changeEvent(QEvent *e)
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
