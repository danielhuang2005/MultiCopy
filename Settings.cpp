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

#include "Settings.hpp"
#include "MultiCopyForm.hpp"

#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QDir>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                  T S e t t i n g s P r i v a t e
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TSettingsPrivate* TSettingsPrivate::m_pInstance = NULL;
QAtomicInt TSettingsPrivate::m_Refs = 0;


//------------------------------------------------------------------------------

TSettingsPrivate::TSettingsPrivate()
{
    QString IniFile = QApplication::applicationFilePath() + ".portable";
    if (QFile::exists(IniFile))
    {
        IniFile = QApplication::applicationDirPath() + QDir::separator() +
                  QApplication::applicationName() + ".settings";
        m_pSettings = new QSettings(IniFile, QSettings::IniFormat);
    }
    else {
        m_pSettings = new QSettings(QSettings::IniFormat,
                                    QSettings::UserScope,
                                    QApplication::organizationName(),
                                    QApplication::applicationName());
    }
    read();
}

//------------------------------------------------------------------------------

TSettingsPrivate::~TSettingsPrivate()
{
    write();
    delete m_pSettings;
}

//------------------------------------------------------------------------------

TSettingsPrivate* TSettingsPrivate::create()
{
    if (m_Refs == 0)
        m_pInstance = new TSettingsPrivate();
    m_Refs.ref();
    return m_pInstance;
}

//------------------------------------------------------------------------------

void TSettingsPrivate::release()
{
    if (m_Refs > 0)
        if (!m_Refs.deref())
            delete m_pInstance;
}

//------------------------------------------------------------------------------

void TSettingsPrivate::read()
{
    m_pSettings->beginGroup("MultiCopy");
    m_CopyData.RAMAutodetect   = m_pSettings->value("RAMAutodetect",   true).toBool();
    m_CopyData.RAMCellSize     = m_pSettings->value("RAMCellSize",     16*1024*1024).toInt();
    m_CopyData.RAMCellCount    = m_pSettings->value("RAMCellCount",    4).toInt();
    m_CopyData.LockMemory      = m_pSettings->value("LockMemory",      false).toBool();
    m_CopyData.NotUseCache     = m_pSettings->value("NotUseCache",     true).toBool();
    m_CopyData.CopyDateTime    = m_pSettings->value("CopyDateTime",    true).toBool();
    m_CopyData.CopyAttr        = m_pSettings->value("CopyAttr",        true).toBool();
    m_CopyData.TotalCalc       = m_pSettings->value("TotalCalc",       true).toBool();
    m_CopyData.CheckFreeSpace  = m_pSettings->value("CheckFreeSpace",  true).toBool();
    m_CopyData.DirContentsOnly = m_pSettings->value("DirContentsOnly", false).toBool();
    m_CopyData.SubDirsDepth    = m_pSettings->value("SubDirsDepth",    -1).toInt();
    m_pSettings->endGroup();

    m_pSettings->beginGroup("ViewData");
    m_ViewData.ShowFileIcons    = m_pSettings->value("ShowFileIcons",    true).toBool();
    m_ViewData.ShowNetworkIcons = m_pSettings->value("ShowNetworkIcons", false).toBool();
    m_ViewData.ShowNameEditors  = m_pSettings->value("ShowNameEditors",  false).toBool();
    m_pSettings->endGroup();

    m_pSettings->beginGroup("SystemData");
    m_SystemData.CheckDirs        = m_pSettings->value("CheckDirs",        true).toBool();
    m_SystemData.CheckNetworkDirs = m_pSettings->value("CheckNetworkDirs", false).toBool();
    m_pSettings->endGroup();
}

//------------------------------------------------------------------------------

void TSettingsPrivate::write()
{
    m_pSettings->beginGroup("MultiCopy");
    m_pSettings->setValue("RAMAutodetect",   m_CopyData.RAMAutodetect);
    m_pSettings->setValue("RAMCellSize",     m_CopyData.RAMCellSize);
    m_pSettings->setValue("RAMCellCount",    m_CopyData.RAMCellCount);
    m_pSettings->setValue("LockMemory",      m_CopyData.LockMemory);
    m_pSettings->setValue("NotUseCache",     m_CopyData.NotUseCache);
    m_pSettings->setValue("CopyDateTime",    m_CopyData.CopyDateTime);
    m_pSettings->setValue("CopyAttr",        m_CopyData.CopyAttr);
    m_pSettings->setValue("TotalCalc",       m_CopyData.TotalCalc);
    m_pSettings->setValue("CheckFreeSpace",  m_CopyData.CheckFreeSpace);
    m_pSettings->setValue("DirContentsOnly", m_CopyData.DirContentsOnly);
    m_pSettings->setValue("SubDirsDepth",    m_CopyData.SubDirsDepth);
    m_pSettings->endGroup();

    m_pSettings->beginGroup("ViewData");
    m_pSettings->setValue("ShowFileIcons",    m_ViewData.ShowFileIcons);
    m_pSettings->setValue("ShowNetworkIcons", m_ViewData.ShowNetworkIcons);
    m_pSettings->setValue("ShowNameEditors",  m_ViewData.ShowNameEditors);
    m_pSettings->endGroup();

    m_pSettings->beginGroup("SystemData");
    m_pSettings->setValue("CheckDirs",        m_SystemData.CheckDirs);
    m_pSettings->setValue("CheckNetworkDirs", m_SystemData.CheckNetworkDirs);
    m_pSettings->endGroup();

    m_pSettings->sync();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                        T S e t t i n g s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TSettings::TSettings()
    : p(TSettingsPrivate::create())
{
}

//------------------------------------------------------------------------------

TSettings::~TSettings()
{
    p->release();
}

//------------------------------------------------------------------------------

QString TSettings::langID()
{
    return p->m_pSettings->value("Language").toString();
}

//------------------------------------------------------------------------------

void TSettings::setLangID(const QString& ID)
{
    p->m_pSettings->setValue("Language", ID);
}

//------------------------------------------------------------------------------
