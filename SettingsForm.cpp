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

//------------------------------------------------------------------------------

TSettingsForm::TSettingsForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TSettingsForm)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    // Заполнение параметров.
    TSettingsData* pData = m_Settings.data();
    ui->CellSize->setValue(pData->RAMCellSize / (1024 * 1024));
    ui->CellsCount->setValue(pData->RAMCellCount);
    ui->CopyDateTime->setChecked(pData->CopyDateTime);
    ui->TotalCalc->setChecked(pData->TotalCalc);
    ui->DirContentsOnly->setChecked(pData->DirContentsOnly);
    ui->SubDirsDepth->setValue(pData->SubDirsDepth);
    calculateRAM();

    languagesList();
    QString CurrentLangID = m_Settings.langID();
    for (int i = 0; i < m_LangDefList.count(); ++i)
    {
        QString LangID = m_LangDefList[i].LangID;
        ui->Languages->addItem(m_LangDefList[i].LangName);
        if (LangID == CurrentLangID)
            ui->Languages->setCurrentIndex(i);
    }
}

//------------------------------------------------------------------------------

TSettingsForm::~TSettingsForm()
{
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

void TSettingsForm::calculateRAM()
{
    int mb = ui->CellSize->value() * ui->CellsCount->value();
    if (mb < 1024)
        ui->RequiredRAM->setText(QString::number(mb) + tr(" MB"));
    else {
        float gb = mb / 1024.0;
        ui->RequiredRAM->setText(QString::number(gb, 'g', 3) + tr(" GB"));
    }
}

//------------------------------------------------------------------------------

void TSettingsForm::on_OK_clicked()
{
    TSettingsData* pData = m_Settings.data();
    pData->RAMCellCount = ui->CellsCount->value();
    pData->RAMCellSize = ui->CellSize->value() * 1024 * 1024;
    pData->CopyDateTime = ui->CopyDateTime->isChecked();
    pData->TotalCalc = ui->TotalCalc->isChecked();
    pData->DirContentsOnly = ui->DirContentsOnly->isChecked();
    pData->SubDirsDepth = ui->SubDirsDepth->value();

    m_Settings.setLangID(m_LangDefList[ui->Languages->currentIndex()].LangID);

    m_Settings.write();
    close();
}

//------------------------------------------------------------------------------
