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

#ifndef __BUFFER__HPP__ED8BD2FE_B9D9_4A09_A6DE_EFBB66FC7FF5__
#define __BUFFER__HPP__ED8BD2FE_B9D9_4A09_A6DE_EFBB66FC7FF5__

#include <QVector>

//------------------------------------------------------------------------------
//! Ячейка буфера.

class TBufferCell
{
    private :
        int m_Size; //!< Размер ячейки (байт).
        char* m_pData;   //!< Указатель на блок данных.
    public :
        int   Length;  //!< Используемый объём буфера. (Не равен его размеру!)
        TBufferCell(int size);
        ~TBufferCell();
        int size() const;
        char* data();
};

//------------------------------------------------------------------------------
//! Буфер.

class TBuffer
{
    private :
        typedef QVector<TBufferCell*> TCellsVector;
        TCellsVector m_CellsVector;  //!< Вектор ячеек.

    public:
        TBuffer(int CellsCount, int CellSize);
        ~TBuffer();

        bool resize(int CellsCount, int CellSize);
        void clear();
        int size() const;
        int count() const;
        int cellSize() const;
        TBufferCell* at(int Index) const;
        TBufferCell* operator[](int Index) const;
        bool isAllocated() const;
};

//------------------------------------------------------------------------------

#endif // __BUFFER__HPP__ED8BD2FE_B9D9_4A09_A6DE_EFBB66FC7FF5__
