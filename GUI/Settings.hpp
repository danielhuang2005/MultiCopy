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

#ifndef __SETTINGS__HPP__
#define __SETTINGS__HPP__

#include "Core/Task/Task.hpp"

//------------------------------------------------------------------------------

class QSettings;

//------------------------------------------------------------------------------
//! Потомок класса TTaskSettings с возможностью сохранения настроек.

struct TTaskSettings2 : public TTaskSettings
{
    void read(QSettings* pS, const QString& Group = QString());
    void write(QSettings* pS, const QString& Group = QString());
};

//------------------------------------------------------------------------------
//! Общие настройки приложения.

struct TGeneralSettings
{
    bool SingleInstance;        //!< Только один экземпляр приложения.
    bool ShowFileIcons;         //!< Показывать иконки файлов и каталогов.
    bool ShowNetworkIcons;      //!< Показывать иконки для сетевых объектов.
    bool ShowNameEditors;       //!< Показывать поля ручного ввода имён.
    bool CheckDestDirs;         //!< Проверять объекты назначения на тип "каталог".
    bool CheckNetworkDestDirs;  //!< Проверять также сетевые объекты назначения.

    void setDefault();
    void read(QSettings* pS, const QString& Group = QString());
    void write(QSettings* pS, const QString& Group = QString());

    TGeneralSettings();
};

//------------------------------------------------------------------------------
//! Класс для хранения настроек.
/*!
   \remarks Класс является синглтоном.
 */

class TSettings
{
    private :
        QSettings* m_pQSettings;

        Q_DISABLE_COPY(TSettings)
        explicit TSettings();
        virtual ~TSettings();
    public:
        static TSettings* instance();

        TTaskSettings2   TaskSettings;     //!< Настройки задания.
        TGeneralSettings GeneralSettings;  //!< Общие настройки приложения.

        void read();
        void write();
        QString langID();
        void setLangID(const QString& ID);

        //! Возвращает указатель на внутренний объект типа QSettings.
        inline QSettings* getQSettings() { return m_pQSettings; }
};

//------------------------------------------------------------------------------

#endif // __SETTINGS__HPP__
