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

#ifndef PROGRESSFORM_HPP
#define PROGRESSFORM_HPP

#include <QDialog>
#include <QTimer>

#include "ControlThread.hpp"

//------------------------------------------------------------------------------

class TErrorHandler;
class TRWCalculator;

//------------------------------------------------------------------------------

namespace Ui {
    class TProgressForm;
}

//------------------------------------------------------------------------------

class TProgressFormPrivate : public QDialog
{
    Q_OBJECT
    private:
        struct TProgress {
            QString Text;
            int Read;
            int Write;
            int Count;
            int Total;

            void clear() {
                Text.clear(); Read = Write = 0;  Count = Total = -1;
            }
        };
        TProgress m_Progress;
        QTimer m_ProgressTimer;

        static TProgressFormPrivate* m_pInstance;
        Ui::TProgressForm* ui;
        TControlThread* m_pControlThread;
        TJobSize m_JobSize;
        qint64 m_CopiedSize;
        qint64 m_CurrentFileSize;
        int m_FilesCopied;
        QString m_ProgressText;
        QTimer m_TimeTimer;
        TSettings m_Settings;

        explicit TProgressFormPrivate(QWidget* parent);
        virtual ~TProgressFormPrivate();

        void setProgressText(const QString& Text);
        void elideProgressText();

        static QString speed(int BytesPerSecond);
        //static QString size(qint64 Size);

        void saveSession();
        void restoreSession();
    protected :
        virtual void resizeEvent(QResizeEvent *Event);
        virtual void closeEvent(QCloseEvent *Event);
    private slots :
        void updateSpeedAndTime();
        void updateProgress();

        void begin();
        void beginJob(const TJob* pJob);
        void outOfMemory();
        void beginCalculate();
        void endCalculate(TJobSize JobSize);
        void beginCopy();
        void beginCopyFile(QString FileName, qint64 FileSize);
        void readProgress(const TRWCalculator* pCalc);
        void writeProgress(const TRWCalculator* pCalc);
        void endCopyFile();
        void endJob();
        void end();

        void on_Cancel_clicked();
        void on_Pause_clicked();
    public:
        static TProgressFormPrivate* create(QWidget* parent);
        void addJob(const TJob& Job);

        static TProgressFormPrivate* instance();
        void retranslateUi();
    public slots :
        void showMessageBox(TErrorHandler* pErrorHandler);
};

//------------------------------------------------------------------------------

class TProgressForm
{
    private:
        TProgressFormPrivate* p;
    public:
        explicit TProgressForm(QWidget* parent);
        ~TProgressForm();

        void addJob(const TJob& Job);

        static void retranslateUi();
};

//------------------------------------------------------------------------------

#endif // PROGRESSFORM_HPP
