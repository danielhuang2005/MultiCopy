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

#ifndef __FILEINFOEX_P__HPP__
#define __FILEINFOEX_P__HPP__

#include <QSharedData>

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <sys/stat.h>
#endif

//------------------------------------------------------------------------------
//! Флаги типа объекта.

enum TFileInfoExTypeFlag {
    ffExists     = 0x0001,  //!< Объект существует.
    ffFile       = 0x0002,  //!< Файл.
    ffDir        = 0x0004,  //!< Каталог.
    ffLink       = 0x0008,  //!< Ссылка.
    ffCyclicLink = 0x0010,  //!< Циклическая ссылка.
    ffHidden     = 0x0020,  //!< Скрытый.
    ffSystem     = 0x0040,  //!< Системный.
    ffNetShare   = 0x0100,  //!< Корень сетевой папки.
    ffNetwork    = 0x0200   //!< Объект на сетевой папке.
    #ifdef Q_OS_WIN
        ,
        ffReparsePoint = 0x00010000,  //!< Точка повторной обработки.
        ffMountPoint   = 0x00020000,  //!< Точка монтирования тома.
        ffSymLink      = 0x00040000,  //!< Символическая ссылка.
        ffLnkFile      = 0x00080000   //!< .lnk-файл.
    #else
        ,
        ffChrDev       = 0x01000000,  //!< Charachter device
        ffBlockDev     = 0x02000000,  //!< Block device
        ffNamedPipe    = 0x04000000,  //!< Named pipe
        ffSocket       = 0x08000000   //!< Socket
    #endif
};

typedef QFlags<TFileInfoExTypeFlag> TFileInfoExTypeFlags;

Q_DECLARE_OPERATORS_FOR_FLAGS(TFileInfoExTypeFlags)


//------------------------------------------------------------------------------
//! Флаги проанализированных свойств.

enum TFileInfoExAnalyzedFlag {
    faLinkAnalyzed = 0x0001,  //!< Анализ на ссылку.
    faLinkTarget   = 0x0002,  //!< Анализ назначения ссылки.
    faLinkCyclic   = 0x0004,  //!< Анализ циклической ссылки.
    faLinkResolved = 0x0008,  //!< Ссылка полностью разыменована.
    faMask         = 0xffff   //!< Маска.
};

typedef QFlags<TFileInfoExAnalyzedFlag> TFileInfoExAnalyzedFlags;

Q_DECLARE_OPERATORS_FOR_FLAGS(TFileInfoExAnalyzedFlags)

//------------------------------------------------------------------------------
//! Разделяемые данные класса TFileInfoEx.

class TFileInfoExData : public QSharedData
{
    public :
        #if defined(Q_OS_WIN)
            QString          LongWinName;  //!< Длинное имя Windows.
            WIN32_FIND_DATAW FindData;     //!< Данные поиска.

            void set(const WIN32_FIND_DATAW* _FindData = NULL);
            void set(const QString& _Name, const WIN32_FIND_DATAW* _FindData);
            void init(const QString& _DirName, const WIN32_FIND_DATAW* _FindData);
        #else
            QByteArray NativeName;
            struct stat64 Stat;

            void set(const struct stat64* _Stat = NULL);
        #endif

        QString Name;                      //!< Имя объекта.
        mutable QString LinkTarget;        //!< Назначение ссылки.
        mutable QString ResolvedName;      //!< Полностью разыменованное имя.
        mutable TFileInfoExAnalyzedFlags AnalyzedFlags;
        mutable TFileInfoExTypeFlags     TypeFlags;

        TFileInfoExData();
        void clear();
        void prepareName(const QString& _Name);

};

//------------------------------------------------------------------------------

#endif // __FILEINFOEX_P__HPP__
