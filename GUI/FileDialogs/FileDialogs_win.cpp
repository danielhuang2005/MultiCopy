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

#include "FileDialogs_win.hpp"

#define INITGUID
#include "WinObjects.hpp"

#include <QLibrary>
#include <QDir>
#include <QApplication>

#include "../../Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// "Обёртка" для IFileOpenDialog.

class IFileOpenDialog_Wrapper
{
    private :
        IFileOpenDialog* m_pInterface;  //!< Интерфейс.

    public :
        //----------------------------------------------------------------------
        //! Конструктор.

        IFileOpenDialog_Wrapper()
        {
            HRESULT HResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            if (SUCCEEDED(HResult))
            {
                HResult = CoCreateInstance(CLSID_FileOpenDialog,
                                           NULL,
                                           CLSCTX_INPROC_SERVER,
                                           IID_IFileOpenDialog,
                                           reinterpret_cast<void**>(&m_pInterface));
                if (FAILED(HResult))
                {
                    qWarning("CoCreateInstance error: %s",
                             qPrintable(GetSystemErrorString(HResult)));
                    CoUninitialize();
                }
            }
            else {
                qWarning("CoInitializeEx error: %s",
                         qPrintable(GetSystemErrorString(HResult)));
            }
        }

        //----------------------------------------------------------------------
        //! Деструктор.

        ~IFileOpenDialog_Wrapper()
        {
            if (m_pInterface) {
                m_pInterface->Release();
                CoUninitialize();
            }
        }

        //----------------------------------------------------------------------
        //! Возвращает true, если интерфейс успешно получен.

        bool isInitialized() const
        {
            return m_pInterface != NULL;
        }

        //----------------------------------------------------------------------
        //! Указатель на интерфейс.

        IFileOpenDialog* iface()
        {
            return m_pInterface;
        }

        //----------------------------------------------------------------------
        //! Оператор "стрелка".

        IFileOpenDialog* operator->()
        {
            return m_pInterface;
        }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool SHCreateItemFromParsingName_Wrapper(const QString& Path, IShellItem** ppShellItem)
{
    typedef HRESULT (WINAPI *TpSHCreateItemFromParsingName)(PCWSTR pszPath,
                                                            IBindCtx *pbc,
                                                            REFIID riid,
                                                            void **ppv);

    static TpSHCreateItemFromParsingName pSHCreateItemFromParsingName =
               reinterpret_cast<TpSHCreateItemFromParsingName>(
                   QLibrary::resolve("Shell32", "SHCreateItemFromParsingName")
            );

    if (pSHCreateItemFromParsingName != NULL)
    {
        HRESULT HResult = pSHCreateItemFromParsingName(
                              (PCWSTR)QDir::toNativeSeparators(Path).utf16(),
                              NULL,
                              IID_IShellItem,
                              reinterpret_cast<void**>(ppShellItem));
        if (FAILED(HResult))
        {
            qWarning("SHCreateItemFromParsingName error on \"%s\": %s.",
                     qPrintable(Path),
                     qPrintable(GetSystemErrorString(HResult)));

        }
        return SUCCEEDED(HResult);
    }
    else {
        qWarning("Poiner to Shell32.SHCreateItemFromParsingName is NULL.");
    }

    return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Установка параметров диалога.
/*!
   \param pDialog          Указатель на интерфейс.
   \param caption          Заголовок окна диалога.
   \param dir              Начальный каталог.
   \param AllowMultiselect Разрешение множественного выбора.
 */

void setFileDialogParams(IFileDialog* pDialog,
                         const QString& caption,
                         const QString& dir,
                         bool AllowMultiselect)
{
    Q_ASSERT(pDialog != NULL);

    HRESULT HResult = S_OK;

    // Заголовок окна.
    if (!caption.isEmpty())
    {
        HResult = pDialog->SetTitle((LPCWSTR)caption.utf16());
        if (FAILED(HResult))
        {
            qWarning("IFileDialog::SetTitle error: %s.",
                     qPrintable(GetSystemErrorString(HResult)));
        }
    }

    // Начальный каталог.
    if (!dir.isEmpty())
    {
        IShellItem* pItem;
        if (SHCreateItemFromParsingName_Wrapper(dir, &pItem))
        {
            pDialog->SetFolder(pItem);
            pItem->Release();
        }
    }

    // Опции.
    FILEOPENDIALOGOPTIONS Options;
    Options = FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM |
              FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST;
    if (AllowMultiselect)
        Options |= FOS_ALLOWMULTISELECT;
    HResult = pDialog->SetOptions(Options);
    if (FAILED(HResult))
    {
        qWarning("IFileDialog::SetOptions error: %lu.", HResult);
    }
}

//------------------------------------------------------------------------------

QStringList showDirDialog(IFileOpenDialog* pDialog, QWidget* parent)
{
    Q_ASSERT(pDialog != NULL);

    QStringList Result;

    if (parent == NULL)
        parent = QApplication::activeWindow();
    HWND hwndParent = parent ? parent->window()->winId() : NULL;

    HRESULT HResult = pDialog->Show(hwndParent);
    if (SUCCEEDED(HResult))
    {
        // Что-то было выбрано.
        IShellItemArray* pShellItemArray;
        HResult = pDialog->GetResults(&pShellItemArray);
        if (SUCCEEDED(HResult))
        {
            DWORD ItemsCount;
            HResult = pShellItemArray->GetCount(&ItemsCount);
            if (SUCCEEDED(HResult))
            {
                IShellItem* pItem;
                for (DWORD i = 0; i < ItemsCount; ++i)
                {
                    HResult = pShellItemArray->GetItemAt(i, &pItem);
                    if (SUCCEEDED(HResult))
                    {
                        wchar_t* lpwName;
                        HResult = pItem->GetDisplayName(SIGDN_FILESYSPATH, &lpwName);
                        if (SUCCEEDED(HResult))
                        {
                            Result.append(QString::fromWCharArray(lpwName));
                            CoTaskMemFree(lpwName);
                        }
                        else {
                            qWarning("IShellItem::GetDisplayName error: %s.",
                                     qPrintable(GetSystemErrorString(HResult)));
                        }
                        pItem->Release();
                    }
                    else {
                        qWarning("IShellItemArray::GetItemAt(%lu, ...) error: %s.",
                                 i, qPrintable(GetSystemErrorString(HResult)));
                    }
                }
                pShellItemArray->Release();
            }
            else {
                qWarning("IShellItemArray::GetCount error: %s",
                         qPrintable(GetSystemErrorString(HResult)));
            }
        }
        else {
            qWarning("IFileOpenDialog::GetResults error: %s",
                     qPrintable(GetSystemErrorString(HResult)));
        }
    }
    else {
        if (HResult == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            // Пользователь ничего не выбрал.
        }
        else {
            qWarning("IFileOpenDialog::Show error: %s",
                     qPrintable(GetSystemErrorString(HResult)));
        }
    }

    return Result;
}

//------------------------------------------------------------------------------

bool getExistingDirectories_win(QStringList* pResult,
                                QWidget* parent,
                                const QString& caption,
                                const QString& dir)
{
    Q_ASSERT(pResult != NULL);

    IFileOpenDialog_Wrapper Dialog;
    if (Dialog.isInitialized())
    {
        setFileDialogParams(Dialog.iface(), caption, dir, true);
        *pResult = showDirDialog(Dialog.iface(), parent);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool getExistingDirectory_win(QString* pResult,
                              QWidget* parent,
                              const QString& caption,
                              const QString& dir)
{
    Q_ASSERT(pResult != NULL);

    IFileOpenDialog_Wrapper Dialog;
    if (Dialog.isInitialized())
    {
        setFileDialogParams(Dialog.iface(), caption, dir, false);
        QStringList List = showDirDialog(Dialog.iface(), parent);
        if (!List.isEmpty())
            *pResult = List.first();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
