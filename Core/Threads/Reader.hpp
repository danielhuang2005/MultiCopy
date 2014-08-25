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

#ifndef __READER__HPP__
#define __READER__HPP__

#include "Core/Threads/ThreadEx.hpp"
#include "Core/Task/Task.hpp"
#include "Core/FastIO/FileInfoEx.hpp"
#include "Core/FastIO/FastFile.hpp"
#include "Core/IO/DirEnumerator.hpp"

//#include <QFileInfo>

//------------------------------------------------------------------------------

class QSemaphore;

class TTaskStatus;
class TBufferCell;
class TCircularBuffer;
class TFileInfoEx;
struct TErrorData;

//------------------------------------------------------------------------------
//! Поток, читающий данные.

class TReader : public TThreadEx
{
    Q_OBJECT
    private:
        TSharedConstTask   m_Task;            //!< Задание.
        TFastFile          m_File;            //!< Считываемый файл.
        QString            m_FileRelName;     //!< Относительное имя файла.
        int                m_FileNumber;      //!< Порядковый номер файла.
        TDirEnumerator*    m_pDirEnumerator;  //!< Перечислитель файлов.
        TFileStatOptions   m_FileStatOptions; //!< Список считываемых параметров.
                                              //!< файлов и каталогов.
        TTaskStatus*       m_pTaskStatus;     //!< Статус задания.
        TCircularBuffer*   m_pBuffer;         //!< Кольцевой буфер.
        const TFileInfoEx* m_pFileInfoEx;     //!< Информация о текущем
                                              //!< обрабатываемом объекте.
        qint64             m_Readed;          //!< Число байт, прочитанных из
                                              //!< текущего файла.
        bool               m_Skip;            //!< Флаг пропуска текущего файла.

        TBufferCell* acquireCell();
        void releaseCell();

        bool openFile();
        qint64 readNextBlock();
        bool readFile();
        void processFile();
        void processDirIn();
        void processDirOut();
        void processSource(const TDirEnumerator::TParams& Params);
        void process();
        void errorHandler(TErrorData* pErrorData);

        Q_DISABLE_COPY(TReader)
    protected :
        virtual void run();
    public:
        explicit TReader();
        virtual ~TReader();

        inline TSharedConstTask task() const { return m_Task; }
        void setTask(TSharedConstTask Task);
        void setBuffer(TCircularBuffer* pBuffer);
        void skip();
        QString fileRelName() const;

        //! Указатель на статус задания.
        inline TTaskStatus* taskStatus() { return m_pTaskStatus; }
        //! Указатель на статус задания.
        inline const TTaskStatus* taskStatus() const { return m_pTaskStatus; }
        //! Установка указателя на статус задания.
        inline void setTaskStatus(TTaskStatus* TaskStatus)
            { m_pTaskStatus = TaskStatus; }

        //! Указатель на кольцевой буфер.
        inline TCircularBuffer* buffer() { return m_pBuffer; }
        inline const TCircularBuffer* buffer() const { return m_pBuffer; }
        inline qint64 readed() const { return m_Readed; }
        inline bool isSkipped() const { return m_Skip; }
        /*! Порядковый номер обрабатываемого файла.
           \remarks Учитываются только файлы, которые удалось успешно открыть.
         */
        inline int fileNumber() const { return m_FileNumber; }

    signals :
        void error(TErrorData* pErrorData);
};

//------------------------------------------------------------------------------

#endif // __READER__HPP__
