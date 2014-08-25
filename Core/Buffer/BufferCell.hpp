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

#ifndef __BUFFERCELL__HPP__
#define __BUFFERCELL__HPP__

//------------------------------------------------------------------------------

#include "Core/Task/Command.hpp"

//------------------------------------------------------------------------------

class TBufferCell
{
    private :
        int      m_Size;      //!< Размер блока данных.
        char*    m_pData;     //!< Указатель на блок данных.
        int      m_UsedSize;  //!< Использованный объём буфера.
        bool     m_Locked;    //!< Флаг блокировки памяти.
        TCommand m_Command;   //!< Команда.

        // Скрываем копирующий конструктор и оператор присваивания.
        TBufferCell(const TBufferCell&);
        TBufferCell& operator=(const TBufferCell&);
    public:
        TCommandData CommandData;

        explicit TBufferCell(int Size, bool Lock);
        ~TBufferCell();

        //! Размер блока данных.
        inline int size() const { return m_Size; }

        //! Указатель на блок данных.
        inline char* data() { return m_pData; }

        //! Возвращает true, если память успешно выделена.
        inline bool isAllocated() { return m_pData != 0; }

        //! Указатель на блок данных.
        inline const char* data() const { return m_pData; }

        //! Использованный объём буфера.
        inline int usedSize() const { return m_UsedSize; }

        //! Возвращает true, если выделенная память заблокирована.
        inline bool locked() const { return m_Locked; }

        //! Установка использованного объёма буфера.
        /*!
           \remarks Проверка переданного значения не производится!
        */
        inline void setUsedSize(int UsedSize) { m_UsedSize = UsedSize; }

        inline TCommand command() const { return m_Command; }
        inline void setCommand(TCommand Command) { m_Command = Command; }
};

//------------------------------------------------------------------------------

#endif // __BUFFERCELL__HPP__
