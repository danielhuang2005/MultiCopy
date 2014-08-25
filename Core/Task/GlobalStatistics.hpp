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

#ifndef __GLOBALSTATISTICS__HPP__
#define __GLOBALSTATISTICS__HPP__

//------------------------------------------------------------------------------

#include <QtGlobal>
#include <QString>

//------------------------------------------------------------------------------

class QSettings;
template<typename T> class TSharedMemory;

//------------------------------------------------------------------------------
//! Класс для подсчёта общей статистики работы программы.

class TGlobalStatistics
{
    public :
        struct TStat {
            qint64 BytesReaded;     //!< Число прочитанных байт.
            qint64 BytesWrited;     //!< Число записанных байт.
            qint64 FilesReaded;     //!< Число прочитанных файлов.
            qint64 FilesWrited;     //!< Число записанных файлов.
            qint64 TasksCompleted;  //!< Число завершённых задач.

            void clear();
            TStat();
        };

    private :
        struct TStat2 : public TStat {
            bool Readed;  //!< Флаг заполнения структуры данными.
            void clear();
            TStat2();
        };

        static const QString DefaultGroup;

        TSharedMemory<TStat2>* m_pSharedMemory;  //!< Общая память.
        TStat2*                m_pStat2;         //!< Данные в общей памяти.

        Q_DISABLE_COPY(TGlobalStatistics)
        explicit TGlobalStatistics();
        virtual ~TGlobalStatistics();

    public:
        static TGlobalStatistics* instance();
        void read(QSettings* pSettings, QString Group = QString());
        void write(QSettings* pSettings, QString Group = QString());

        void append(const TStat& Stat);
        TStat get() const;
};

//------------------------------------------------------------------------------

#endif // __GLOBALSTATISTIC__HPP__
