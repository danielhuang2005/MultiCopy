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

#define INITGUID

#include "TaskbarControl_win.hpp"

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------

DWORD WM_TASKBARBUTTONCREATED = RegisterWindowMessageW(L"TaskbarButtonCreated");

ITaskbarList3* TITaskbarList3::m_pITaskbarList3 = NULL;
QAtomicInt     TITaskbarList3::m_Counter;

//------------------------------------------------------------------------------

TITaskbarList3::TITaskbarList3()
{
    m_Counter.ref();
    if (m_pITaskbarList3 == NULL)
    {
        HRESULT HResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(HResult))
        {
            HResult = CoCreateInstance(CLSID_TaskbarList,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_ITaskbarList3,
                                       (void**)&m_pITaskbarList3);
            if (HResult == S_OK && m_pITaskbarList3 != NULL)
            {
                HResult = m_pITaskbarList3->HrInit();
                if (SUCCEEDED(HResult)) {
                    return;
                }
                else {
                    qWarning("Error initializing ITaskbarList3: \"%s\".",
                             qPrintable(GetSystemErrorString(HResult)));
                }
            }
            else {
                qWarning("Error get ITaskbarList3: \"%s\".",
                         qPrintable(GetSystemErrorString(HResult)));
            }
            CoUninitialize();
        }
    }
}

//------------------------------------------------------------------------------

TITaskbarList3::TITaskbarList3(const TITaskbarList3& other)
{
    Q_UNUSED(other);
    m_Counter.ref();
}

//------------------------------------------------------------------------------

TITaskbarList3::~TITaskbarList3()
{
    if (!m_Counter.deref()) {
        if (m_pITaskbarList3 != NULL) {
            m_pITaskbarList3->Release();
            m_pITaskbarList3 = NULL;
            CoUninitialize();
        }
    }
}

//------------------------------------------------------------------------------

bool TITaskbarList3::setProgressValue(HWND hWnd, ULONGLONG Completed, ULONGLONG Total)
{
    if (m_pITaskbarList3 != NULL)
    {
        HRESULT HResult = m_pITaskbarList3->SetProgressValue(hWnd, Completed, Total);
        if (SUCCEEDED(HResult)) {
            return true;
        }
        else {
            qWarning("ITaskbarList3::SetProgressValue error: \"%s\"",
                     qPrintable(GetSystemErrorString(HResult)));
        }
    }
    else {
        qWarning("TITaskbarList3::setProgressValue. "
                 "Pointer to ITaskbarList3 is NULL.");
    }
    return false;
}

//------------------------------------------------------------------------------

bool TITaskbarList3::setProgressState(HWND hWnd, TBPFLAG State)
{
    if (m_pITaskbarList3 != NULL)
    {
        HRESULT HResult = m_pITaskbarList3->SetProgressState(hWnd, State);
        if (SUCCEEDED(HResult)) {
            return true;
        }
        else {
            qWarning("ITaskbarList3::SetProgressState error: \"%s\"",
                     qPrintable(GetSystemErrorString(HResult)));
        }
    }
    else {
        qWarning("TITaskbarList3::setProgressState. "
                 "Pointer to ITaskbarList3 is NULL.");
    }
    return false;
}

//------------------------------------------------------------------------------

bool TITaskbarList3::setOverlayIcon(HWND hWnd, HICON hIcon)
{
    if (m_pITaskbarList3 != NULL)
    {
        HRESULT HResult = m_pITaskbarList3->SetOverlayIcon(hWnd, hIcon, NULL);
        if (SUCCEEDED(HResult)) {
            return true;
        }
        else {
            qWarning("ITaskbarList3::SetOverlayIcon error: \"%s\"",
                     qPrintable(GetSystemErrorString(HResult)));
        }
    }
    else {
        qWarning("TITaskbarList3::setOverlayIcon. "
                 "Pointer to ITaskbarList3 is NULL.");
    }
    return false;
}

//------------------------------------------------------------------------------
