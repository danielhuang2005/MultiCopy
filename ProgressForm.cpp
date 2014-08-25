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

#include "ProgressForm.hpp"
#include "ui_ProgressForm.h"

#include <QMessageBox>
#include <QResizeEvent>
#include <QTime>
#include <QDebug>

#include "ErrorHandler.hpp"
#include "CircularBuffer.hpp"

//------------------------------------------------------------------------------

QString time(qint64 msec)
{
    QTime Time;
    Time = Time.addSecs(msec / 1000);
    return Time.toString();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//             T P r o g r e s s F o r m P r i v a t e
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TProgressFormPrivate* TProgressFormPrivate::m_pInstance = NULL;

//------------------------------------------------------------------------------

TProgressFormPrivate::TProgressFormPrivate(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::TProgressForm()),
      m_pControlThread(new TControlThread(this))
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    connect(m_pControlThread, SIGNAL(begin()),
            SLOT(begin()));
    connect(m_pControlThread, SIGNAL(beginJob(const TJob*)),
            SLOT(beginJob(const TJob*)));
    connect(m_pControlThread, SIGNAL(beginCalculate()),
            SLOT(beginCalculate()));
    connect(m_pControlThread, SIGNAL(endCalculate(TJobSize)),
            SLOT(endCalculate(TJobSize)));
    connect(m_pControlThread, SIGNAL(beginCopy()),
            SLOT(beginCopy()));
    connect(m_pControlThread, SIGNAL(beginCopyFile(QString,qint64)),
            SLOT(beginCopyFile(QString,qint64)));
    connect(m_pControlThread, SIGNAL(readProgress(const TRWCalculator*)),
            SLOT(readProgress(const TRWCalculator*)));
    connect(m_pControlThread, SIGNAL(writeProgress(const TRWCalculator*)),
            SLOT(writeProgress(const TRWCalculator*)));
    connect(m_pControlThread, SIGNAL(endCopyFile()),
            SLOT(endCopyFile()));
    connect(m_pControlThread, SIGNAL(endJob()),
            SLOT(endJob()));
    connect(m_pControlThread, SIGNAL(end()),
            SLOT(end()));
    connect(m_pControlThread, SIGNAL(outOfMemory()),
            SLOT(outOfMemory()));

    // Таймеры.
    m_TimeTimer.setInterval(1000);
    connect(&m_TimeTimer, SIGNAL(timeout()), SLOT(updateSpeedAndTime()));
    m_ProgressTimer.setInterval(250);
    connect(&m_ProgressTimer, SIGNAL(timeout()), SLOT(updateProgress()));

    restoreSession();
}

//------------------------------------------------------------------------------

TProgressFormPrivate::~TProgressFormPrivate()
{
    delete m_pControlThread;
    saveSession();
    delete ui;
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::setProgressText(const QString& Text)
{
    m_ProgressText = Text;
    elideProgressText();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::elideProgressText()
{
    QFontMetrics FM = ui->ProgressText->fontMetrics();
    QString Text = FM.elidedText(m_ProgressText,
                       Qt::ElideMiddle,
                       ui->ProgressText->width() - 10);
    ui->ProgressText->setText(Text);
}

//------------------------------------------------------------------------------

QString TProgressFormPrivate::speed(int BytesPerSec)
{
    if (BytesPerSec < 1024)
        return QString::number(BytesPerSec) + " " + tr("B/s");
    qreal f = BytesPerSec / 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', 4) + " " + tr("kB/s");
    f /= 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', 4) + " " + tr("MB/s");
    f /= 1024.0;
    return QString::number(f, 'g', 4) + " " + tr("GB/s");
}

//------------------------------------------------------------------------------
/*
QString TProgressFormPrivate::size(qint64 Size)
{
    if (Size < 1024)
        return QString::number(Size) + " " + tr("B");
    qreal f = Size / 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', 4) + " " + tr("kB");
    f /= 1024.0;
    if (f < 1024)
        return QString::number(f, 'g', 4) + " " + tr("MB");
    f /= 1024.0;
    return QString::number(f, 'g', 4) + " " + tr("GB");
}
*/
//------------------------------------------------------------------------------

void TProgressFormPrivate::updateSpeedAndTime()
{
    if (m_pControlThread->status() != TControlThread::Calculating)
    {
        const TRWCalculator* pCalc = m_pControlThread->rwCalc();
        QString S = speed(pCalc->speed());
        ui->Speed->setText(S);
        ui->Elapsed->setText(time(pCalc->time()));
        qint64 et = pCalc->remaining(m_JobSize.FilesSize -
                                     m_CopiedSize -
                                     pCalc->writedBytes());
        ui->Remaining->setText((et >= 0) ? time(et) : tr("Unknown"));
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::updateProgress()
{
    setProgressText(m_Progress.Text);
    ui->ReadProgress->setValue(m_Progress.Read);
    ui->WriteProgress->setValue(m_Progress.Write);
    if (m_Progress.Total >= 0)
        ui->TotalProgress->setValue(m_Progress.Total);
    if (m_Progress.Count >= 0)
        ui->CountProgress->setValue(m_Progress.Count);
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::saveSession()
{
    QSettings* pS = m_Settings.getQSettings();
    pS->beginGroup(objectName());
    pS->setValue("DoNotClose", ui->DoNotClose->isChecked());
    pS->endGroup();
    m_Settings.write();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::restoreSession()
{
    QSettings* pS = m_Settings.getQSettings();
    pS->beginGroup(objectName());
    ui->DoNotClose->setChecked(pS->value("DoNotClose").toBool());
    pS->endGroup();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::resizeEvent(QResizeEvent *Event)
{
    if (Event->size().width() != Event->oldSize().width())
        elideProgressText();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::closeEvent(QCloseEvent *Event)
{
    if (m_pControlThread->isRunning())
    {
        on_Cancel_clicked();
        Event->ignore();
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::outOfMemory()
{
    QMessageBox::critical(this, QApplication::applicationName(),
                          tr("Out of Memory!"));
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::beginCopy()
{
    m_Progress.clear();
    m_ProgressTimer.start();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::beginCopyFile(QString FileName, qint64 FileSize)
{
    m_CurrentFileSize = FileSize;
    /*setProgressText(FileName);
    ui->ReadProgress->setValue(0);
    ui->WriteProgress->setValue(0);*/
    m_Progress.Text = FileName;
    m_Progress.Read = 0;
    m_Progress.Write = 0;
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::endCopyFile()
{
    ++m_FilesCopied;
    m_CopiedSize += m_CurrentFileSize;
    if (m_JobSize.FilesCount > 0)
    {
        qreal f = ((qreal)m_CopiedSize / m_JobSize.FilesSize) * 100.0;
        /*ui->TotalProgress->setValue(qRound(f));
        ui->CountProgress->setValue(m_FilesCopied);*/
        m_Progress.Total = qRound(f);
        m_Progress.Count = m_FilesCopied;
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::endJob()
{
    m_ProgressTimer.stop();
    updateProgress();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::readProgress(const TRWCalculator* pCalc)
{
    qreal f = ((qreal)pCalc->readedBytes() / m_CurrentFileSize) * 100.0;
    //ui->ReadProgress->setValue(qRound(f));
    m_Progress.Read = qRound(f);
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::writeProgress(const TRWCalculator* pCalc)
{
    qreal f = ((qreal)pCalc->writedBytes() / m_CurrentFileSize) * 100;
    //ui->WriteProgress->setValue(qRound(f));
    m_Progress.Write = qRound(f);
    if (m_JobSize.FilesCount > 0)
    {
        f = ((qreal)(m_CopiedSize + pCalc->writedBytes()) / m_JobSize.FilesSize) * 100.0;
        //ui->TotalProgress->setValue(qRound(f));
        m_Progress.Total = qRound(f);
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::begin()
{
    ui->Cancel->setEnabled(true);
    ui->Pause->setEnabled(true);
    ui->Elapsed->clear();
    ui->Speed->clear();
    ui->Remaining->clear();
    m_TimeTimer.start();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::beginJob(const TJob* pJob)
{
    m_CopiedSize = 0;
    m_FilesCopied = 0;
    m_JobSize.clear();

    ui->CountProgress_Label->setEnabled(false);
    ui->CountProgress->setRange(0, 0);
    ui->CountProgress->setValue(0);
    ui->CountProgress->update();

    ui->TotalProgress_Label->setEnabled(false);
    ui->TotalProgress->setRange(0, 0);
    ui->TotalProgress->setValue(0);
    ui->TotalProgress->update();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::end()
{
    m_TimeTimer.stop();
    if (!ui->DoNotClose->isChecked())
        hide();
    else
        ui->ProgressText->setText(tr("All jobs are completed!"));
    ui->ReadProgress->setValue(100);
    ui->WriteProgress->setValue(100);
    ui->Cancel->setEnabled(false);
    ui->Pause->setEnabled(false);
    ui->Remaining->clear();
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::beginCalculate()
{
    ui->ReadProgress->setRange(0, 0);
    ui->ReadProgress->setValue(-1);
    ui->ReadProgress->update();

    ui->WriteProgress->setEnabled(false);
    ui->WriteProgress_Label->setEnabled(false);
    ui->WriteProgress->setRange(0, 0);
    ui->WriteProgress->setValue(0);
    ui->WriteProgress->update();

    setProgressText(tr("Calculate job size..."));
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::endCalculate(TJobSize JobSize)
{
    m_JobSize = JobSize;

    ui->ReadProgress->setRange(0, 100);
    ui->ReadProgress->setValue(0);
    ui->ReadProgress->update();

    ui->WriteProgress->setEnabled(true);
    ui->WriteProgress_Label->setEnabled(true);
    ui->WriteProgress->setRange(0, 100);
    ui->WriteProgress->setValue(0);
    ui->WriteProgress->update();

    ui->CountProgress_Label->setEnabled(true);
    ui->CountProgress->setRange(0, JobSize.FilesCount);
    ui->CountProgress->setValue(0);
    ui->CountProgress->update();

    ui->TotalProgress_Label->setEnabled(true);
    ui->TotalProgress->setRange(0, 100);
    ui->TotalProgress->setValue(0);
    ui->TotalProgress->update();

    setProgressText(tr("Preparing..."));
}

//------------------------------------------------------------------------------

TProgressFormPrivate* TProgressFormPrivate::create(QWidget* parent)
{
    if (m_pInstance == NULL)
        m_pInstance = new TProgressFormPrivate(parent);
    return m_pInstance;
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::addJob(const TJob& Job)
{
    m_pControlThread->addJob(Job);
    show();
    m_pControlThread->start();
}

//------------------------------------------------------------------------------

TProgressFormPrivate* TProgressFormPrivate::instance()
{
    return m_pInstance;
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::retranslateUi()
{
    ui->retranslateUi(this);
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::on_Cancel_clicked()
{
    bool Paused = m_pControlThread->isPaused();
    if (!Paused)
        m_pControlThread->pause();
    QMessageBox::StandardButton Btn = QMessageBox::question(this,
                                          QApplication::applicationName(),
                                          tr("You really want to cancel copy?"),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
    if (Btn == QMessageBox::Yes)
    {
        m_pControlThread->cancelAllJobs();
        // Если потоки были приостановлены, предыдущего оператора будет
        // недостаточно. Необходимо ещё продолжить выполнение потоков.
        m_pControlThread->resume();
    }
    else {
        if (!Paused)
            m_pControlThread->resume();
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::on_Pause_clicked()
{
    if (ui->Pause->isChecked())
    {
        m_pControlThread->pause();
        ui->Pause->setText(tr("&Resume"));
    }
    else {
        m_pControlThread->resume();
        ui->Pause->setText(tr("&Pause"));
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::showMessageBox(TErrorHandler* pErrorHandler)
{
    // Приостанавливаем счётчик времени на период отображения диалогового
    // окна с запросом пользователю.
    m_pControlThread->rwCalc()->pause();
    pErrorHandler->messageBox(this);
    m_pControlThread->rwCalc()->resume();
}




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                  T P r o g r e s s F o r m
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TProgressForm::TProgressForm(QWidget* parent)
    : p(TProgressFormPrivate::create(parent))
{

}

//------------------------------------------------------------------------------

TProgressForm::~TProgressForm()
{

}

//------------------------------------------------------------------------------

void TProgressForm::addJob(const TJob& Job)
{
    p->addJob(Job);
}

//------------------------------------------------------------------------------

void TProgressForm::retranslateUi()
{
    TProgressFormPrivate* p = TProgressFormPrivate::instance();
    if (p) p->retranslateUi();
}

//------------------------------------------------------------------------------
