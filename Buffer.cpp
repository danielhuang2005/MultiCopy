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

#include "Buffer.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.
/*!
 * \param Size Объём ячейки (байт).
 */

TBufferCell::TBufferCell(int Size)
    : m_Size(Size), m_pData(NULL), Length(0)
{
    if (Size > 0)
        m_pData = (char*)malloc(Size);
}

//------------------------------------------------------------------------------
//! Деструктор

TBufferCell::~TBufferCell()
{
    free(m_pData);
    m_pData = NULL;
}

//------------------------------------------------------------------------------
//! Размер ячейки буфера.

int TBufferCell::size() const
{
    return m_Size;
}

//------------------------------------------------------------------------------
//! Указатель на блок данных.

char* TBufferCell::data()
{
    return m_pData;
}




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.
/*!
 * \param CellsCount Число ячеек.
 * \param CellSize Объём ячейки (байт).
 */

TBuffer::TBuffer(int CellsCount, int CellSize)
{
    resize(CellsCount, CellSize);
}

//------------------------------------------------------------------------------
//! Деструктор.

TBuffer::~TBuffer()
{
    clear();
}

//------------------------------------------------------------------------------
//! Изменение размера буфера.
/*!
 * \param CellsCount Число ячеек.
 * \param CellSize Объём ячейки (байт).
 *
 * \return true, если изменение размера прошло успешно и false если произошла
 *   ошибка. При возникновении ошибки буфер разрушается.
 *
 * \remarks Старое содержимое буфера разрушается!
 */

bool TBuffer::resize(int CellsCount, int CellSize)
{
    clear();
    m_CellsVector.reserve(CellsCount);
    for (int i = CellsCount; i > 0; --i)
    {
        TBufferCell* pBC = new TBufferCell(CellSize);
        if ((pBC != NULL) && (pBC->data() != NULL))
            m_CellsVector.append(pBC);
        else {
            clear();
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//! Разрушение буфера.

void TBuffer::clear()
{
    for (TCellsVector::const_iterator I = m_CellsVector.constBegin();
         I != m_CellsVector.constEnd(); ++I)
    {
        delete *I;
    }
    m_CellsVector.clear();
}

//------------------------------------------------------------------------------
//! Число ячеек в буфере.

int TBuffer::size() const
{
    return m_CellsVector.size();
}

//------------------------------------------------------------------------------
//! То же, что и \c size().

inline int TBuffer::count() const
{
    return size();
}

//------------------------------------------------------------------------------
//! Объём ячейки буфера.
/*!
 * \remarks Если буфер пуст, возвращает ноль.
 */

int TBuffer::cellSize() const
{
    if (m_CellsVector.count() > 0)
        return m_CellsVector.at(0)->size();
    else return 0;
}

//------------------------------------------------------------------------------
//! Указатель на ячейку буфера.
/*!
 * \remarks Если индекс выходит за границы допустимого диапазона, возвращает
 *   NULL.
 */

TBufferCell* TBuffer::at(int Index) const
{
    if ((Index >= 0) && (Index < size()))
        return m_CellsVector.at(Index);
    return NULL;
}

//------------------------------------------------------------------------------
//! То же, что и \c at().

TBufferCell* TBuffer::operator[](int Index) const
{
    return at(Index);
}

//------------------------------------------------------------------------------
//! Признак готовности буфера.
/*!
 * Возвращает true, если буфер готов к работе (память распределена) и false
 * в противном случае.
 */

bool TBuffer::isAllocated() const
{
    return m_CellsVector.count() > 0;
}

//------------------------------------------------------------------------------
