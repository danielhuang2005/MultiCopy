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

#include "TaskbarControl.hpp"

#include <objbase.h>
#include <QWidget>

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Общие данные, описывающие состояние кнопки в панели задач.

struct TTaskbarControlData
{
    QAtomicInt Refs;         //!< Число ссылок.
    QWidget*   Parent;       //!< Окно-владелец кнопки.
    QPixmap    OverlayIcon;  //!< Оверлейная иконка.
    #ifdef Q_OS_WIN
        TITaskbarList3* pITaskbarList3;  //!< Интерфейс управления.
    #endif

    //! Прогресс-бар.
    struct TProgress {
        TTaskbarProgress::TState State;    //!< Состояние.
        int                      Minimum;  //!< Минимальное значение.
        int                      Maximum;  //!< Максимальное значение.
        int                      Value;    //!< Текущее значение.
        //! Конструктор.
        TProgress()
            : State(TTaskbarProgress::NoProgress),
              Minimum(0),
              Maximum(0),
              Value(0)
        { }
    } Progress;

    //! Конструктор.
    TTaskbarControlData() : Parent(NULL) { }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Статические переменные.

TTaskbarControl::TDataMap TTaskbarControl::m_DataMap;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Применение параметров прогресс-бара.

void TTaskbarProgress::applyProgress()
{
    Q_ASSERT(m_pData != NULL);

    if (m_pData->Progress.State <= Indeterminate)
        return;

    TTaskbarControlData::TProgress* pProgress = &m_pData->Progress;

    #ifdef Q_OS_WIN
        m_pData->pITaskbarList3->setProgressValue(
                    m_pData->Parent->winId(),
                    pProgress->Value - pProgress->Minimum,
                    pProgress->Maximum - pProgress->Minimum);
    #endif
}

//------------------------------------------------------------------------------
//! Применение параметров состояния прогресс-бара.

void TTaskbarProgress::applyState()
{
    Q_ASSERT(m_pData != NULL);

    #ifdef Q_OS_WIN
        m_pData->pITaskbarList3->setProgressState(
                    m_pData->Parent->winId(),
                    static_cast<TBPFLAG>(m_pData->Progress.State));
    #endif
    if (m_pData->Progress.State >= Normal)
        applyProgress();
}

//------------------------------------------------------------------------------
//! Конструктор.

TTaskbarProgress::TTaskbarProgress(TTaskbarControlData* pData)
    : m_pData(pData)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TTaskbarProgress::~TTaskbarProgress()
{
}

//------------------------------------------------------------------------------

TTaskbarProgress::TState TTaskbarProgress::state() const
{
    Q_ASSERT(m_pData != NULL);

    return m_pData->Progress.State;
}

//------------------------------------------------------------------------------

void TTaskbarProgress::setState(TTaskbarProgress::TState State)
{
    Q_ASSERT(m_pData != NULL);

    if (m_pData->Progress.State != State)
    {
        m_pData->Progress.State = State;
        applyState();
    }
}

//------------------------------------------------------------------------------

int TTaskbarProgress::value() const
{
    Q_ASSERT(m_pData != NULL);

    return m_pData->Progress.Value;
}

//------------------------------------------------------------------------------

void TTaskbarProgress::setValue(int Value)
{
    Q_ASSERT(m_pData != NULL);

    TTaskbarControlData::TProgress* pProgress = &m_pData->Progress;

    if (Value < pProgress->Minimum) Value = pProgress->Minimum;
    if (Value > pProgress->Maximum) Value = pProgress->Maximum;
    pProgress->Value = Value;
    applyProgress();
}


//------------------------------------------------------------------------------

int TTaskbarProgress::minimum() const
{
    Q_ASSERT(m_pData != NULL);

    return m_pData->Progress.Minimum;
}

//------------------------------------------------------------------------------

void TTaskbarProgress::setMinimum(int Minimum)
{
    Q_ASSERT(m_pData != NULL);

    if (m_pData->Progress.Minimum != Minimum)
    {
        if (m_pData->Progress.Maximum < Minimum)
            m_pData->Progress.Maximum = Minimum;
        applyProgress();
    }
}

//------------------------------------------------------------------------------

int TTaskbarProgress::maximum() const
{
    Q_ASSERT(m_pData != NULL);

    return m_pData->Progress.Maximum;
}

//------------------------------------------------------------------------------

void TTaskbarProgress::setMaximum(int Maximum)
{
    Q_ASSERT(m_pData != NULL);

    if (m_pData->Progress.Maximum != Maximum)
    {
        if (m_pData->Progress.Minimum > Maximum)
            m_pData->Progress.Minimum = Maximum;
        applyProgress();
    }
}

//------------------------------------------------------------------------------

void TTaskbarProgress::setProgress(int Minimum, int Maximum, int Value)
{
    Q_ASSERT(m_pData != NULL);

    if (Minimum > Maximum) Maximum = Minimum;
    if (Value < Minimum) Value = Minimum;
    if (Value > Maximum) Value = Maximum;

    m_pData->Progress.Minimum = Minimum;
    m_pData->Progress.Maximum = Maximum;
    m_pData->Progress.Value   = Value;

    if (Minimum != Maximum) {
        // TODO : А если сейчас Indeterminate?
        applyProgress();
    }
    else {
        if (m_pData->Progress.State == NoProgress ||
             m_pData->Progress.State == Normal)
        {
            setState(Indeterminate);
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void TTaskbarOverlayIcon::applyIcon()
{
    Q_ASSERT(m_pData != NULL);
    #ifdef Q_OS_WIN
        const HICON hIcon = m_pData->OverlayIcon.isNull() ?
                            NULL :
                            m_pData->OverlayIcon.toWinHICON();
        m_pData->pITaskbarList3->setOverlayIcon(
                    m_pData->Parent->winId(),
                    hIcon);
        DestroyIcon(hIcon);
    #endif
}

//------------------------------------------------------------------------------

TTaskbarOverlayIcon::TTaskbarOverlayIcon(TTaskbarControlData* pData)
    :m_pData(pData)
{
}

//------------------------------------------------------------------------------

TTaskbarOverlayIcon::~TTaskbarOverlayIcon()
{
}

//------------------------------------------------------------------------------

QPixmap TTaskbarOverlayIcon::pixmap() const
{
    return m_pData->OverlayIcon;
}

//------------------------------------------------------------------------------

void TTaskbarOverlayIcon::setPixmap(const QPixmap& Pixmap)
{
    m_pData->OverlayIcon = Pixmap;
    applyIcon();
}

//------------------------------------------------------------------------------

void TTaskbarOverlayIcon::remove()
{
    m_pData->OverlayIcon = QPixmap();
    applyIcon();
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TTaskbarControl::TTaskbarControl(QWidget* Parent)
    : m_pData(NULL),
      m_pProgressBar(NULL),
      m_pOverlayIcon(NULL)
{
    QWidget* p = Parent->parentWidget();
    while (p != NULL) {
        Parent = p;
        p = p->parentWidget();
    }

    QObject::setParent(Parent);

    for (TDataMap::iterator I = m_DataMap.begin(); I != m_DataMap.end(); ++I)
    {
        if ((*I)->Parent == Parent) {
            m_pData = *I;
            m_pData->Refs.ref();
        }
    }

    if (m_pData == NULL)
    {
        m_pData = new TTaskbarControlData();
        m_pData->Refs.ref();
        m_pData->Parent = Parent;
        m_DataMap.append(m_pData);
        m_pData->pITaskbarList3 = new TITaskbarList3();
    }
    m_pProgressBar = new TTaskbarProgress(m_pData);
    m_pOverlayIcon = new TTaskbarOverlayIcon(m_pData);
}

//------------------------------------------------------------------------------

TTaskbarControl::TTaskbarControl(const TTaskbarControl &other)
    : QObject(other.parent()),
      m_pData(other.m_pData),
      m_pProgressBar(new TTaskbarProgress(m_pData)),
      m_pOverlayIcon(new TTaskbarOverlayIcon(m_pData))
{
    m_pData->Refs.ref();
}

//------------------------------------------------------------------------------

TTaskbarControl::~TTaskbarControl()
{
    Q_ASSERT(m_DataMap.indexOf(m_pData) >= 0);

    if (!m_pData->Refs.deref()) {
        m_DataMap.remove(m_DataMap.indexOf(m_pData));
    }
}

//------------------------------------------------------------------------------

TTaskbarControl& TTaskbarControl::operator=(const TTaskbarControl& other)
{
    m_pData = other.m_pData;
    m_pData->Refs.ref();
    return *this;
}

//------------------------------------------------------------------------------

TTaskbarProgress* TTaskbarControl::progressBar()
{
    return m_pProgressBar;
}

//------------------------------------------------------------------------------

TTaskbarOverlayIcon *TTaskbarControl::overlayIcon()
{
    return m_pOverlayIcon;
}


//------------------------------------------------------------------------------

/*
//------------------------------------------------------------------------------
#include <QAbstractEventDispatcher>
#include "TaskbarControl_win.hpp"

QAbstractEventDispatcher::EventFilter Old_EventFilter = NULL;

bool TBC_EventFilter(void* msg)
{
    MSG* pMSG = static_cast<MSG*>(msg);
    if (pMSG != NULL) {
        if (pMSG->message == WM_TASKBARBUTTONCREATED) {
            qDebug("WM_TASKBARBUTTONCREATED for %p", pMSG->hwnd);
        }
    }
    if (Old_EventFilter != NULL)
        return Old_EventFilter(msg);
    return false;
}

void TTaskbarControl::initialize()
{
    QAbstractEventDispatcher* pDispatcher = QAbstractEventDispatcher::instance();
    Old_EventFilter = pDispatcher->setEventFilter(TBC_EventFilter);
}

//------------------------------------------------------------------------------
*/
