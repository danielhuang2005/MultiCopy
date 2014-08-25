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

#include "DirIterator.hpp"

#ifndef Q_OS_WIN
    #include <errno.h>
#endif

#include <QDir>

#include "FileInfoEx.hpp"
#include "Core/Common/CommonFn.hpp"

#include "Core/FastIO/FileInfoEx_p.hpp"

//------------------------------------------------------------------------------

void TDirIterator::start()
{
    Q_ASSERT(m_Status != stStarted);

    #ifdef Q_OS_WIN
        QString Path = PathToLongWinPath(m_StartPath);
        if (!Path.endsWith(QDir::separator()))
            Path += QDir::separator();
        Path += '*';

        m_hFind = FindFirstFileExW((LPCWSTR)Path.utf16(),
                                   FindExInfoStandard,
                                   &m_FindData,
                                   FindExSearchNameMatch,
                                   NULL,
                                   0);
        if (m_hFind != INVALID_HANDLE_VALUE) {
            m_FileInfoEx.m_Data->init(m_StartPath, &m_FindData);
            m_Status = stStarted;
        }
        else {
            DWORD ErrCode = GetLastError();
            if (ErrCode != ERROR_FILE_NOT_FOUND) {
                qWarning("TDirIterator::start. FindFirstFileEx error on \"%s\": %s",
                         qPrintable(m_StartPath),
                         qPrintable(GetSystemErrorString(ErrCode)));
            }
            m_Status = stFinished;
        }
    #else
        m_pDir = ::opendir(m_StartPath.toLocal8Bit().data());
        if (m_pDir != NULL) {
            m_Status = stStarted;
            getNext();
        }
        else {
            qWarning("TDirIterator::start. opendir error on \"%s\": %s",
                     qPrintable(m_StartPath),
                     qPrintable(GetSystemErrorString()));
            m_Status = stFinished;
        }
    #endif

    if (!m_FileInfoEx.isDir()) {
        qWarning("TDirIterator::start(). Not folder: \"%s\".",
                 qPrintable(m_StartPath));
        finish();
    }
}

//------------------------------------------------------------------------------

bool TDirIterator::nextRequired()
{
    if (m_Status == stFinished)
        return false;

    #ifdef Q_OS_WIN
        if (wcscmp(m_FindData.cFileName, L".") == 0 ||
            wcscmp(m_FindData.cFileName, L"..") == 0)
        {
            return true;
        }
    #else
        if (m_FileInfoEx.fileName().compare(".") == 0 ||
            m_FileInfoEx.fileName().compare("..") == 0)
        {
            return true;
        }
    #endif

        // Проверка типа объекта.
        bool OK = (m_Filters.testFlag(Dirs)  && m_FileInfoEx.isDir()) ||
                  (m_Filters.testFlag(Files) && m_FileInfoEx.isFile());


        if (OK) {
            // Проверка атрибутов объекта.
            OK = (!m_FileInfoEx.isHidden() || m_Filters.testFlag(Hidden)) &&
                 (!m_FileInfoEx.isSystem() || m_Filters.testFlag(System));
        }

        return !OK;
}

//------------------------------------------------------------------------------

void TDirIterator::getNext()
{
    if (m_Status != stStarted)
        return;

    #ifdef Q_OS_WIN
        Q_ASSERT(m_hFind != INVALID_HANDLE_VALUE);

        if (FindNextFileW(m_hFind, &m_FindData) != 0) {
            m_FileInfoEx.m_Data->init(m_StartPath, &m_FindData);
        }
        else {
            DWORD ErrCode = GetLastError();
            if (ErrCode != ERROR_NO_MORE_FILES) {
                qWarning("TDirIterator::getNext. FindNextFile error on \"%s\": %s",
                         qPrintable(m_StartPath),
                         qPrintable(GetSystemErrorString(ErrCode)));
            }
            finish();
        }
    #else
        Q_ASSERT(m_pDir != NULL);

        errno = 0;
        struct dirent64* pDirEnt = ::readdir64(m_pDir);
        if (pDirEnt != NULL) {
            m_FileInfoEx.setName(AddWithSeparator(m_StartPath,
                                                  QString::fromLocal8Bit(pDirEnt->d_name)));
        }
        else {
            if (errno != 0) {
                qWarning("TDirIterator::getNext(). readdir64 error on \"%s\": %s",
                         qPrintable(m_StartPath),
                         qPrintable(GetSystemErrorString()));
            }
            finish();
        }
    #endif
}

//------------------------------------------------------------------------------

void TDirIterator::finish()
{
    #ifdef Q_OS_WIN
        if (m_hFind != INVALID_HANDLE_VALUE) {
            FindClose(m_hFind);
            m_hFind = INVALID_HANDLE_VALUE;
        }
    #else
    if (m_pDir != NULL) {
        if (::closedir(m_pDir) != 0) {
            qWarning("TDirIterator::finish. closedir error: %s",
                     qPrintable(GetSystemErrorString()));
        }
        m_pDir = NULL;
    }
    #endif
    m_FileInfoEx.clear();
    m_Status = stFinished;
}

//------------------------------------------------------------------------------

TDirIterator::TDirIterator(const QString& StartPath, TFilters Filters)
    : m_Filters(Filters), m_Status(stNotStarted)
      #ifdef Q_OS_WIN
          ,
          m_hFind(INVALID_HANDLE_VALUE)
      #else
          ,
          m_pDir(NULL)
      #endif
{
    m_StartPath = QDir::toNativeSeparators(StartPath);
    while (m_StartPath.endsWith(QDir::separator()) && m_StartPath.length() > 1)
        m_StartPath.chop(1);
}

//------------------------------------------------------------------------------

TDirIterator::~TDirIterator()
{
    finish();
}

//------------------------------------------------------------------------------

bool TDirIterator::next()
{
    if (m_Status == stNotStarted) {
        start();
        while (nextRequired()) {
            getNext();
        }
    }
    else {
        if (m_Status == stStarted) {
            do {
                getNext();
            } while (nextRequired());
        }
    }

    return m_Status == stStarted;
}

//------------------------------------------------------------------------------

QString TDirIterator::fileName() const
{
    return m_FileInfoEx.fileName();
}

//------------------------------------------------------------------------------

QString TDirIterator::filePath() const
{
    return m_FileInfoEx.name();
}

//------------------------------------------------------------------------------

QString TDirIterator::path() const
{
    return m_StartPath;
}

//------------------------------------------------------------------------------

TFileInfoEx TDirIterator::info() const
{
    return m_FileInfoEx;
}

//------------------------------------------------------------------------------
