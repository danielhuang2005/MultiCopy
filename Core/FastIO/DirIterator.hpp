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

#ifndef __DIRITERATOR__HPP__4F804DD9_F806_4C0C_AE53_83FC68C494B8__
#define __DIRITERATOR__HPP__4F804DD9_F806_4C0C_AE53_83FC68C494B8__

//------------------------------------------------------------------------------

#include <QString>
#include <QFlags>

//------------------------------------------------------------------------------

#if defined(Q_OS_WIN)
    #include <windows.h>
#else
    #include <dirent.h>
#endif

#include "FileInfoEx.hpp"

//------------------------------------------------------------------------------

class TDirIterator
{
    public :
        //! Флаги выборки объектов файловой системы.
        enum TFilter {
            Files  = 0x0001,  //!< Файлы.
            Dirs   = 0x0002,  //!< Каталоги.
            Hidden = 0x0004,  //!< Скрытые объекты.
            System = 0x0008   //!< Системные объекты.
        };
        //! Фильтр выборки объектов файловой системы.
        typedef QFlags<TFilter> TFilters;

    private :
        //! Перечисление статусов экземпляра класса.
        enum TStatus {
            stNotStarted,  //!< Перечисление ещё не запущено.
            stStarted,     //!< Перечисление запущено.
            stFinished     //!< Перечисление завершено.
        };

        QString     m_StartPath;   //!< Начальный путь перечисления.
        TFilters    m_Filters;     //!< Фильтр выборки объектов.
        TFileInfoEx m_FileInfoEx;  //!< Информация о текущем объекте.
        TStatus     m_Status;      //!< Статус экземпляра класса.

        #ifdef Q_OS_WIN
            HANDLE m_hFind;               //!< Дескриптор поиска.
            WIN32_FIND_DATAW m_FindData;  //!< Данные текущего объекта.
        #else
            DIR* m_pDir;
        #endif

        void start();
        bool nextRequired();
        void getNext();
        void finish();

    public:
        TDirIterator(const QString& StartPath, TFilters Filters);
        virtual ~TDirIterator();

        bool next();

        QString fileName() const;
        QString filePath() const;
        QString startPath() const;
        TFileInfoEx info() const;
};

//------------------------------------------------------------------------------

Q_DECLARE_OPERATORS_FOR_FLAGS(TDirIterator::TFilters)

//------------------------------------------------------------------------------

#endif // __DIRITERATOR__HPP__4F804DD9_F806_4C0C_AE53_83FC68C494B8__
