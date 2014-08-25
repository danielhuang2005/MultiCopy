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

#ifndef __CONTROLTHREAD__HPP__
#define __CONTROLTHREAD__HPP__

#include <QStringList>
#include <QFileInfo>
#include <QMetaType>

#include "ThreadEx.hpp"
#include "Settings.hpp"
#include "ErrorHandler.hpp"
#include "RWCalculator.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//               F o r w a r d   d e c l a r a t i o n s
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TProgressFormPrivate;
class TFileReader;
class TFileWriter;
class TCircularBuffer;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Задание копирования.

struct TJob {
    QStringList Srcs;  //!< Список источников.
    QStringList Dests; //!< Список назначений.
    TCopyData SettingsData;
    void clear() { Srcs.clear(); Dests.clear(); }
};

//------------------------------------------------------------------------------
//! Размер задания копирования.

struct TJobSize {
    int FilesCount;    //!< Число файлов.
    qint64 FilesSize;  //!< Суммарный размер файлов.
    //! Добавление файла.
    inline void addFile(qint64 Size)
        { ++FilesCount; FilesSize += Size; }
    //! Очистка.
    inline void clear()
        { FilesCount = 0; FilesSize = 0; }
};
Q_DECLARE_METATYPE(TJobSize);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                    T C o n t r o l T h r e a d
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TControlThread : public QThread
{
    Q_OBJECT
    public :
        //! Статус обработки задания.
        enum Status {
            Preparing,    //!< Подготовка задания к обработке.
            Calculating,  //!< Вычисление объёма задания.
            Copying,      //!< Копирование файлов.
            Finished      //!< Обработка завершена.
        };

    private :
        //! Тип вектора заданий.
        typedef QVector<TJob> TJobs;

        //! Данные, ассоциированные с потоком записи.
        struct TWriterData {
            TFileWriter* pWriter;  //!< Указатель на поток записи.
            bool DestroyPending;   //!< Флаг отложенного удаления потока.

            //! Конструктор.
            TWriterData()
                : pWriter(NULL), DestroyPending(false) {}
            //! Конструктор.
            TWriterData(TFileWriter* Writer)
                : pWriter(Writer), DestroyPending(false) {}
        };

        //! Тип вектора данных, ассоциированных с потоками записи.
        typedef QVector<TWriterData> TWritersVector;

        //! Варианты отмены операции.
        enum Cancel {
            cNoCancel = 0,  //!< Не отменять.
            cCurrent,       //!< Отменить текущее задание.
            cAll            //!< Отменить все задания.
        };

        TJobs            m_Jobs;          //!< Вектор заданий.
        TJob             m_CurrentJob;    //!< Текущее задание.
        TJobSize         m_JobSize;       //!< Размер текущего задания.
        mutable QMutex   m_JobsLocker;    //!< Мьютекс-блокировщик списка задач.
        mutable QMutex   m_ThreadsLocker; //!< Мьютекс-блокировщик доступа к
                                          //!< потокам.
        TCircularBuffer* m_pBuffer;       //!< Кольцевой буфер.
        TFileReader*     m_pReader;       //!< Поток чтения.
        TWritersVector   m_Writers;       //!< Вектор потоков записи и данных.
        QFileInfo        m_FileInfo;      //!< Информация о файле.
        TRWCalculator    m_RWCalculator;  //!< Счётчик чтения/записи.
        TErrorHandler    m_ErrorHandler;  //!< Обработчик ошибок.
        Cancel           m_Cancel;        //!< Флаг отмены операции.
        bool             m_Paused;        //!< Флаг постановки на паузу.
        Status           m_Status;        //!< Статус обработки задания.

        void createThreads(int WritersCount);
        void registerThreads();
        void destroyAllThreads();
        void destroyPendingThreads();
        void destroyThreadLater(QThread* pThread);
        void unregisterThread(TFileWriter* pFileWriter);

        void copyFile(const QString& FileName,
                      const QString& RelativePath = QString());
        void copyFolderEntry(const QString& DirName,
                             const QString& RelativePath = QString(),
                             int SubDirsDepth = -1);
        void copyFolder(const QString& DirName,
                        int SubDirsDepth = -1);
        void calculate(const QString& Name, int SubDirsDepth = -1);
        void calculate(const TJob* Job);
        void checkFreeSpace(TJob* Job);
        void cancelCurrentFile();
    protected :
        virtual void run();
    public :
        explicit TControlThread(TProgressFormPrivate* Parent);
        virtual ~TControlThread();

        void addJob(const TJob& Job);
        void pause();
        void resume();
        bool isPaused() const;
        Status status() const { return m_Status; }

        void cancelCurrentJob();
        void cancelAllJobs();

        TCircularBuffer* buffer() const { return m_pBuffer; }
        TRWCalculator* rwCalc() { return &m_RWCalculator; }
        const TJobSize* jobSize() const { return &m_JobSize; }

        TErrorHandler::Action error(TErrorHandler::ErrorData* pErrorData,
                                    QThread* pThread);
        void readedBlock(qint64 Length);
        void writedBlock(const TFileWriter* pFileWriter, qint64 Length);
    signals :
        //! Начало операции копирования.
        void begin();

        //! Начало обработки задания.
        /*!
         * \param pJob Указатель на структуру данных задания.
         *
         * \remarks Гарантируется, что указатель будет валидным как минимум
         *   до сигнала endJob.
         */
        void beginJob(const TJob* pJob);

        //! Начало процесса подсчёта размера задания.
        void beginCalculate();

        //! Конец процесса подсчёта размера задания.
        /*!
         * \param pJobSize Структура с вычисленным размером задания.
         */
        void endCalculate(TJobSize pJobSize);

        void beginCopy();

        //! Начало копирования файла.
        /*!
         * \param FileName Имя файла.
         * \param FileSize Объём файла (байт).
         */
        void beginCopyFile(QString FileName, qint64 FileSize);

        //! Сигнал прочтения очередной порции данных.
        /*!
         * \param pCalc Указатель на счётчик чтения/записи.
         */
        void readProgress(const TRWCalculator* pCalc);

        //! Сигнал записи всеми потоками очередной порции данных.
        /*!
         * \param pCalc Указатель на счётчик чтения/записи.
         */
        void writeProgress(const TRWCalculator* pCalc);

        //! Завершение копирования файла.
        void endCopyFile();

        //! Завершение обработки задания.
        void endJob();

        //! Завершение операции копирования.
        void end();

        //! Недостаточно памяти для кольцевого буфера.
        void outOfMemory();
};

//------------------------------------------------------------------------------

#endif // __CONTROLTHREAD__HPP__
