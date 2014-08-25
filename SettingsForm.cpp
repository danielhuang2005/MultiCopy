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

#include "SettingsForm.hpp"
#include "ui_SettingsForm.h"

#include <QDir>
#include <QTranslator>

#include "Translator.hpp"
#include "MultiCopyForm.hpp"

//------------------------------------------------------------------------------

TSettingsForm::TSettingsForm(TMultiCopy *parent) :
    QDialog(parent),
    ui(new Ui::TSettingsForm),
    m_pParent(parent)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    #ifndef Q_OS_WIN
        ui->LockMemory->setVisible(false);
        ui->NotUseCache->setVisible(false);
    #endif

    QIcon Icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
    QPixmap Pixmap = Icon.pixmap(128);
    ui->WarningIcon->setPixmap(Pixmap);

    // Указатели на структуры данных.
    TCopyData*   pCopyData   = m_Settings.copyData();
    TViewData*   pViewData   = m_Settings.viewData();
    TSystemData* pSystemData = m_Settings.systemData();

    // Вкладка параметров копирования.
    ui->CopyDateTime->setChecked(pCopyData->CopyDateTime);
    ui->CopyAttr->setChecked(pCopyData->CopyAttr);
    ui->TotalCalc->setChecked(pCopyData->TotalCalc);
    ui->CheckFreeSpace->setChecked(pCopyData->CheckFreeSpace);
    ui->DirContentsOnly->setChecked(pCopyData->DirContentsOnly);
    ui->SubDirsDepth->setValue(pCopyData->SubDirsDepth);
    calculateRAM();

    // Вкладка параметров внешнего вида.
    languagesList();
    QString CurrentLangID = m_Settings.langID();
    for (int i = 0; i < m_LangDefList.count(); ++i)
    {
        ui->Languages->addItem(m_LangDefList[i].LangName);
        if (m_LangDefList[i].LangID == CurrentLangID)
            ui->Languages->setCurrentIndex(i);
    }
    ui->ShowFileIcons->setChecked(pViewData->ShowFileIcons);
    ui->ShowNetworkIcons->setChecked(pViewData->ShowNetworkIcons);
    on_ShowFileIcons_clicked(pViewData->ShowFileIcons);
    ui->ShowNameEditors->setChecked(pViewData->ShowNameEditors);

    // Вкладка системных параметров.
    ui->CheckDestDirs->setChecked(pSystemData->CheckDirs);
    ui->CheckDestNetworkDirs->setChecked(pSystemData->CheckNetworkDirs);
    on_CheckDestDirs_clicked(pSystemData->CheckDirs);

    // Вкладка дополнительных параметров.
    ui->AutodetectRAM->setChecked(pCopyData->RAMAutodetect);
    ui->CellSize->setValue(pCopyData->RAMCellSize / (1024 * 1024));
    ui->CellsCount->setValue(pCopyData->RAMCellCount);
    ui->LockMemory->setChecked(pCopyData->LockMemory);
    ui->NotUseCache->setChecked(pCopyData->NotUseCache);

    restoreSession();
}

//------------------------------------------------------------------------------

TSettingsForm::~TSettingsForm()
{
    saveSession();
    delete ui;
}

//------------------------------------------------------------------------------

void TSettingsForm::languagesList()
{
    QStringList LangPath = langPaths();       // Пути к файлам перевода.
    m_LangDefList.append(TLangDef("", tr("Autodetect")));
    m_LangDefList.append(TLangDef("en_US", "English (embedded)"));
    for (int i = 0; i < LangPath.count(); ++i)
    {
        QDir Dir(LangPath[i]);
        QStringList Files = Dir.entryList(QStringList(QApplication::applicationName()+ ".*.qm"),
                                          QDir::Files, QDir::Name);
        QTranslator Translator;
        for (int j = 0; j < Files.count(); ++j)
        {
            if (Translator.load(Dir.filePath(Files[j])))
            {
                TLangDef LangDef;
                LangDef.LangName = Translator.translate("LanguageDef", "English");
                LangDef.LangID = Translator.translate("LanguageDef", "en_US");
                m_LangDefList.append(LangDef);
            }
        }
    }
}

//------------------------------------------------------------------------------

void TSettingsForm::saveSession()
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

void TSettingsForm::restoreSession()
{
    QSettings* pS = m_Settings.getQSettings();
    pS->beginGroup(objectName());
    ui->tabWidget->setCurrentIndex(pS->value("currentIndex").toInt());
    pS->endGroup();
}

//------------------------------------------------------------------------------

void TSettingsForm::calculateRAM()
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

void TSettingsForm::on_OK_clicked()
{
    // Указатели на структуры данных.
    TCopyData*   pCopyData   = m_Settings.copyData();
    TViewData*   pViewData   = m_Settings.viewData();
    TSystemData* pSystemData = m_Settings.systemData();

    // Параметры копирования.
    pCopyData->CopyDateTime    = ui->CopyDateTime->isChecked();
    pCopyData->CopyAttr        = ui->CopyAttr->isChecked();
    pCopyData->TotalCalc       = ui->TotalCalc->isChecked();
    pCopyData->CheckFreeSpace  = ui->CheckFreeSpace->isChecked();
    pCopyData->DirContentsOnly = ui->DirContentsOnly->isChecked();
    pCopyData->SubDirsDepth    = ui->SubDirsDepth->value();

    // Параметры внешнего вида.
    m_Settings.setLangID(m_LangDefList[ui->Languages->currentIndex()].LangID);
    pViewData->ShowFileIcons    = ui->ShowFileIcons->isChecked();
    pViewData->ShowNetworkIcons = ui->ShowNetworkIcons->isChecked();
    pViewData->ShowNameEditors  = ui->ShowNameEditors->isChecked();

    // Системные параметры.
    pSystemData->CheckDirs        = ui->CheckDestDirs->isChecked();
    pSystemData->CheckNetworkDirs = ui->CheckDestNetworkDirs->isChecked();

    // Дополнительные параметры.
    pCopyData->RAMAutodetect = ui->AutodetectRAM->isChecked();
    pCopyData->RAMCellCount  = ui->CellsCount->value();
    pCopyData->RAMCellSize   = ui->CellSize->value() * 1024 * 1024;
    pCopyData->LockMemory    = ui->LockMemory->isChecked();
    pCopyData->NotUseCache   = ui->NotUseCache->isChecked();

    m_Settings.write();
    accept();
}

//------------------------------------------------------------------------------

void TSettingsForm::on_ShowFileIcons_clicked(bool checked)
{
    ui->ShowNetworkIcons->setEnabled(checked);
    if (!checked)
        ui->ShowNetworkIcons->setChecked(false);
}

//------------------------------------------------------------------------------

void TSettingsForm::on_CheckDestDirs_clicked(bool checked)
{
    ui->CheckDestNetworkDirs->setEnabled(checked);
    if (!checked)
        ui->CheckDestNetworkDirs->setChecked(false);
}

//------------------------------------------------------------------------------
/*
void TSettingsForm::on_AutodetectRAM_clicked(bool checked)
{
    ui->RAMBox->setEnabled(!checked);
}
*/
//------------------------------------------------------------------------------
