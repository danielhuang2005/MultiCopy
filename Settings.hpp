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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QSettings>
#include <QListWidget>

//------------------------------------------------------------------------------

struct TSettingsData
{
    int  RAMCellSize;
    int  RAMCellCount;
    bool CopyDateTime;
    bool TotalCalc;
    bool DirContentsOnly;
    int  SubDirsDepth;
};

//------------------------------------------------------------------------------

class TSettingsPrivate
{
    private :
        static QAtomicInt m_Refs;
        static TSettingsPrivate* m_pInstance;

        TSettingsPrivate();
        ~TSettingsPrivate();
    public :
        QSettings* m_pSettings;
        TSettingsData m_pData;

        static TSettingsPrivate* create();
        void release();
        void read();
        void write();
};

//------------------------------------------------------------------------------

class TSettings
{
    private :
        TSettingsPrivate* p;
    public:
        explicit TSettings();
        virtual ~TSettings();
        QString langID();
        void setLangID(const QString& ID);

        inline TSettingsData* data() { return &(p->m_pData); }
        inline QSettings* getQSettings() { return p->m_pSettings; }
        inline void write() { p->write(); }
        inline void sync() { p->m_pSettings->sync(); }
};

//------------------------------------------------------------------------------

#endif // SETTINGS_HPP
