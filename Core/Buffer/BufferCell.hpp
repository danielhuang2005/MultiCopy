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

#ifndef __BUFFERCELL__HPP__3F1C3FDD_2FEF_4C18_B08C_A54A914DD9D1__
#define __BUFFERCELL__HPP__3F1C3FDD_2FEF_4C18_B08C_A54A914DD9D1__

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

        Q_DISABLE_COPY(TBufferCell)

    public:
        TCommandData CommandData;  //! Данные команды.

        explicit TBufferCell(int Size, bool Lock);
        ~TBufferCell();

        //! Размер блока данных.
        inline int size() const { return m_Size; }

        //! Указатель на блок данных.
        inline char* data() { return m_pData; }

        //! Указатель на константный блок данных.
        inline const char* data() const { return m_pData; }

        //! Возвращает true, если память успешно выделена.
        inline bool isAllocated() const { return m_pData != 0; }

        //! Использованный объём буфера.
        inline int usedSize() const { return m_UsedSize; }

        //! Возвращает true, если выделенная память заблокирована.
        inline bool locked() const { return m_Locked; }

        //! Установка использованного объёма буфера.
        /*!
           \remarks Проверка переданного значения не производится!
        */
        inline void setUsedSize(int UsedSize) { m_UsedSize = UsedSize; }

        //! Команда.
        inline TCommand command() const { return m_Command; }

        //! Установка команды.
        inline void setCommand(TCommand Command) { m_Command = Command; }
};

//------------------------------------------------------------------------------

#endif // __BUFFERCELL__HPP__3F1C3FDD_2FEF_4C18_B08C_A54A914DD9D1__
