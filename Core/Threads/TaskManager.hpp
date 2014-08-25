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

#ifndef __TASKMANAGER__HPP__
#define __TASKMANAGER__HPP__

#include <QThread>
#include <QQueue>

#include "Core/Task/Task.hpp"
#include "Core/Errors/LastActions.hpp"

//------------------------------------------------------------------------------

class QSemaphore;

class TRWCalculator;
class TSizeCalculator;
class TReader;
class TWriter;
class TTaskStatus;
class TCircularBuffer;
struct TErrorData;

//------------------------------------------------------------------------------

class TTaskManager : public QThread
{
    Q_OBJECT
    private :
        typedef QQueue<TErrorData*> TErrorsQueue;

        class TDestsCounter {
            private :
                int     m_Dests;
                int     m_Skipped;
                QString m_FileRelName;
            public :
                TDestsCounter();
                void init(int DestsCount);
                int skip(const QString& FileRelName);
                void clear();
                void cancelDest();

                inline int dests() const { return m_Dests; }
                inline int skipped() const { return m_Skipped; }
                inline QString fileRelName() const { return m_FileRelName; }
        };

        TErrorsQueue     m_ErrorsQueue;   //!< Очередь ошибок на обработку.
        TTaskList        m_TaskList;      //!< Список задач.
        TSharedConstTask m_pCurrentTask;  //!< Текущая задача.
        QAtomicInt       m_Paused;        //!< Флаг постановки на паузу.
        bool             m_Cancel;        //!< Флаг отмены выполнения.
        TLastActions     m_LastActions;
        bool             m_UserPromptInProcess;  //!< Флаг вызова диалогового
                                                 //!< запроса пользователю.
        QAtomicInt       m_LockProcessErrors;    //!< Флаг запрета обработки ошибок.
        TDestsCounter    m_DestsCounter;

        TReader*         m_pReader;      //!< Процесс чтения.
        QList<TWriter*>  m_WritersList;  //!< Процессы записи.
        TSizeCalculator* m_pSizeCalc;    //!< Вычислитель размера задания.
        TTaskStatus*     m_pTaskStatus;  //!< Состояние задания.
        TCircularBuffer* m_pBuffer;      //!< Кольцевой буфер.

        void bufferSizeAutodetect(int* pCellCount, int* pCellSize);
        void checkFreeSpace(QStringList* pDestList);

        void initThreads(const QStringList& DestList);
        void processCurrentTask();
        void processAllTasks();
        bool userPromptRequired(TErrorData* pErrorData);
        void processNextError(TErrorData* pErrorData);
        void processNextError();
        void skipDest(TWriter* pWriter);
        void cancelDest();
        void finishProcessError(TErrorData* pErrorData);

        // Скрываем копирующий конструктор и оператор присваивания.
        Q_DISABLE_COPY(TTaskManager)

    protected :
        virtual void run();

    public :
        explicit TTaskManager(QObject* Parent);
        virtual ~TTaskManager();

        void addTask(TSharedConstTask Task);
        void addTasks(const TTaskList& TaskList);
        void deleteTask(TSharedConstTask Task);
        void deleteTasks(const TTaskList& TaskList);
        void moveTask(int Index, int Delta);
        TSharedConstTask currentTask() const;

        TTaskSize taskSize() const;

        void pause();
        void resume();
        bool isPaused() const;
        bool isCalculating() const;
        void cancelCurrentTask();
        void cancelAllTasks();

        void lockProcessErrors();
        void unlockProcessErrors();

        //! Возвращает true, если текущее задание было отменено.
        inline bool isCancelled() const { return m_Cancel; }
        //! Возвращает true, если вызывается запрос пользователю.
        inline bool isUserPromptInProcess() const { return m_UserPromptInProcess; }
        //! Указатель на процесс чтения.
        inline const TReader* reader() const { return m_pReader; }
        //! Указатель на состояние задания.
        inline const TTaskStatus* taskStatus() const { return m_pTaskStatus; }
        //! Указатель на вычислитель размера задания.
        inline const TSizeCalculator* sizeCalc() const { return m_pSizeCalc; }

    signals :
        void begin();
        void beginTask(TSharedConstTask Task);
        void beginCalculate();
        void endCalculate(TTaskSize TaskSize);
        void beginCopy();
        void endCopy();
        void endTask(TSharedConstTask Task);
        void cancelTasks(TTaskList TaskList);
        void end();

        void error(TErrorData* pErrorData);
        void outOfMemory();

    private slots :
        void errorReceiver(TErrorData* pErrorData);
        void errorProcessed(TErrorData* pErrorData);
};

//------------------------------------------------------------------------------

#endif // __TASKMANAGER__HPP__
