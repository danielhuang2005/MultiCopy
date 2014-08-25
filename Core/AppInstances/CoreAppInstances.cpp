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

#include "CoreAppInstances.hpp"

#include <QCoreApplication>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include "../Common/SharedMemory.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Общие данные.

struct TCoreAppInstances::TSharedData {
    int Instances;  //!< Число запущенных экземпляров программы.
    int Index;      //!< Индекс последнего запущенного экземпляра программы.
    //! Конструктор.
    TSharedData()
        : Instances(0), Index(-1)
    { }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Метод инициализации.

void TCoreAppInstances::init()
{
    QString AppId;
    if (m_AppId.isEmpty()) {
        AppId = QCoreApplication::applicationFilePath();
        #ifdef Q_OS_WIN
            AppId = AppId.toLower();
        #endif
    }
    else {
        AppId = m_AppId;
    }
    AppId = QLatin1String("TSingleInstance-") + AppId.toUtf8().toHex();


    // Общая память.
    m_pSharedMemory = new TSharedMemory<TSharedData>(AppId);
    m_pSharedData = m_pSharedMemory->data();
    {
        TSharedMemoryLocker Locker(m_pSharedMemory);
        Q_UNUSED(Locker);

        ++m_pSharedData->Instances;
        m_Index = ++m_pSharedData->Index;
    }

    // Прослушивающий сокет.
    m_pLocalServer = new QLocalServer(this);
    if (m_pLocalServer->listen(AppId)) {
        connect(m_pLocalServer, SIGNAL(newConnection()),
                SLOT(newConnection()));
    }
    else {
        qWarning("TCoreAppInstances::init. Error listening: %s.",
                 qPrintable(m_pLocalServer->errorString()));
    }
}

//------------------------------------------------------------------------------
//! Конструктор.
/*!
   \arg AppId "Ключ", уникальный идентификатор приложения.
   \arg Parent Указатель на родительский объект.

   \remarks Следует внимательно отнестись к выбору уникального идентификатора
     приложения. Если два различных приложения будут иметь один и тот же
     уникальный идентификатор, они будут восприняты как одно.
 */

TCoreAppInstances::TCoreAppInstances(const QString& AppId, QObject* Parent)
    : QObject(Parent),
      m_pSharedMemory(NULL),
      m_AppId(AppId),
      m_pSharedData(NULL),
      m_pLocalServer(NULL),
      m_Index(0)
{
    init();
}

//------------------------------------------------------------------------------
//! Конструктор.
/*!
   \arg Parent Указатель на родительский объект.

   \remarks Данный вариант конструктора генерирует уникальный идентификатор
     приложения, основываясь на пути к исполняемому файлу. Данный вариант
     следует использовать с осторожностью, поскольку если одно и то же
     приложение будет запускаться из разных мест или иметь различные имена
     исполняемых файлов, то их уникальные идентификаторы окажутся различными и
     они будут восприняты как различные приложения.
 */

TCoreAppInstances::TCoreAppInstances(QObject* Parent)
    : QObject(Parent),
      m_pSharedMemory(NULL),
      m_pSharedData(NULL),
      m_pLocalServer(NULL),
      m_Index(0)
{
    init();
}

//------------------------------------------------------------------------------
//! Деструктор.

TCoreAppInstances::~TCoreAppInstances()
{
    delete m_pLocalServer;
    if (m_pSharedData) {
        TSharedMemoryLocker Locker(m_pSharedMemory);
        Q_UNUSED(Locker);

        --m_pSharedData->Instances;
    }
    delete m_pSharedMemory;
}

//------------------------------------------------------------------------------
//! Обработчик новых соединений с прослушивающим сокетом.

void TCoreAppInstances::newConnection()
{
    QLocalSocket* pSocket = m_pLocalServer->nextPendingConnection();
    if (pSocket == NULL) {
        qWarning("TCoreAppInstances::newConnection. "
                 "nextPendingConnection return empty socket.");
        return;
    }

    QByteArray Message;
    while (pSocket->waitForReadyRead(1000))
        Message.append(pSocket->readAll());

    emit messageReceived(QString::fromUtf8(Message));

    delete pSocket;
}

//------------------------------------------------------------------------------
//! Отсылка сообщения первому экземпляру приложения.
/*!
   Метод отсылает первому экземпляру приложения сообщение Message.

   \sa messageReceived
 */

bool TCoreAppInstances::sendMessageToFirst(const QString& Message)
{
    QLocalSocket Socket;
    Socket.connectToServer(m_pSharedMemory->key());
    if (Socket.waitForConnected(1000)) {
        if (!Message.isEmpty()) {
            Socket.write(Message.toUtf8());
            Socket.waitForBytesWritten(1000);
        }
        Socket.close();
        return true;
    }
    else {
        qWarning("TAppInstances::sendMessage. Cannot connect to server.\n"
                 "\"%s\"", qPrintable(Socket.errorString()));
    }
    return false;
}

//------------------------------------------------------------------------------
//! Число запущенных экземпляров приложения.

int TCoreAppInstances::instancesCount() const
{
    if (m_pSharedMemory != NULL && m_pSharedData != NULL)
    {
        TSharedMemoryLocker Locker(m_pSharedMemory);
        Q_UNUSED(Locker);

        return m_pSharedData->Instances;
    }

    return 0;
}

//------------------------------------------------------------------------------
