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

#ifndef __TASKBARCONTROL_WIN__HPP__3EB45F74_35BC_4629_A9DD_F3B6AAA90405__
#define __TASKBARCONTROL_WIN__HPP__3EB45F74_35BC_4629_A9DD_F3B6AAA90405__

//------------------------------------------------------------------------------

#if defined(QT_VERSION) && !defined(Q_OS_WIN)
    #error "This file may be compiled in Windows only!"
#endif

//------------------------------------------------------------------------------

#include <windows.h>
#include <commctrl.h>

#include <QAtomicInt>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Сообщение о создании кнопки в панели задач.

extern DWORD WM_TASKBARBUTTONCREATED;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// {56FDF344-FD6D-11d0-958A-006097C9A090}
DEFINE_GUID(CLSID_TaskbarList, 0x56FDF344, 0xFD6D, 0x11d0, 0x95, 0x8A,
                               0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90);
// {EA1AFB91-9E28-4B86-90E9-9E9F8A5EEFAF}
DEFINE_GUID(IID_ITaskbarList3, 0xEA1AFB91, 0x9E28, 0x4B86, 0x90, 0xE9,
                               0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef __ITaskbarList_INTERFACE_DEFINED__
#define __ITaskbarList_INTERFACE_DEFINED__

MIDL_INTERFACE("56FDF342-FD6D-11D0-958A-006097C9A090")
ITaskbarList : public IUnknown
{
    STDMETHOD(HrInit(
        void)) PURE;
    STDMETHOD(AddTab(
        HWND hwnd)) PURE;
    STDMETHOD(DeleteTab(
        HWND hwnd)) PURE;
    STDMETHOD(ActivateTab(
        HWND hwnd)) PURE;
    STDMETHOD(SetActiveAlt(
        HWND hwnd)) PURE;
};

#endif // __ITaskbarList_INTERFACE_DEFINED__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef __ITaskbarList2_INTERFACE_DEFINED__
#define __ITaskbarList2_INTERFACE_DEFINED__

MIDL_INTERFACE("602D4995-B13A-429B-A66E-1935E44F4317")
ITaskbarList2 : public ITaskbarList
{
    STDMETHOD(MarkFullscreenWindow(HWND hwnd, BOOL fFullscreen)) PURE;
};

#endif // __ITaskbarList2_INTERFACE_DEFINED__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef __ITaskbarList3_INTERFACE_DEFINED__
#define __ITaskbarList3_INTERFACE_DEFINED__

enum TBPFLAG
{
    TBPF_NOPROGRESS    = 0x00,
    TBPF_INDETERMINATE = 0x01,
    TBPF_NORMAL	       = 0x02,
    TBPF_ERROR	       = 0x04,
    TBPF_PAUSED	       = 0x08
};

//------------------------------------------------------------------------------

enum THUMBBUTTONFLAGS
{
    THBF_ENABLED	    = 0x00,
    THBF_DISABLED	    = 0x01,
    THBF_DISMISSONCLICK	= 0x02,
    THBF_NOBACKGROUND	= 0x04,
    THBF_HIDDEN	        = 0x08,
    THBF_NONINTERACTIVE	= 0x10
};

//------------------------------------------------------------------------------

enum THUMBBUTTONMASK
{
    THB_BITMAP	= 0x1,
    THB_ICON	= 0x2,
    THB_TOOLTIP	= 0x4,
    THB_FLAGS	= 0x8
};

//------------------------------------------------------------------------------

#include <pshpack8.h>
struct THUMBBUTTON
{
    THUMBBUTTONMASK  dwMask;
    UINT             iId;
    UINT             iBitmap;
    HICON            hIcon;
    WCHAR            szTip[260];
    THUMBBUTTONFLAGS dwFlags;
};
#include <poppack.h>

typedef THUMBBUTTON* LPTHUMBBUTTON;

//------------------------------------------------------------------------------

// #define THBN_CLICKED 0x1800

//------------------------------------------------------------------------------

MIDL_INTERFACE("EA1AFB91-9E28-4B86-90E9-9E9F8A5EEFAF")
ITaskbarList3 : public ITaskbarList2
{
    STDMETHOD(SetProgressValue(
        HWND hwnd,
        ULONGLONG ullCompleted,
        ULONGLONG ullTotal)) PURE;

    STDMETHOD(SetProgressState(
        HWND hwnd,
        TBPFLAG tbpFlags)) PURE;

    STDMETHOD(RegisterTab(
        HWND hwndTab,
        HWND hwndMDI)) PURE;

    STDMETHOD(UnregisterTab(
        HWND hwndTab)) PURE;

    STDMETHOD(SetTabOrder(
        HWND hwndTab,
        HWND hwndInsertBefore)) PURE;

    STDMETHOD(SetTabActive(
        HWND hwndTab,
        HWND hwndMDI,
        DWORD dwReserved)) PURE;

    STDMETHOD(ThumbBarAddButtons(
        HWND hwnd,
        UINT cButtons,
        LPTHUMBBUTTON pButton)) PURE;

    STDMETHOD(ThumbBarUpdateButtons(
        HWND hwnd,
        UINT cButtons,
        LPTHUMBBUTTON pButton)) PURE;

    STDMETHOD(ThumbBarSetImageList(
        HWND hwnd,
        HIMAGELIST himl)) PURE;

    STDMETHOD(SetOverlayIcon(
        HWND hwnd,
        HICON hIcon,
        LPCWSTR pszDescription)) PURE;

    STDMETHOD(SetThumbnailTooltip(
        HWND hwnd,
        LPCWSTR pszTip)) PURE;

    STDMETHOD(SetThumbnailClip(
        HWND hwnd,
        RECT *prcClip)) PURE;

};

#endif // __ITaskbarList3_INTERFACE_DEFINED__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TITaskbarList3
{
    private :
        static ITaskbarList3* m_pITaskbarList3;
        static QAtomicInt     m_Counter;

    public :
        explicit TITaskbarList3();
        TITaskbarList3(const TITaskbarList3& other);
        virtual ~TITaskbarList3();

        inline ITaskbarList3* iface() { return m_pITaskbarList3; }
        inline ITaskbarList3* operator->() { return m_pITaskbarList3; }

        bool setProgressValue(HWND hWnd, ULONGLONG Completed, ULONGLONG Total);
        bool setProgressState(HWND hWnd, TBPFLAG State);
        bool setOverlayIcon(HWND hWnd, HICON hIcon);
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // __TASKBARCONTROL_WIN__HPP__3EB45F74_35BC_4629_A9DD_F3B6AAA90405__
