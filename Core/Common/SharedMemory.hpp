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

#ifndef __SHAREDMEMORY_HPP__EFBE88F2_69C3_4B61_88F4_4ADEA20B06D2__
#define __SHAREDMEMORY_HPP__EFBE88F2_69C3_4B61_88F4_4ADEA20B06D2__

//------------------------------------------------------------------------------

#include <QString>
#include <QSharedMemory>
#include <QSystemSemaphore>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//            t e m p l a t e    T S h a r e d M e m o r y
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Шаблон класса для работы с общей памятью.
/*!
   Шаблон является обёрткой класса QSharedMemory, обеспечивая большее удобство
   работы и дополнительную функциональность. Выделяет общую для всех
   выполняющихся на данном компьютере приложений память размера sizeof(T) и,
   если память ранее не была выделена, вызывает конструктор класса T для
   инициализации выделенной памяти.

   \remarks Не используйте в качестве аргумента шаблона классы, выделяющие
     динамическую память. Такие классы выделят память а адресном пространстве
     вызывающего экземпляра и при его закрытии память будет разрушена. Во
     избежание некорректного поведения используйте только структуры C++,
     содержащие в качестве членов простые типы данных (int,char, массивы с
     фиксированным числом элементов и т.п.).
     Для начальной инициализации членов структур определите конструктор.

   \remarks При создании второго (и последующего) экземпляра класса с тем же
     ключом происходит подключение экземпляра к выделенной области общей для
     всех выполняющихся на данном компьютере приложений памяти. Следует
     избегать создания нескольких экземпляров с разными параметрами шаблона (T)
     и одинаковыми ключами, поскольку поведение класса в таких случаях не
     определено.

   \remarks Ключ совместим с ключом класса QSharedMemory. Т.е. классы
     QSharedMemory и TSharedMemory, созданные с одинаковыми ключами, указывают
     на одну и ту же область общей памяти.
 */

template<typename T> class TSharedMemory
{
    private :
        QSharedMemory* m_pMemory;  //!< Общая память.

    public :
        explicit TSharedMemory(const QString& Key);
        TSharedMemory(const TSharedMemory<T>& other);
        virtual ~TSharedMemory();

        T* data();
        bool lock();
        bool unlock();
        QString key() const;
        QSharedMemory sharedMemory() const;

        TSharedMemory<T>& operator=(const TSharedMemory<T>& other);
        operator T*();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.
/*!
   \arg Key Ключ (уникальный идентификатор выделенной памяти)
 */

template<typename T> TSharedMemory<T>::TSharedMemory(const QString& Key)
    : m_pMemory(new QSharedMemory(Key))
{
    /* Может показаться, что для блокировки доступа достаточно использовать
       встроенный блокировщик класса QSharedMemory. Однако, это не так.
       Встроенный блокировщик (семафор) не рекурсивный и используется внутри
       функции QSharedMemory::create. Это приводит к тому, что после
       разблокировки семафора функцией create даже при предшествующей её вызову
       блокировке семафора блокировка снимается и другой поток или приложение,
       использующее данный класс, способно начать работу с выделенной памятью,
       в то время как конструктор память ещё не инициализировал. Чтобы избежать
       такой ситуации вводится системный семафор (общий для всех приложений),
       который защищает весь код конструктора. Любой другой поток будет
       блокирован до тех пор, пока конструктор не закончит работу, т.е. выделит
       и инициализирует память.
    */
    QSystemSemaphore Semaphore(QLatin1String("TShMemSysSem-") + Key, 1);
    Semaphore.acquire();

    bool Created = m_pMemory->create(sizeof(T));
    if (Created || m_pMemory->error() == QSharedMemory::AlreadyExists)
    {
        if (m_pMemory->isAttached() || m_pMemory->attach())
        {
            void* p = m_pMemory->data();
            if (p != NULL) {
                if (Created) {
                    new(p) T();
                }
            }
            else {
               qWarning("TSharedMemory. Shared memory pointer is NULL.");
            }
        }
        else {
            qWarning("TSharedMemory. Error attaching to shared memory: %s",
                     qPrintable(m_pMemory->errorString()));
        }
    }
    else {
        qWarning("TSharedMemory. Error allocating shared memory: %s",
                 qPrintable(m_pMemory->errorString()));
    }

    Semaphore.release();
}

//------------------------------------------------------------------------------
//! Копирующий конструктор.

template<typename T> TSharedMemory<T>::TSharedMemory(const TSharedMemory<T>& other)
    : m_pMemory(new QSharedMemory(other.key()))
{
    if (!m_pMemory->attach())
        qWarning("TSharedMemory. Error attaching to shared memory: %s",
                 qPrintable(m_pMemory->errorString()));
}

//------------------------------------------------------------------------------
//! Деструктор.

template<typename T> TSharedMemory<T>::~TSharedMemory()
{
    delete m_pMemory;
}

//------------------------------------------------------------------------------
//! Указатель на общую память.

template<typename T> T* TSharedMemory<T>::data()
{
    Q_ASSERT(m_pMemory != NULL);
    return static_cast<T*>(m_pMemory->data());
}

//------------------------------------------------------------------------------
//! Блокировка доступа к общей памяти.
/*!
   \remarks Блокировщик не рекурсивный!
 */

template<typename T> bool TSharedMemory<T>::lock()
{
    Q_ASSERT(m_pMemory != NULL);
    return m_pMemory->lock();
}

//------------------------------------------------------------------------------
//! Разблокировка доступа к общей памяти.
/*!
   \remarks Блокировщик не рекурсивный!
 */

template<typename T> bool TSharedMemory<T>::unlock()
{
    Q_ASSERT(m_pMemory != NULL);
    return m_pMemory->unlock();
}

//------------------------------------------------------------------------------
//! Ключ (уникальный идентификатор выделенной памяти).

template<typename T> QString TSharedMemory<T>::key() const
{
    Q_ASSERT(m_pMemory != NULL);
    return m_pMemory->key();
}

//------------------------------------------------------------------------------
//! Копия внутреннего экземпляра класса QSharedMemory.

template<typename T> QSharedMemory TSharedMemory<T>::sharedMemory() const
{
    Q_ASSERT(m_pMemory != NULL);
    return *m_pMemory;
}

//------------------------------------------------------------------------------
//! Оператор присваивания.
/*!
   \remarks После выполнения этого оператора левая часть указывает на ту же
     область общей памяти, что и правая.
 */

template<typename T> TSharedMemory<T>& TSharedMemory<T>::operator=(const TSharedMemory<T>& other)
{
    if (this != &other) {
        delete m_pMemory;
        m_pMemory = new QSharedMemory(other.key());
        if (!m_pMemory->attach())
            qWarning("TSharedMemory. Error attaching to shared memory: %s",
                     qPrintable(m_pMemory->errorString()));
    }
    return *this;
}

//------------------------------------------------------------------------------
//! Оператор преобразования к указателю на шаблонный тип.

template<typename T> TSharedMemory<T>::operator T*()
{
    return data();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//              T S h a r e d M e m o r y L o c k e r
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//! Класс для блокировки доступа к экземпляру класса типа TSharedMemory.

class TSharedMemoryLocker
{
    private :
        TSharedMemory<void>* m_sm;

    public :
        template<typename T> TSharedMemoryLocker(TSharedMemory<T>* sm)
            : m_sm(reinterpret_cast<TSharedMemory<void>* >(sm))
        {
            m_sm->lock();
        }
        ~TSharedMemoryLocker()
        {
            m_sm->unlock();
        }
};

//------------------------------------------------------------------------------

#endif // __SHAREDMEMORY_HPP__EFBE88F2_69C3_4B61_88F4_4ADEA20B06D2__
