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

#include "SettingsForm.hpp"
#include "ui_SettingsForm.h"

#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>

#include "GUI/Translator.hpp"
#include "GUI/Settings.hpp"

//------------------------------------------------------------------------------

TSettingsForm::TSettingsForm(QWidget* Parent)
    : QDialog(Parent),
      ui(new Ui::TSettingsForm),
      m_pSettings(TSettings::instance())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    //setWindowTitle(windowTitle().arg(QApplication::applicationName()));
    retranslateTitle();

    languagesList();
    QString CurrentLangID = m_pSettings->langID();
    for (int i = 0; i < m_LangDefList.count(); ++i)
    {
        ui->Languages->addItem(m_LangDefList[i].LangName);
        if (m_LangDefList[i].LangID == CurrentLangID)
            ui->Languages->setCurrentIndex(i);
    }

    readData(&m_pSettings->GeneralSettings);
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
    QStringList LangPath = langPaths();
    m_LangDefList.append(TLangDef("", tr("Autodetect",
                                         "Specify here language name")));
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
        QSettings* pS = m_pSettings->getQSettings();
        pS->beginGroup(objectName());
        pS->setValue("currentIndex", ui->Tabs->currentIndex());
        pS->endGroup();
        m_pSettings->write();
    }
}

//------------------------------------------------------------------------------

void TSettingsForm::restoreSession()
{
    QSettings* pS = m_pSettings->getQSettings();
    pS->beginGroup(objectName());
    ui->Tabs->setCurrentIndex(pS->value("currentIndex").toInt());
    pS->endGroup();
}

//------------------------------------------------------------------------------

void TSettingsForm::readData_ViewParams(const TGeneralSettings* pGS)
{
    // Вкладка параметров внешнего вида.
    ui->ShowFileIcons->setChecked(pGS->ShowFileIcons);
    ui->ShowNetworkIcons->setChecked(pGS->ShowNetworkIcons);
    ui->ShowNameEditors->setChecked(pGS->ShowNameEditors);
    ui->FlatToolButtons->setChecked(pGS->FlatToolButtons);
    ui->ToolButtonStyle->setCurrentIndex(pGS->ToolButtonStyle);
}

//------------------------------------------------------------------------------

void TSettingsForm::readData_SystemParams(const TGeneralSettings* pGS)
{
    // Вкладка системных параметров.
    ui->SingleInstance->setChecked(pGS->SingleInstance);
    ui->CheckDestDirs->setChecked(pGS->CheckDestDirs);
    ui->CheckDestNetworkDirs->setChecked(pGS->CheckNetworkDestDirs);
}

//------------------------------------------------------------------------------

void TSettingsForm::readData(const TGeneralSettings* pGS)
{
    readData_ViewParams(pGS);
    readData_SystemParams(pGS);
}

//------------------------------------------------------------------------------

void TSettingsForm::writeData_ViewParams(TGeneralSettings* pGS)
{
    // Параметры внешнего вида.
    m_pSettings->setLangID(m_LangDefList[ui->Languages->currentIndex()].LangID);
    pGS->ShowFileIcons    = ui->ShowFileIcons->isChecked();
    pGS->ShowNetworkIcons = ui->ShowNetworkIcons->isChecked();
    pGS->ShowNameEditors  = ui->ShowNameEditors->isChecked();
    pGS->FlatToolButtons  = ui->FlatToolButtons->isChecked();
    pGS->ToolButtonStyle  = ui->ToolButtonStyle->currentIndex();
}

//------------------------------------------------------------------------------

void TSettingsForm::writeData_SystemParams(TGeneralSettings* pGS)
{
    // Системные параметры.
    pGS->SingleInstance       = ui->SingleInstance->isChecked();
    pGS->CheckDestDirs        = ui->CheckDestDirs->isChecked();
    pGS->CheckNetworkDestDirs = ui->CheckDestNetworkDirs->isChecked();
}

//------------------------------------------------------------------------------

void TSettingsForm::writeData(TGeneralSettings* pGS)
{
    writeData_ViewParams(pGS);
    writeData_SystemParams(pGS);
}

//------------------------------------------------------------------------------
//! Перевод заголовка окна.
/*!
   Метод осуществляет перевод заголовка окна. Должен вызываться после
   retranslateUi.
 */

void TSettingsForm::retranslateTitle()
{
    setWindowTitle(windowTitle().arg(QApplication::applicationName()));
}

//------------------------------------------------------------------------------

void TSettingsForm::closeEvent(QCloseEvent* Event)
{
    Q_UNUSED(Event);
    reject();
}

//------------------------------------------------------------------------------

void TSettingsForm::on_Default_clicked()
{
    if (QMessageBox::question(this,
                              tr("General Settings"),
                              tr("Do set parameters on this tab to their default values?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        TGeneralSettings GeneralSettings;
        switch (ui->Tabs->currentIndex()) {
            case 0 :
                readData_ViewParams(&GeneralSettings);
                ui->Languages->setCurrentIndex(0);
                break;
            case 1 :
                readData_SystemParams(&GeneralSettings);
                break;
            default :
                qWarning("TSettingsForm::on_Default_clicked. "
                         "Unhandled tab index (%i).",
                         ui->Tabs->currentIndex());
        }
    }
}

//------------------------------------------------------------------------------
//! Обработчик изменения языка приложения.

void TSettingsForm::on_Languages_currentIndexChanged(int index)
{
    Q_UNUSED(index);

    QString LangId = m_LangDefList[ui->Languages->currentIndex()].LangID;
    loadTranslators(LangId);
}

//------------------------------------------------------------------------------

void TSettingsForm::accept()
{
    writeData(&m_pSettings->GeneralSettings);
    m_pSettings->write();

    QDialog::accept();
}

//------------------------------------------------------------------------------

void TSettingsForm::reject()
{
    // Возвращаем настройки языка.
    QString LangId = m_LangDefList[ui->Languages->currentIndex()].LangID;
    if (LangId != m_pSettings->langID())
        loadTranslators(m_pSettings->langID());

    QDialog::reject();
}

//------------------------------------------------------------------------------

void TSettingsForm::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            retranslateTitle();
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------
