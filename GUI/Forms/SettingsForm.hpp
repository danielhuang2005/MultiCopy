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

#ifndef __SETTINGSFORM__HPP__
#define __SETTINGSFORM__HPP__

#include <QDialog>

//------------------------------------------------------------------------------

namespace Ui {
    class TSettingsForm;
}
class TSettings;
struct TGeneralSettings;

//------------------------------------------------------------------------------

class TSettingsForm : public QDialog
{
    Q_OBJECT
    private:
        Ui::TSettingsForm *ui;
        TSettings* m_pSettings;
        struct TLangDef {
            QString LangID;
            QString LangName;
            TLangDef() {}
            TLangDef(const QString& ID, const QString& Name)
                : LangID(ID), LangName(Name) {}
        };
        typedef QList<TLangDef> TLangDefList;
        TLangDefList m_LangDefList;

        void languagesList();
        void saveSession();
        void restoreSession();
        void readData_ViewParams(const TGeneralSettings* pGS);
        void readData_SystemParams(const TGeneralSettings* pGS);
        void readData(const TGeneralSettings* pGS);
        void writeData_ViewParams(TGeneralSettings* pGS);
        void writeData_SystemParams(TGeneralSettings* pGS);
        void writeData(TGeneralSettings* pGS);

    protected :
        virtual void closeEvent(QCloseEvent *Event);
        virtual void changeEvent(QEvent *e);

    public:
        explicit TSettingsForm(QWidget* Parent = NULL);
        ~TSettingsForm();

    public slots :
        virtual void accept();
        virtual void reject();

    private slots :
        void on_Default_clicked();
        void on_Languages_currentIndexChanged(int index);
};

//------------------------------------------------------------------------------

#endif // __SETTINGSFORM__HPP__
