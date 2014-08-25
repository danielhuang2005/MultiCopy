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

#ifndef __WINOBJECTS__HPP__
#define __WINOBJECTS__HPP__

//------------------------------------------------------------------------------

#include <windows.h>
#include <objbase.h>

//------------------------------------------------------------------------------
// Неиспользуемые типы.

#if defined(_MSC_VER)
    #include <Shobjidl.h>
#else

DECLARE_INTERFACE(IFileDialogEvents);
DECLARE_INTERFACE(IShellItem);
DECLARE_INTERFACE(IShellItemFilter);
DECLARE_INTERFACE(IEnumShellItems);

#ifndef PROPERTYKEY_DEFINED
    #define PROPERTYKEY_DEFINED
    struct PROPERTYKEY;
#endif

//------------------------------------------------------------------------------

struct COMDLG_FILTERSPEC {
    LPCWSTR pszName;
    LPCWSTR pszSpec;
};

//------------------------------------------------------------------------------

enum FDAP {
  FDAP_BOTTOM = 0,
  FDAP_TOP    = 1
};

//------------------------------------------------------------------------------

typedef int GETPROPERTYSTOREFLAGS;

//------------------------------------------------------------------------------

typedef DWORD FILEOPENDIALOGOPTIONS;
#define FOS_OVERWRITEPROMPT	   0x00000002
#define FOS_STRICTFILETYPES	   0x00000004
#define FOS_NOCHANGEDIR	       0x00000008
#define FOS_PICKFOLDERS	       0x00000020
#define FOS_FORCEFILESYSTEM	   0x00000040
#define FOS_ALLNONSTORAGEITEMS 0x00000080
#define FOS_NOVALIDATE         0x00000100
#define FOS_ALLOWMULTISELECT   0x00000200
#define FOS_PATHMUSTEXIST      0x00000800
#define FOS_FILEMUSTEXIST      0x00001000
#define FOS_CREATEPROMPT       0x00002000
#define FOS_SHAREAWARE         0x00004000
#define FOS_NOREADONLYRETURN   0x00008000
#define FOS_NOTESTFILECREATE   0x00010000
#define FOS_HIDEMRUPLACES      0x00020000
#define FOS_HIDEPINNEDPLACES   0x00040000
#define FOS_NODEREFERENCELINKS 0x00100000
#define FOS_DONTADDTORECENT    0x02000000
#define FOS_FORCESHOWHIDDEN    0x10000000
#define FOS_DEFAULTNOMINIMODE  0x20000000
#define FOS_FORCEPREVIEWPANEON 0x40000000

//------------------------------------------------------------------------------

enum SIGDN {
    SIGDN_NORMALDISPLAY               = 0x00000000,
    SIGDN_PARENTRELATIVEPARSING       = 0x80018001,
    SIGDN_PARENTRELATIVEFORADDRESSBAR = 0x8001c001,
    SIGDN_DESKTOPABSOLUTEPARSING      = 0x80028000,
    SIGDN_PARENTRELATIVEEDITING       = 0x80031001,
    SIGDN_DESKTOPABSOLUTEEDITING      = 0x8004c000,
    SIGDN_FILESYSPATH                 = 0x80058000,
    SIGDN_URL                         = 0x80068000
};

//------------------------------------------------------------------------------

enum SIATTRIBFLAGS {
    SIATTRIBFLAGS_AND	    = 0x1,
    SIATTRIBFLAGS_OR	    = 0x2,
    SIATTRIBFLAGS_APPCOMPAT	= 0x3,
    SIATTRIBFLAGS_MASK	    = 0x3
};

//------------------------------------------------------------------------------

DECLARE_INTERFACE_(IShellItem, IUnknown)
{
    STDMETHOD(BindToHandler)(THIS_ IBindCtx *pbc, REFGUID bhid, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetParent)(THIS_ IShellItem **ppsi) PURE;
    STDMETHOD(GetDisplayName)(THIS_ SIGDN sigdnName, LPWSTR *ppszName) PURE;
    STDMETHOD(GetAttributes)(THIS_ ULONG sfgaoMask, ULONG *psfgaoAttribs) PURE;
    STDMETHOD(Compare)(THIS_ IShellItem *psi, DWORD hint, int *piOrder) PURE;
};

//------------------------------------------------------------------------------

DECLARE_INTERFACE_(IShellItemArray, IUnknown)
{
    STDMETHOD(BindToHandler)(THIS_ IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppvOut) PURE;
    STDMETHOD(GetPropertyStore)(THIS_ GETPROPERTYSTOREFLAGS flags, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetPropertyDescriptionList)(THIS_ const PROPERTYKEY *keyType, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributes)(THIS_ SIATTRIBFLAGS dwAttribFlags, ULONG sfgaoMask, ULONG *psfgaoAttribs) PURE;
    STDMETHOD(GetCount)(THIS_ DWORD *pdwNumItems) PURE;
    STDMETHOD(GetItemAt)(THIS_ DWORD dwIndex, IShellItem **ppsi) PURE;
    STDMETHOD(EnumItems)(THIS_ IEnumShellItems **ppenumShellItems) PURE;
};

//------------------------------------------------------------------------------

DECLARE_INTERFACE_(IModalWindow, IUnknown)
{
    STDMETHOD(Show)(THIS_ HWND hwndParent) PURE;
};

//------------------------------------------------------------------------------

DECLARE_INTERFACE_(IFileDialog, IModalWindow)
{
    STDMETHOD(SetFileTypes)(THIS_ UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec) PURE;
    STDMETHOD(SetFileTypeIndex)(THIS_ UINT iFileType) PURE;
    STDMETHOD(GetFileTypeIndex)(THIS_ UINT *piFileType) PURE;
    STDMETHOD(Advise)(THIS_ IFileDialogEvents *pfde, DWORD *pdwCookie) PURE;
    STDMETHOD(Unadvise)(THIS_ DWORD dwCookie) PURE;
    STDMETHOD(SetOptions)(THIS_ FILEOPENDIALOGOPTIONS  fos) PURE;
    STDMETHOD(GetOptions)(THIS_ FILEOPENDIALOGOPTIONS  *pfos) PURE;
    STDMETHOD(SetDefaultFolder)(THIS_ IShellItem *psi) PURE;
    STDMETHOD(SetFolder)(THIS_ IShellItem *psi) PURE;
    STDMETHOD(GetFolder)(THIS_ IShellItem **ppsi) PURE;
    STDMETHOD(GetCurrentSelection)(THIS_ IShellItem **ppsi) PURE;
    STDMETHOD(SetFileName)(THIS_ LPCWSTR pszName) PURE;
    STDMETHOD(GetFileName)(THIS_ LPWSTR *pszName) PURE;
    STDMETHOD(SetTitle)(THIS_ LPCWSTR pszTitle) PURE;
    STDMETHOD(SetOkButtonLabel)(THIS_ LPCWSTR pszText) PURE;
    STDMETHOD(SetFileNameLabel)(THIS_ LPCWSTR pszLabel) PURE;
    STDMETHOD(GetResult)(THIS_ IShellItem **ppsi) PURE;
    STDMETHOD(AddPlace)(THIS_ IShellItem *psi, FDAP fdap) PURE;
    STDMETHOD(SetDefaultExtension)(THIS_ LPCWSTR pszDefaultExtension) PURE;
    STDMETHOD(Close)(THIS_ HRESULT hr) PURE;
    STDMETHOD(SetClientGuid)(THIS_ REFGUID guid) PURE;
    STDMETHOD(ClearClientData)(THIS_) PURE;
    STDMETHOD(SetFilter)(THIS_ IShellItemFilter *pFilter) PURE;
};

//------------------------------------------------------------------------------

DECLARE_INTERFACE_(IFileOpenDialog, IFileDialog)
{
    STDMETHOD(GetResults)(THIS_ IShellItemArray **ppenum) PURE;
    STDMETHOD(GetSelectedItems)(THIS_ IShellItemArray **ppsai) PURE;
};

//------------------------------------------------------------------------------

DEFINE_GUID(IID_IFileOpenDialog,
            0xd57c7288, 0xd4ad, 0x4768, 0xbe, 0x02, 0x9d, 0x96, 0x95, 0x32, 0xd9, 0x60);
DEFINE_GUID(IID_IShellItem,
            0x43826d1e, 0xe718, 0x42ee, 0xbc, 0x55, 0xa1, 0xe2, 0x61, 0xc3, 0x7b, 0xfe);
DEFINE_GUID(CLSID_FileOpenDialog,
            0xdc1c5a9c, 0xe88a, 0x4dde, 0xa5, 0xa1, 0x60, 0xf8, 0x2a, 0x20, 0xae, 0xf7);

//------------------------------------------------------------------------------

#endif // defined(_MSC_VER)

#endif // __WINOBJECTS__HPP__
