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

#include "Settings.hpp"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QWidget>
#include <QToolButton>

#include "../../Core/Resources.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define WRITE(Field) \
    pS->setValue(#Field, Field);

#define READ(Field, Type) \
    Field = pS->value(#Field, Field).value<Type>();

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Чтение настроек.
/*!
   \arg pS Указатель на экземпляр объекта для хранения настроек.
   \arg Group Имя группы для хранения настроек.
 */

void TTaskSettings2::read(QSettings *pS, const QString &Group)
{
    Q_ASSERT(pS != NULL);

    if (!Group.isEmpty())
        pS->beginGroup(Group);

    READ(BufferSizeAutoselect, bool);
    READ(RAMCellSize,          int);
    READ(RAMCellCount,         int);
    READ(LockMemory,           bool);
    READ(NoUseCache,           bool);
    READ(NoUseCacheFor,        qint64);
    READ(TotalCalc,            bool);
    READ(CheckFreeSpace,       bool)
    READ(NoCreateRootDir,      bool);
    READ(SubDirsDepth,         int);
    READ(CopyDateTime,         bool);
    READ(CopyAttr,             bool);
    READ(CopyHidden,           bool);
    READ(CopySystem,           bool);
    READ(FollowShortcuts,      bool);
    READ(CopyEmptyDirs,        bool);

    if (!Group.isEmpty())
        pS->endGroup();
}

//------------------------------------------------------------------------------
//! Запись настроек.
/*!
   \arg pS Указатель на экземпляр объекта для хранения настроек.
   \arg Group Имя группы для хранения настроек.
 */

void TTaskSettings2::write(QSettings *pS, const QString &Group)
{
    Q_ASSERT(pS != NULL);

    if (!Group.isEmpty())
        pS->beginGroup(Group);

    WRITE(BufferSizeAutoselect);
    WRITE(RAMCellSize);
    WRITE(RAMCellCount);
    WRITE(LockMemory);
    WRITE(NoUseCache);
    WRITE(NoUseCacheFor);
    WRITE(TotalCalc);
    WRITE(CheckFreeSpace);
    WRITE(NoCreateRootDir);
    WRITE(SubDirsDepth);
    WRITE(CopyDateTime);
    WRITE(CopyAttr);
    WRITE(CopyHidden);
    WRITE(CopySystem);
    WRITE(FollowShortcuts);
    WRITE(CopyEmptyDirs);

    if (!Group.isEmpty())
        pS->endGroup();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Установка в значения по умолчанию.

void TGeneralSettings::setDefault()
{
    SingleInstance          = true;
    ShowFileIcons           = true;
    ShowNetworkIcons        = false;
    ShowNameEditors         = false;
    CheckDestDirs           = true;
    CheckNetworkDestDirs    = false;
    FlatToolButtons         = true;
    ToolButtonStyle         = static_cast<int>(Qt::ToolButtonIconOnly);
    AfterStartClearSrcList  = false;
    AfterStartClearDestList = false;
}

//------------------------------------------------------------------------------
//! Чтение настроек.
/*!
   \arg pS Указатель на экземпляр объекта для хранения настроек.
   \arg Group Имя группы для хранения настроек.
 */

void TGeneralSettings::read(QSettings *pS, const QString &Group)
{
    Q_ASSERT(pS != NULL);

    if (!Group.isEmpty())
        pS->beginGroup(Group);

    setDefault();
    READ(SingleInstance,          bool);
    READ(ShowFileIcons,           bool);
    READ(ShowNetworkIcons,        bool);
    READ(ShowNameEditors,         bool);
    READ(CheckDestDirs,           bool);
    READ(CheckNetworkDestDirs,    bool);
    READ(FlatToolButtons,         bool);
    READ(ToolButtonStyle,         int);
    READ(AfterStartClearSrcList,  bool);
    READ(AfterStartClearDestList, bool);

    if (!Group.isEmpty())
        pS->endGroup();
}

//------------------------------------------------------------------------------
//! Запись настроек.
/*!
   \arg pS Указатель на экземпляр объекта для хранения настроек.
   \arg Group Имя группы для хранения настроек.
 */

void TGeneralSettings::write(QSettings *pS, const QString &Group)
{
    Q_ASSERT(pS != NULL);

    if (!Group.isEmpty())
        pS->beginGroup(Group);

    WRITE(SingleInstance);
    WRITE(ShowFileIcons);
    WRITE(ShowNetworkIcons);
    WRITE(ShowNameEditors);
    WRITE(CheckDestDirs);
    WRITE(CheckNetworkDestDirs);
    WRITE(FlatToolButtons);
    WRITE(ToolButtonStyle);
    WRITE(AfterStartClearSrcList);
    WRITE(AfterStartClearDestList);

    if (!Group.isEmpty())
        pS->endGroup();
}

//------------------------------------------------------------------------------
//! Конструктор.

TGeneralSettings::TGeneralSettings()
{
    setDefault();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#undef READ
#undef WRITE



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                        T S e t t i n g s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TSettings::TSettings()
    : m_pQSettings(NULL)
{
    QString PortableFlag = QApplication::applicationFilePath() + ".portable";
    if (QFile::exists(PortableFlag))
    {
        QString IniFile = QApplication::applicationDirPath() + QDir::separator() +
                          baseAppName() + ".settings";
        m_pQSettings = new QSettings(IniFile, QSettings::IniFormat);
    }
    else {
        m_pQSettings = new QSettings(QSettings::IniFormat,
                                     QSettings::UserScope,
                                     QApplication::organizationName(),
                                     baseAppName());
    }
    read();
}

//------------------------------------------------------------------------------
//! Деструктор.

TSettings::~TSettings()
{
    delete m_pQSettings;
}

//------------------------------------------------------------------------------
//! Возвращает идентификатор языка.

QString TSettings::langID()
{
    return m_pQSettings->value("Language").toString();
}

//------------------------------------------------------------------------------
//! Установка идентификатора языка.

void TSettings::setLangID(const QString& ID)
{
    m_pQSettings->setValue("Language", ID);
}

//------------------------------------------------------------------------------
//! Возвращает указатель на экземпляр класса хранения настроек.

TSettings* TSettings::instance()
{
    static TSettings Instance;
    return &Instance;
}

//------------------------------------------------------------------------------
//! Чтение всех настроек.

void TSettings::read()
{
    TaskSettings.read(m_pQSettings, "TaskSettings");
    GeneralSettings.read(m_pQSettings, "GeneralSettings");
}

//------------------------------------------------------------------------------
//! Запись всех настроек.

void TSettings::write()
{
    TaskSettings.write(m_pQSettings, "TaskSettings");
    GeneralSettings.write(m_pQSettings, "GeneralSettings");
    m_pQSettings->sync();
}

//------------------------------------------------------------------------------

void setToolButtonsSettings(QWidget* pForm)
{
    QList<QToolButton*> ToolButtons = pForm->findChildren<QToolButton*>();
    if (!ToolButtons.isEmpty())
    {
        register TGeneralSettings* pGS = &TSettings::instance()->GeneralSettings;
        bool Flat = pGS->FlatToolButtons;
        Qt::ToolButtonStyle Style = static_cast<Qt::ToolButtonStyle>(pGS->ToolButtonStyle);

        for (int i = ToolButtons.count() - 1; i >= 0; --i)
        {
            register QToolButton* pButton = ToolButtons[i];
            pButton->setAutoRaise(Flat);
            pButton->setToolButtonStyle(Style);
        }
    }
}

//------------------------------------------------------------------------------
