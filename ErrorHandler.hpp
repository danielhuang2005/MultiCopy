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

#ifndef __ERRORHANDLER__HPP__
#define __ERRORHANDLER__HPP__

#include <QObject>
#include <QMutex>
#include <QMap>

//------------------------------------------------------------------------------

class TProgressFormPrivate;
class QPushButton2;

//------------------------------------------------------------------------------
//! Обработчик ошибок.

class TErrorHandler : public QObject
{
    Q_OBJECT
    public :
        //! Типы ошибок.
        enum Error {
            eOpenFile,      //!< Ошибка при открытии файла (для чтения).
            eReadFile,      //!< Ошибка при чтении файла.
            eMakeDir,       //!< Ошибка при создании каталога.
            eAlreadyExists, //!< Файл уже существует
            eCreateFile,    //!< Ошибка при создании файла.
            eWriteFile      //!< Ошибка при записи файла.
        };
        //! Возможные действия.
        enum Action {
            aNoAction,         //!< Нет действия.
            aOverwrite,        //!< Перезаписать файл.
            aOverwriteAll,     //!< Перезаписать все файлы.
            aRetry,            //!< Повторить.
            aSkip,             //!< Пропустить.
            aSkipAll,          //!< Пропустить всё.
            aCancelDest,       //!< Отменить копирование в данное назначение.
            aCancelCurrentJob, //!< Отменить текущее задание.
            aCancelAllJobs     //!< Отменить все задания.
        };
        //! Структура с информацией об ошибке.
        struct ErrorData {
            Error   Code;      //!< Тип (код) ошибки.
            QString Message;   //!< Сообщение системы об ошибке.
            QString FileName;  //!< Имя файла (каталога), обработка которого
                               //!< вызвала ошибку.
        };

    private :
        //! Список последних выбранных действий.
        typedef QMap<Error, Action> TLastActions;

        mutable QMutex m_Mutex;        //!< Мьютекс для блокировки доступа.
        ErrorData      m_ErrorData;    //!< Информация об ошибке.
        Action         m_LastAction;   //!< Последнее выбранное действие.
        TLastActions   m_LastActions;  //!< Последние выбранные действия.

        QString errorText() const;
        QPushButton2* newButton(const Action A) const;
    public:
        explicit TErrorHandler(TProgressFormPrivate* Parent);
        virtual ~TErrorHandler();

        void messageBox(QWidget* Parent, int WritersCount);
        void clear();
        Action error(const ErrorData& ErrorData);
        Action lastAction() const;
    signals :
        void showMessageBox(TErrorHandler* pErrorHandler);
};

//------------------------------------------------------------------------------

#endif // __ERRORHANDLER__HPP__
