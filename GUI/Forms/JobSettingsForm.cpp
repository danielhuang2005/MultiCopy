/*******************************************************************************
 *
 *            Copyright (С) 2012 Юрий Владимирович Круглов
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
 *                 Copyright (C) 2012 Yuri V. Krugloff
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

#include "JobSettingsForm.hpp"
#include "ui_JobSettingsForm.h"

#include <QMessageBox>

#include "MultiCopyForm.hpp"

//------------------------------------------------------------------------------

TJobSettingsForm::TJobSettingsForm()
    : QDialog(NULL),
      ui(new Ui::TJobSettingsForm)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    if (sizeof(void*) >= 8) {
        ui->CellSize->setMaximum(256);
        ui->CellsCount->setMaximum(256);
    }

    // TODO: � еализовать для UNIX.
    #ifndef Q_OS_WIN
        ui->LockMemory->setVisible(false);
        ui->NotUseCache->setVisible(false);
    #endif

    QIcon Icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
    QPixmap Pixmap = Icon.pixmap(128);
    ui->WarningIcon->setPixmap(Pixmap);

    readData(m_Settings.taskSettings());
    restoreSession();
}

//------------------------------------------------------------------------------

TJobSettingsForm::~TJobSettingsForm()
{
    saveSession();
    delete ui;
}

//------------------------------------------------------------------------------

void TJobSettingsForm::saveSession()
{
    if (result() == Accepted)
    {
        QSettings* pS = m_Settings.getQSettings();
        pS->beginGroup(objectName());
        pS->setValue("currentIndex", ui->tabWidget->currentIndex());
        pS->endGroup();
        m_Settings.write();
    }
}

//------------------------------------------------------------------------------

void TJobSettingsForm::restoreSession()
{
    QSettings* pS = m_Settings.getQSettings();
    pS->beginGroup(objectName());
    ui->tabWidget->setCurrentIndex(pS->value("currentIndex").toInt());
    pS->endGroup();
}

//------------------------------------------------------------------------------

void TJobSettingsForm::readData(const TTaskSettings* pJS)
{
    // Вкладка параметров копирования.
    ui->CopyDateTime->setChecked(pJS->CopyDateTime);
    ui->CopyAttr->setChecked(pJS->CopyAttr);
    ui->TotalCalc->setChecked(pJS->TotalCalc);
    ui->CheckFreeSpace->setChecked(pJS->CheckFreeSpace);
    ui->DirContentsOnly->setChecked(pJS->DirContentsOnly);
    ui->SubDirsDepth->setValue(pJS->SubDirsDepth);
    calculateRAM();

    // Вкладка дополнительных параметров.
    ui->AutodetectRAM->setChecked(pJS->BufferSizeAutoselect);
    ui->CellSize->setValue(pJS->RAMCellSize / (1024 * 1024));
    ui->CellsCount->setValue(pJS->RAMCellCount);
    ui->LockMemory->setChecked(pJS->LockMemory);
    ui->NotUseCache->setChecked(pJS->NotUseCache);
}

//------------------------------------------------------------------------------

void TJobSettingsForm::writeData(TTaskSettings* pJS)
{
    // Параметры копирования.
    pJS->CopyDateTime    = ui->CopyDateTime->isChecked();
    pJS->CopyAttr        = ui->CopyAttr->isChecked();
    pJS->TotalCalc       = ui->TotalCalc->isChecked();
    pJS->CheckFreeSpace  = ui->CheckFreeSpace->isChecked();
    pJS->DirContentsOnly = ui->DirContentsOnly->isChecked();
    pJS->SubDirsDepth    = ui->SubDirsDepth->value();

    // Дополнительные параметры.
    pJS->BufferSizeAutoselect = ui->AutodetectRAM->isChecked();
    pJS->RAMCellCount         = ui->CellsCount->value();
    pJS->RAMCellSize          = ui->CellSize->value() * 1024 * 1024;
    pJS->LockMemory           = ui->LockMemory->isChecked();
    pJS->NotUseCache          = ui->NotUseCache->isChecked();
}

//------------------------------------------------------------------------------

void TJobSettingsForm::calculateRAM()
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

void TJobSettingsForm::on_OK_clicked()
{
    writeData(m_Settings.taskSettings());
    m_Settings.write();
    accept();
}

//------------------------------------------------------------------------------
//! Установка значений по умолчанию.

void TJobSettingsForm::on_Default_clicked()
{
    if (QMessageBox::question(this,
                              "Job Settings",
                              "To set all parameters in their default values?",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        TTaskSettings JobSettings;
        JobSettings.setDefault();
        readData(&JobSettings);
    }
}

//------------------------------------------------------------------------------
