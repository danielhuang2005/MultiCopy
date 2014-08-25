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

#ifndef __FILEWRITER__HPP__
#define __FILEWRITER__HPP__

#include <QFile>

#include "ThreadEx.hpp"

//------------------------------------------------------------------------------

class TControlThread;

//------------------------------------------------------------------------------
//! Класс, записывающий файл из кольцевого буфера.

class TFileWriter : public TThreadEx
{
    private :
        QFile m_File;                      //!< Файл.
        TControlThread* m_pControlThread;  //!< Управляющий поток.
        bool m_Cancel;                     //!< Флаг отмены операции.
        qint64 m_Size;                     //!< Требуемый объём файла.
        qint64 m_Written;                  //!< Записанный объём данных.

        qint64 writeBlock();
    protected :
        virtual void run();
    public :
        explicit TFileWriter(TControlThread* pControlThread);
        virtual ~TFileWriter();

        QString fileName() const;
        bool openFile(const QString& FileName, qint64 Size);
        void closeFile();
        void cancel();
        bool isCancelled() const;
        bool readyToRun() const;
        qint64 written() const;
};

//------------------------------------------------------------------------------

#endif // __FILEWRITER__HPP__
