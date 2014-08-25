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

#ifndef __FILEINFOEX__HPP__
#define __FILEINFOEX__HPP__

//------------------------------------------------------------------------------

#include <QString>
#include <QSharedData>

//------------------------------------------------------------------------------

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <utime.h>
#endif

//------------------------------------------------------------------------------
//! Параметры статистической информации о файлах/каталогах.

enum TFileStatOption {
    fsoTime = 0x01,  //!< Дата/время.
    fsoAttr = 0x02   //!< Атрибуты.
};

typedef QFlags<TFileStatOption> TFileStatOptions;

Q_DECLARE_OPERATORS_FOR_FLAGS(TFileStatOptions)

//------------------------------------------------------------------------------
//! Статистическая информация о файлах/каталогах.

struct TFileStat {
    TFileStatOptions Options;     //!< Параметры.
    #ifdef Q_OS_WIN
        FILETIME CreationTime;    //!< Время создания.
        FILETIME LastAccessTime;  //!< Время последнего доступа.
        FILETIME LastWriteTime;   //!< Время последнего изменения.
        DWORD    Attr;            //!< Атрибуты файла.
    #else
        time_t AccessTime;        //!< Время последнего доступа.
        time_t ModificationTime;  //!< Время последнего изменения.
        mode_t Mode;              //!< Права доступа.
    #endif

    TFileStat();
    void clear();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TFileInfoExData;

//------------------------------------------------------------------------------
//! Класс предоставляет расширенную информацию об объектах файловой системы.

class TFileInfoEx
{
    private :
        friend class TDirIterator;

        QSharedDataPointer<TFileInfoExData> m_Data;  //!< Данные.

        void getInfo(const QString& Name);
        void resolveLink() const;
        void fullResolveLink() const;
        void analyzeCyclicLink() const;

    public:
        TFileInfoEx();
        TFileInfoEx(const TFileInfoEx& other);
        TFileInfoEx(const QString& Name);
        virtual ~TFileInfoEx();

        TFileInfoEx& operator=(const TFileInfoEx& other);

        QString name() const;
        QString fileName() const;
        void setName(const QString& Name);
        void refresh();
        void clear();

        bool exists() const;
        bool isFile() const;
        bool isDir() const;
        bool isHidden() const;
        bool isSystem() const;
        bool isLink() const;
        bool isShortcut() const;
        bool isCyclicLink() const;
        qint64 size() const;

        TFileStat stat(TFileStatOptions What = fsoTime | fsoAttr) const;
        TFileStatOptions copyStatTo(const QString& Target,
                                    TFileStatOptions What = fsoTime | fsoAttr) const;
        TFileStatOptions setStat(const TFileStat& Stat);

        QString linkTarget() const;
        QString fullyResolvedLink() const;

        static bool exists(const QString& Name);
        static TFileStatOptions setStat(const QString& Name, const TFileStat& Stat);
};

//------------------------------------------------------------------------------

#endif // __FILEINFOEX__HPP__
