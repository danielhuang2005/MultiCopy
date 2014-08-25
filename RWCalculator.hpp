/*******************************************************************************
 *
 *            Copyright (С) 2011 Юрий Владимирович Круглов
 *
 * Эта программа является свободным программным обеспечением. Вы можете
 * распространять и/или  модифицировать её согласно условиям Стандартной
 * Общественной Лицензии GNU, опубликованной Организацией Свободного
 * Программного Обеспечения, версии 3, либо по Вашему желанию, любой более
 * поздней версии.
 *
 * Эта программа распространяется в надежде на то, что окажется полезной, но
 * БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, даже без подразумеваемых гарантий ОКУПАЕМОСТИ или
 * СООТВЕТСТВИЯ КОНКРЕТНЫМ ЦЕЛЯМ.
 * Подробнее - см. Стандартной Общественную Лицензию GNU.
 *
 * Вы должны были получить копию Основной Общественной Лицензии GNU вместе с
 * этой программой. При её отсутствии обратитесь на сайт
 * http://www.gnu.org/licenses/.
 *
 *******************************************************************************
 *
 *                 Copyright (C) 2011 Yuri V. Krugloff
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#ifndef __RWCALCULATOR__HPP__
#define __RWCALCULATOR__HPP__

//------------------------------------------------------------------------------

#include <QHash>
#include <QMutex>

#include "TimeCounter.hpp"

//------------------------------------------------------------------------------
//! Класс счётчика прочитанных и записанных байт.
/*!
 * Класс потокобезопасен (thread safe), т.е. все его методы могут быть
 * вызваны одновременно из нескольких потоков.
 */

class TRWCalculator
{
    private :
        struct TWriterData {
            bool Started;
            qint64 WritedBytes;
            TWriterData() : Started(false), WritedBytes(0) {}
        };
        /*! Тип массива указателей на процессы записи и ассоциированные с ними
            объёмы записанных байт. */
        typedef QHash<const void *const, TWriterData> TData;

        qint64 m_WritedTotal;   //!< Число байт, записанных самым медленным
                                //!< потоком во все файлы.
        qint64 m_WritedCurrent; //!< Число байт, записанных самым медленным
                                //!< потоком в текущий файл.
        qint64 m_ReadedCurrent; //!< Число прочитанных байт из текущего файла.
        TData  m_Data;          //!< Данные процессов записи.
        mutable QMutex m_Mutex; //!< Мьютекс для блокировки доступа к счётчикам.
        TTimeCounter m_TimeCounter; //!< Счётчик времени.

        bool updateWritedBytes();
    public :
        TRWCalculator();
        virtual ~TRWCalculator();
        bool registerWriter(const void *const pFileWriter);
        bool unregisterWriter(const void *const pFileWriter);
        bool isWriterRegistered(const void *const pFileWriter) const;
        void unregisterAll();
        void newFile();
        bool writeProgress(const void *const pFileWriter,
                           const qint64 WritedBytes);
        bool readProgress(const qint64 ReadedBytes);

        void begin();
        void pause();
        void resume();
        bool isPaused() const;
        void end();
        void clear();
        qint64 time() const;
        qint64 speed() const;
        qint64 remaining(qint64 Size) const;

        int writersCount() const;
        qint64 readedBytes() const;
        qint64 writedBytes() const;
        qint64 writedBytes(const void *const pFileWriter) const;
        qint64 writedTotal() const;
};

//------------------------------------------------------------------------------

#endif // __RWCALCULATOR__HPP__
