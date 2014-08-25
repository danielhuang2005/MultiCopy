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

#ifndef __COREAPPINSTANCES__HPP__769E9E95_4C0E_4DE0_B5F9_B8CF6A5FBFC0__
#define __COREAPPINSTANCES__HPP__769E9E95_4C0E_4DE0_B5F9_B8CF6A5FBFC0__

//------------------------------------------------------------------------------

#include <QObject>

//------------------------------------------------------------------------------

class QLocalServer;
template<typename T> class TSharedMemory;

//------------------------------------------------------------------------------
/*!
   Класс для определения числа ранее запущенных экземпляров программы.
   Позволяет получить общее число уже запущенных экземпляров, порядковый номер
   текущего, а также передавать строковые сообщения первому запущенному
   экземпляру.

   Пример использования данного класса.
   \code
     #include <QCoreApplication>
     #include "CoreAppInstances.hpp"

     int main(int argc, char *argv[])
     {
         QCoreApplication a(argc, argv);

         TCoreAppInstances CoreAppInstances;
         if (!CoreAppInstances.isFirst()) {
             sendMessageToFirst("This is a message.");
             return 0;
         }

         return a.exec();
     }
   \endcode

   \remarks В пределах одного приложения нельзя создавать несколько экземпляров
     объектов данного класса с одинаковым AppId. Каждый последующий созданный
     объект увеличит счётчики. Однако, можно создавать несколько экземпляров с
     различными AppId.

   \remarks Не следует разрушать объект до завершения работы программы,
     поскольку после его разрушения другие экземпляры программы не будут знать о
     существовании данного экземпляра.
 */

class TCoreAppInstances : public QObject
{
    Q_OBJECT
    private :
        struct TSharedData;
        TSharedMemory<TSharedData>* m_pSharedMemory;  //!< Общая память.
        QString       m_AppId;         //!< Идентификатор приложения.
        TSharedData*  m_pSharedData;   //!< Указатель на общие данные.
        QLocalServer* m_pLocalServer;  //!< Сокет для приёма сообщений.
        int           m_Index;         //!< Порядковый номер текущего
                                       //!< экземпляра приложения.

        void init();

        Q_DISABLE_COPY(TCoreAppInstances)

    public:
        explicit TCoreAppInstances(const QString& AppId, QObject* Parent = NULL);
        explicit TCoreAppInstances(QObject* Parent = NULL);
        virtual ~TCoreAppInstances();

        bool sendMessageToFirst(const QString& Message = QString());
        int instancesCount() const;

        //! Идентификатор приложения.
        inline QString appId() const { return m_AppId; }
        //! Метод возвращает true, если данный экземпляр приложения первый.
        inline bool isFirst() const { return m_Index == 0; }
        //! Метод возвращает true, если данный экземпляр приложения не первый.
        inline bool isNext() const { return !isFirst(); }
        //! То же, что и \c isNext.
        inline bool isRunning() const { return isNext(); }
        //! Порядковый номер (индекс) текущего экземпляра приложения.
        inline int index() const { return m_Index; }

    private slots :
        void newConnection();

    signals :
        //! Сигнал о получении сообщения.
        /*!
           Сигнал генерируется в случае, когда другой экземпляр приложения
           посылает сообщение первому экземпляру. В строке Message содержится
           текст сообщения (может быть пустым).
         */
        void messageReceived(QString Message);
};

//------------------------------------------------------------------------------

#endif // __COREAPPINSTANCES__HPP__769E9E95_4C0E_4DE0_B5F9_B8CF6A5FBFC0__
