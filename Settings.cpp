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
    m_pData.RAMCellSize     = m_pSettings->value("RAMCellSize",     16*1024*1024).toInt();
    m_pData.RAMCellCount    = m_pSettings->value("RAMCellCount",    4).toInt();
    m_pData.CopyDateTime    = m_pSettings->value("CopyDateTime",    true).toBool();
    m_pData.TotalCalc       = m_pSettings->value("TotalCalc",       true).toBool();
    m_pData.DirContentsOnly = m_pSettings->value("DirContentsOnly", false).toBool();
    m_pData.SubDirsDepth    = m_pSettings->value("SubDirsDepth",    -1).toInt();
    m_pSettings->endGroup();
}

//------------------------------------------------------------------------------

void TSettingsPrivate::write()
{
    m_pSettings->beginGroup("MultiCopy");
    m_pSettings->setValue("RAMCellSize",     m_pData.RAMCellSize);
    m_pSettings->setValue("RAMCellCount",    m_pData.RAMCellCount);
    m_pSettings->setValue("CopyDateTime",    m_pData.CopyDateTime);
    m_pSettings->setValue("TotalCalc",       m_pData.TotalCalc);
    m_pSettings->setValue("DirContentsOnly", m_pData.DirContentsOnly);
    m_pSettings->setValue("SubDirsDepth",    m_pData.SubDirsDepth);
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
