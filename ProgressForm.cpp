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

    connect(m_pControlThread, SIGNAL(begin()), SLOT(begin()));
    connect(m_pControlThread, SIGNAL(beginJob(const TJob*)),
            SLOT(beginJob(const TJob*)));
    connect(m_pControlThread, SIGNAL(beginCalculate()),
            SLOT(beginCalculate()));
    connect(m_pControlThread, SIGNAL(endCalculate(TJobSize)),
            SLOT(endCalculate(TJobSize)));
    connect(m_pControlThread, SIGNAL(beginCopyFile(QString,qint64)),
            SLOT(beginCopyFile(QString,qint64)));
    connect(m_pControlThread, SIGNAL(readProgress(const TRWCalculator*)),
            SLOT(readProgress(const TRWCalculator*)));
    connect(m_pControlThread, SIGNAL(writeProgress(const TRWCalculator*)),
            SLOT(writeProgress(const TRWCalculator*)));
    connect(m_pControlThread, SIGNAL(endCopyFile()),
            SLOT(endCopyFile()));
    connect(m_pControlThread, SIGNAL(end()), SLOT(end()));
    connect(m_pControlThread, SIGNAL(outOfMemory()),
            SLOT(outOfMemory()));
    m_Timer.setInterval(1000);
    connect(&m_Timer, SIGNAL(timeout()), SLOT(updateSpeedAndTime()));
}

//------------------------------------------------------------------------------

TProgressFormPrivate::~TProgressFormPrivate()
{
    delete m_pControlThread;
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

QString TProgressFormPrivate::speed(int BytesPerSec) const
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

void TProgressFormPrivate::updateSpeedAndTime()
{
    const TRWCalculator* pCalc = m_pControlThread->rwCalc();
    QString S = speed(pCalc->speed());
    ui->Speed->setText(S);
    ui->Elapsed->setText(time(pCalc->time()));
    qint64 et = pCalc->remaining(m_JobSize.FilesSize - m_CopiedSize - pCalc->writedBytes());
    ui->Remaining->setText((et >= 0) ? time(et) : tr("Unknown"));
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

void TProgressFormPrivate::beginCopyFile(QString FileName, qint64 FileSize)
{
    m_CurrentFileSize = FileSize;
    setProgressText(FileName);
    ui->ReadProgress->setValue(0);
    ui->WriteProgress->setValue(0);
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::endCopyFile()
{
    ++m_FilesCopied;
    m_CopiedSize += m_CurrentFileSize;
    if (m_JobSize.FilesCount > 0)
    {
        qreal f = ((qreal)m_CopiedSize / m_JobSize.FilesSize) * 100.0;
        ui->TotalProgress->setValue(qRound(f));
        ui->CountProgress->setValue(m_FilesCopied);
    }
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::readProgress(const TRWCalculator* pCalc)
{
    qreal f = ((qreal)pCalc->readedBytes() / m_CurrentFileSize) * 100.0;
    ui->ReadProgress->setValue(qRound(f));
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::writeProgress(const TRWCalculator* pCalc)
{
    qreal f = ((qreal)pCalc->writedBytes() / m_CurrentFileSize) * 100;
    ui->WriteProgress->setValue(qRound(f));
    if (m_JobSize.FilesCount > 0)
    {
        f = ((qreal)(m_CopiedSize + pCalc->writedBytes()) / m_JobSize.FilesSize) * 100.0;
        ui->TotalProgress->setValue(qRound(f));
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
    m_Timer.start();
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
    ui->TotalProgress_Label->setEnabled(false);
    ui->TotalProgress->setRange(0, 0);
    ui->TotalProgress->setValue(0);
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::end()
{
    m_Timer.stop();
    if (!ui->DoNotClose->isChecked())
        hide();
    else
        ui->ProgressText->setText(tr("All jobs are completed!"));
    ui->Cancel->setEnabled(false);
    ui->Pause->setEnabled(false);
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::beginCalculate()
{
    ui->ReadProgress->setRange(0, 0);
    ui->ReadProgress->setValue(-1);
    ui->WriteProgress->setRange(0, 0);
    ui->WriteProgress->setValue(0);
    setProgressText(tr("Calculate job size..."));
}

//------------------------------------------------------------------------------

void TProgressFormPrivate::endCalculate(TJobSize JobSize)
{
    m_JobSize = JobSize;
    ui->ReadProgress->setRange(0, 100);
    ui->ReadProgress->setValue(0);
    ui->WriteProgress->setRange(0, 100);
    ui->WriteProgress->setValue(0);
    ui->CountProgress_Label->setEnabled(true);
    ui->CountProgress->setRange(0, JobSize.FilesCount);
    ui->CountProgress->setValue(0);
    ui->TotalProgress_Label->setEnabled(true);
    ui->TotalProgress->setRange(0, 100);
    ui->TotalProgress->setValue(0);
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
    pErrorHandler->messageBox(this, m_pControlThread->buffer()->consumersCount());
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
