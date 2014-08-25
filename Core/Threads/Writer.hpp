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

#ifndef __WRITER__HPP__
#define __WRITER__HPP__

#include "Core/Threads/ThreadEx.hpp"
#include "Core/Task/Task.hpp"
#include "Core/FastIO/FastFile.hpp"

//------------------------------------------------------------------------------

class QSemaphore;

class TBufferCell;
class TCircularBuffer;
class TTaskStatus;
struct TErrorData;

//------------------------------------------------------------------------------
//! Поток, записывающий данные.

class TWriter : public TThreadEx
{
    Q_OBJECT
    private :
        //! Структура информации о файле-назначении.
        struct TFileData {
            QString   DestDir;      //!< Каталог назначения.
            TFastFile File;         //!< Файл.
            qint64    WritedBytes;  //!< Число записанных в файл байт.
            bool      Skip;         //!< Признак пропуска текущего файла.
            bool      Cancel;

            TFileData(const QString& Dest)
                : DestDir(Dest), WritedBytes(0), Skip(false), Cancel(false) { }
        };
        typedef QList<TFileData*> TFileDataList;

        TSharedConstTask  m_Task;         //!< Задание.
        TCircularBuffer*  m_pBuffer;      //!< Кольцевой буфер.
        TFileDataList     m_FileData;     //!< Файлы и их данные.
        QString           m_FileRelName;  //!< Относительное имя файла.
        int               m_FileNumber;   //!< Порядковый номер файла.
        qint64            m_Size;         //!< Размер обрабатываемого файла.
        TTaskStatus*      m_pTaskStatus;  //!< Статус задания.

        const TBufferCell* acquireCell();
        void releaseCell();
        bool createDir(TFileData* pFileData, const QString& DirName);

        bool newFile(TFileData* pFileData, const QString& FileName);
        void newFiles(const TBufferCell* pCell);
        void newDir(TFileData *pFileData, const QString& DirName);
        void newDir(const TBufferCell *pCell);
        void writeBlock(TFileData* pFileData, const TBufferCell* pCell);
        void writeBlock(const TBufferCell* pCell);
        void closeFile(TFileData* pFileData, const TBufferCell* pCell = NULL);
        void closeFiles(const TBufferCell* pCell = NULL);
        void setFileStat(TFileData* pFileData, const TBufferCell* pCell);
        void setFileStat(const TBufferCell* pCell);
        void setDirStat(TFileData *pFileData, const TBufferCell* pCell);
        void setDirStat(const TBufferCell* pCell);
        void uncompleteFile(TFileData* pFileData, const TBufferCell* pCell);
        void uncompleteFile(const TBufferCell* pCell);
        void checkDests();
        void process();
        void errorHandler(TFileData* pFileData, TErrorData* pErrorData);

        Q_DISABLE_COPY(TWriter)
    protected :
        virtual void run();
    public:
        explicit TWriter();
        virtual ~TWriter();

        inline TSharedConstTask task() const { return m_Task; }
        void setTask(TSharedConstTask Task);

        QStringList dests() const;
        int destsCount() const;
        void setDest(const QString& Dest);
        void addDest(const QString& Dest);
        void setDests(const QStringList& Dests);
        void addDests(const QStringList& Dests);
        void clearDests();
        int openedFiles() const;
        QList<const void*> handlers() const;
        int activeHandlersCount() const;
        void setTaskStatus(TTaskStatus* TaskStatus);
        QString fileRelName() const;

        //! То же, что и \c destsCount.
        inline int handlersCount() const { return destsCount(); }
        //! Указатель на кольцевой буфер.
        inline TCircularBuffer* buffer() { return m_pBuffer; }
        //! Указатель на кольцевой буфер.
        inline const TCircularBuffer* buffer() const { return m_pBuffer; }
        //! Установка указателя на кольцевой буфер.
        inline void setBuffer(TCircularBuffer* pBuffer) { m_pBuffer = pBuffer; }
        //! Порядковый номер обрабатываемого файла.
        inline int fileNumber() const { return m_FileNumber; }

        //! Указатель на статус задания.
        inline TTaskStatus* taskStatus() { return m_pTaskStatus; }
        //! Указатель на статус задания.
        inline const TTaskStatus* taskStatus() const { return m_pTaskStatus; }

    signals :
        void error(TErrorData* pErrData);
};

//------------------------------------------------------------------------------

#endif // __WRITER__HPP__
