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

#ifndef MULTICOPYMAINWINDOW_HPP
#define MULTICOPYMAINWINDOW_HPP

#include <QMainWindow>
#include <QListWidget>

#include "Synchronizer.hpp"
#include "Settings.hpp"

//------------------------------------------------------------------------------

namespace Ui {
    class TMultiCopyForm;
}

//------------------------------------------------------------------------------

class TMultiCopy : public QMainWindow
{
    Q_OBJECT

    private:
        Ui::TMultiCopyForm *ui;
        TSettings Settings;

        void saveSession();
        void restoreSession();
        bool loadStringListFromFile(const QString& FileName, QStringList& List);
        bool saveStringListToFile(const QString& FileName, const QStringList& List);
        bool loadJobFromFile(const QString& FileName);
        bool saveJobToFile(const QString& FileName);
        void setShowNameEditors(bool Show);

    public:
        explicit TMultiCopy(QWidget *parent = 0);
        ~TMultiCopy();

        void retranslateAllUi(QString LangID);
        void loadListsFromSettings(QSettings* pS = NULL);
        //void saveListsToSettings(QSettings* pS = NULL);

    private slots:
        void srcChanged();
        void destChanged();
        void addCustomSrc();
        void addCustomDest();

        void on_SrcAddFile_clicked();
        void on_SrcAddFolder_clicked();
        void on_SrcRemove_clicked();
        void on_SrcClear_clicked();
        void on_SrcUp_clicked();
        void on_SrcDown_clicked();
        void on_SrcList_currentRowChanged(int currentRow);

        void on_DestAddFolder_clicked();
        void on_DestRemove_clicked();
        void on_DestClear_clicked();
        void on_DestList_currentRowChanged(int currentRow);

        void on_Start_clicked();
        void on_Settings_clicked();

        void on_actionAbout_triggered();
        void on_actionLoadSourcesList_triggered();
        void on_actionLoadDestinationsList_triggered();
        void on_actionSaveSourcesList_triggered();
        void on_actionSaveDestinationsList_triggered();
        void on_actionSaveJob_triggered();
        void on_actionLoadJob_triggered();
        void on_actionAddSubfolders_triggered();
};

//------------------------------------------------------------------------------

#endif // MULTICOPYMAINWINDOW_HPP
