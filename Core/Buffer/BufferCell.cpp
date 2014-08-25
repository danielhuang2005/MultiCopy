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

#include "BufferCell.hpp"

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! Конструктор.
/*!
 * \param Size Объём ячейки (байт).
 * \param Lock Флаг блокировки страниц памяти.
 */

TBufferCell::TBufferCell(int Size, bool Lock)
    : m_Size(Size),
      m_pData(NULL),
      m_UsedSize(0),
      m_Locked(Lock),
      m_Command(cmdNoOp)
{
    if (Size > 0)
        m_pData = (char*)GetLockedMem(Size, &m_Locked);
}

//------------------------------------------------------------------------------
//! Деструктор

TBufferCell::~TBufferCell()
{
    FreeLockedMem(m_pData);
}

//------------------------------------------------------------------------------