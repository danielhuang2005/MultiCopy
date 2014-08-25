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

#ifndef __TASKSTATUS__HPP__
#define __TASKSTATUS__HPP__

#include <QString>
#include <QHash>
#include <QMutex>

#include "Core/TimeCounter.hpp"
#include "Core/Task/Task.hpp"

//------------------------------------------------------------------------------

class TGlobalStatistics;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TTaskStatus
{
    private:
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        // Типы

        //! Структура общей статистики.
        struct TCounters {
            private :
                int    m_FilesCompleted;         //!< Обработано файлов.
                qint64 m_CurrentProcessedBytes;  //!< Обработано байт в текущем файле.
                qint64 m_TotalProcessedBytes;    //!< Обработано байт во всех файлах.
                int    m_FilesSkipped;           //!< Пропущено файлов.
                qint64 m_CurrentSkippedBytes;    //!< Пропущено байт в текущем файле.
                qint64 m_TotalSkippedBytes;      //!< Пропущено байт во всех файлах.

            public :
                TCounters();
                void clear();
                void end();
                void nextNormal();
                void addBytes(qint64 Bytes);
                void skipBytes(qint64 Bytes);
                void nextSkipped(qint64 Bytes = 0);
                int files() const;
                qint64 currentBytes() const;
                qint64 totalBytes() const;

                inline int filesCompleted() const { return m_FilesCompleted; }
                inline int filesSkipped() const { return m_FilesSkipped; }
                inline qint64 currentProcessedBytes() const { return m_CurrentProcessedBytes; }
                inline qint64 currentSkippedBytes() const { return m_CurrentSkippedBytes; }
                inline qint64 totalProcessedBytes() const { return m_TotalProcessedBytes; }
                inline qint64 totalSkippedBytes() const { return m_TotalSkippedBytes; }
        };

        //----------------------------------------------------------------------
        //! Структура статистики потока чтения.
        struct TReaderStat {
            private :
                TCounters m_Counters;     //!< Счётчики.
                QString   m_DirName;      //!< Каталог-источник.
                QString   m_RelName;      //!< Имя текущего файла относительно
                                          //!< каталога-источника.
                qint64   m_CurrentSize;   //!< Размер текущего файла (байт).
            public :
                TReaderStat();
                void clear();
                void end();

                inline TCounters* counters() { return &m_Counters; }
                inline const TCounters* counters() const { return &m_Counters; }
                inline QString dirName() const { return m_DirName; }
                inline void setDirName(const QString& DirName) { m_DirName = DirName; }
                inline QString relName() const { return m_RelName; }
                inline void setRelName(const QString& RelName) { m_RelName = RelName; }
                inline qint64 currentSize() const { return m_CurrentSize; }
                inline void setCurrentSize(const qint64 Size) { m_CurrentSize = Size; }
        };

        //----------------------------------------------------------------------
        //! Массив счётчиков потока записи.
        typedef QHash<const void*, TCounters> TCountersMap;

        //----------------------------------------------------------------------
        //! Структура статистики потока записи.
        struct TWriterStat {
            private :
                TCountersMap m_CountersMap; //!< Счётчики.
                QString      m_RelName;     //!< Имя текущего файла относительно
                                            //!< каталога-назначения.
                qint64       m_Size;        //!< Размер текущего файла (байт).
            public :
                TWriterStat();

                bool isHandlerRegistered(const void* pHandler) const;
                int handlersCount() const;
                TCounters* counters(const void* pHandler);
                const TCounters* counters(const void* pHandler) const;
                TCounters* registerHandler(const void* pHandler);
                bool unregisterHandler(const void* pHandler);
                const TCounters* slowest(const void** ppHandler = NULL) const;

                void clear();
                void end();

                void newFile(const QString& RelName, qint64 Size);
                inline QString relName() const { return m_RelName; }
                inline qint64 size() const { return m_Size; }
        };

        //----------------------------------------------------------------------
        //! Массив статистики потоков записи.
        typedef QHash<const void*, TWriterStat> TWritersStat;

        //----------------------------------------------------------------------
        struct TTotalStat {
            private :
                int    m_TotalWritedFiles;
                qint64 m_TotalWritedBytes;
            public :
                TTotalStat();
                void addWritedBytes(qint64 WritedBytes);
                void addWritedFiles(int Count = 1);
                inline qint64 writedBytes() const { return m_TotalWritedBytes; }
                inline int writedFiles() const { return m_TotalWritedFiles; }
                void clear();
        };


        //----------------------------------------------------------------------

        struct TFinalStat {
            int Files;
            qint64 Bytes;
            TFinalStat() : Files(0), Bytes(0) {}
        };

        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        // Внутренние переменные.

        mutable QMutex m_ReaderMutex;   //!< Мьютекс-блокировщик доступа к
                                        //!< статистике потоков чтения.
        mutable QMutex m_WritersMutex;  //!< Мьютекс-блокировщик доступа к
                                        //!< статистике потоков записи.
        TReaderStat  m_ReaderStat;      //!< Статистика потока чтения.
        TWritersStat m_WritersStat;     //!< Статистика потоков записи.
        TTimeCounter m_TimeCounter;     //!< Счётчик времени.
        TTotalStat   m_TotalStat;       //!< Общая статистика.
        TTaskSize    m_TaskSize;        //!< Размер задания.
        TFinalStat   m_FinalStat;

        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        // Внутренние функции.

        TWriterStat* writerStat(const void* pWriter);
        const TWriterStat* writerStat(const void *pWriter) const;
        TCounters* counters(const void* pWriter, const void* pHandler);
        bool counters(const void *pWriter, const void *pHandler,
                      TWriterStat **ppWriterStat,
                      TCounters** ppCounters);
        bool counters(const void *pWriter, const void *pHandler,
                      const TWriterStat **ppWriterStat ,
                      const TCounters **ppCounters) const;
        const TCounters* counters(const void* pWriter, const void* pHandler) const;
        float currentReadedPercent_Private() const;
        int readedFiles_Private() const;
        int handlersCount_Private() const;
        qint64 slowestWriter(const TWriterStat **ppWriterStat = NULL,
                             const TCounters **ppCounters = NULL) const;
        void unregisterWriter_Private(const void* pWriter,
                                      const TCounters* pCounters = NULL);

        // Скрываем конструктор по умолчанию и оператор присваивания.
        Q_DISABLE_COPY(TTaskStatus)


    public:
        //! Статус операции чтения или записи.
        struct TStatus {
            QString DirName;       //!< Каталог-источник.
            QString RelName;       //!< Имя обрабатываемого файла относительно
                                   //!< каталога-источника.
            int     Files;         //!< Число обработанных файлов.
            qint64  Bytes;         //!< Число обработанных байт в текущем файле.
            float   Percent;       //!< Процент завершения текущего файла.
            qint64  TotalBytes;    //!< Число обработанных байт во всех файлах.
            float   TotalPercent;  //!< Процент завершения задания.
        };

        //! Время и скорость обработки.
        struct TSpeedAndTime {
            qint64 ElapsedTime;    //!< Прошло времени (миллисекунды).
            qint64 RemainingTime;  //!< Осталось времени (миллисекунды).
            qint64 Speed;          //!< Скорость (байт в секунду).
        };

        explicit TTaskStatus();
        virtual ~TTaskStatus();

        bool registerWriter(const void* pWriter, const void* pHandler);
        int  registerWriter(const void* pWriter, QList<const void*> Handlers);
        bool unregisterWriter(const void* pWriter, const void* Handler);
        int  unregisterWriter(const void* pWriter, QList<const void*> Handlers);
        bool unregisterWriter(const void* pWriter);
        bool isWriterRegistered(const void* pWriter) const;
        bool isWriterRegistered(const void* pWriter, const void* Handler) const;
        void unregisterAllWriters();
        int  writersCount() const;
        int  handlersCount(const void* pWriter) const;
        int  handlersCount() const;

        void readerBeginTask();
        void readerNewFile(const QString& DirName, const QString& RelName,
                           qint64 Size = -1);
        void readerProgress(qint64 DeltaBytes);
        void readerSkipFile(const QString& DirName, const QString& RelName,
                            qint64 Size = 0);
        void readerSkip(qint64 DeltaBytes);
        void readerEndFile();
        void readerEndTask();

        void writerBeginTask(const void* pWriter);
        void writerNewFile(const void* pWriter, const QString& RelName,
                           qint64 Size = -1);
        void writerNewFile(const void* pWriter, const void* pHandler);
        void writerProgress(const void* pWriter, const void* pHandler,
                            qint64 DeltaBytes);
        void writerSkipFile(const void* pWriter, const void* pHandler);
        void writerSkip(const void* pWriter, const void* pHandler,
                        qint64 DeltaBytes);
        void writerEndFile(const void* pWriter, const void* pHandler);
        void writerEndTask(const void* pWriter);

        void begin();
        void end();
        void clear();
        void pause();
        void resume();
        bool isStarted() const;
        bool isPaused() const;
        void speedAndTime(TSpeedAndTime* pSpeedAndTime) const;
        TSpeedAndTime speedAndTime() const;

        QString readedFileName() const;
        QString readedFileRelName() const;
        QString readedFileDirName() const;
        int     readedFiles() const;
        float   currentReadedPercent() const;
        TStatus readingStatus() const;

        QString slowestWritedFileName() const;
        int     slowestWritedFiles() const;
        qint64  slowestCurrentWritedBytes() const;
        float   slowestCurrentWritedPercent() const;
        TStatus writingStatus() const;

        TTaskSize taskSize() const;
        const TTaskSize* taskSizePtr() const;
        void setTaskSize(const TTaskSize* pTaskSize);
        void setTaskSize(const TTaskSize& TaskSize);
        void clearTaskSize();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // __TASKSTATUS__HPP__
