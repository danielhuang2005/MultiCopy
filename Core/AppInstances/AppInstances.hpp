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

#ifndef __APPINSTANCES__HPP__769E9E95_4C0E_4DE0_B5F9_B8CF6A5FBFC0__
#define __APPINSTANCES__HPP__769E9E95_4C0E_4DE0_B5F9_B8CF6A5FBFC0__

#include "CoreAppInstances.hpp"

//------------------------------------------------------------------------------
/*!
   Расширение класса TCoreAppInstances для работы с приложениями с графическим
   интерфейсом. В дополнение к функциональности базового класса, позволяет
   выводить первый экземпляр приложения на передний план.

   Примеры использования.
   Пример 1. Запуск только одной копии приложения и активация окна уже
     запущенного экземпляра при попытке запуска нового экземпляра.
   \code
     #include <QApplication>
     #include <QWidget>
     #include "AppInstances.hpp"

     int main(int argc, char *argv[])
     {
         QApplication a(argc, argv);

         TAppInstances AppInstances;
         if (!AppInstances.isFirst()) {
             AppInstances.activateFirst();
             return 0;
         }

         QWidget w;
         w.show();
         AppInstances.setActivationWindow(&w);
         AppInstances.setActivateOnMessage(true);

         return a.exec();
     }
   \endcode

   Пример 2. Каждый вновь запущенный экземпляр приложения будет иметь в
     заголовке своего главного окна порядковый номер.
   \code
     #include <QApplication>
     #include <QWidget>
     #include "AppInstances.hpp"

     int main(int argc, char *argv[])
     {
         QApplication a(argc, argv);

         TAppInstances AppInstances;
         QWidget w;
         w.setWindowTitle(QString("[%1] ").arg(AppInstances.index() + 1) +
                          w.windowTitle());
         w.show();

         return a.exec();
     }
   \endcode
 */

class TAppInstances : public TCoreAppInstances
{
    Q_OBJECT
    private :
        QWidget* m_pActivationWidget; //!< Виджет для активации.
        bool     m_ActivateOnMessage; //!< Активировать при получении сообщения.

        void init();

        Q_DISABLE_COPY(TAppInstances)

    public:
        explicit TAppInstances(const QString& AppId, QObject* Parent = NULL);
        explicit TAppInstances(QObject* Parent = NULL);
        virtual ~TAppInstances();

        bool activateFirst();

        /*! Установка окна, которое будет активировано при получении сообщения.
           \sa activationWidget, activateOnMessage
           \remarks Если активируемое окно не установлено, класс попытается при
             получении сообщения активировать последнее созданное окно верхнего
             уровня, не являющееся скрытым.
         */
        inline void setActivationWindow(QWidget* Window)
            { m_pActivationWidget = Window; }

        /*! Окно, которое будет активировано при получении сообщения.
           \sa setActivationWindow, activateOnMessage.
         */
        inline QWidget* activationWindow() const
            { return m_pActivationWidget; }

        /*! Установка флага активации окна при получении сообщения.
           \sa activateOnMessage, activationWindow
         */
        inline void setActivateOnMessage(bool Activate)
            { m_ActivateOnMessage = Activate; }

        /*! Флаг активации окна при получении сообщения.
          \sa setActivateOnMessage, activationWindow
         */
        inline bool activateOnMessage() const
            { return m_ActivateOnMessage; }

    private slots :
        void activateWindow();
};

//------------------------------------------------------------------------------

#endif // __APPINSTANCES__HPP__769E9E95_4C0E_4DE0_B5F9_B8CF6A5FBFC0__
