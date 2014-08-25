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

#ifndef __TASKBARCONTROL__HPP__F4628C13_1BA4_4789_B085_DBEA3A3AEF1F__
#define __TASKBARCONTROL__HPP__F4628C13_1BA4_4789_B085_DBEA3A3AEF1F__

//------------------------------------------------------------------------------

#include <QObject>
#include <QVector>
#include <QPixmap>

#ifdef Q_OS_WIN
    #include "TaskbarControl_win.hpp"
#endif

//------------------------------------------------------------------------------

class QWidget;

class TTaskbarControl;
struct TTaskbarControlData;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Прогресс-бар на кнопке в панели задач.
/*!
   \remarks Экземпляры класса создаются автоматически классом
     \c TTaskbarControl.
 */

class TTaskbarProgress
{
    friend class TTaskbarControl;
    public :
        //! Состояние прогресс-бара.
        enum TState {
            NoProgress    = 0x0,  //!< Нет отображения.
            Indeterminate = 0x1,  //!< Неопределённое
            Normal        = 0x2,  //!< Нормальное
            Error         = 0x4,  //!< Ошибка
            Paused        = 0x8   //!< Пауза
        };

    private :
        TTaskbarControlData* m_pData;  //!< Указатель на общие данные.

        void applyProgress();
        void applyState();

        TTaskbarProgress(TTaskbarControlData* pData);
        virtual ~TTaskbarProgress();
        Q_DISABLE_COPY(TTaskbarProgress)

    public :
        TState state() const;
        void   setState(TState State);
        int  value() const;
        void setValue(int Value);
        int  minimum() const;
        void setMinimum(int Minimum);
        int  maximum() const;
        void setMaximum(int Maximum);
        void setProgress(int Minimum, int Maximum, int Value);
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Оверлейная иконка на кнопке в панели задач.
/*!
   \remarks Экземпляры класса создаются автоматически классом
     \c TTaskbarControl.
 */

class TTaskbarOverlayIcon
{
    friend class TTaskbarControl;
    private :
        TTaskbarControlData* m_pData;  //!< Указатель на общие данные.

        void applyIcon();

        TTaskbarOverlayIcon(TTaskbarControlData* pData);
        virtual ~TTaskbarOverlayIcon();
        Q_DISABLE_COPY(TTaskbarOverlayIcon)

    public :
        QPixmap pixmap() const;
        void setPixmap(const QPixmap& Pixmap);
        void remove();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Управление кнопкой в панели задач.

class TTaskbarControl : public QObject
{
    private :
        typedef QVector<TTaskbarControlData*> TDataMap;
        static TDataMap      m_DataMap;
        TTaskbarControlData* m_pData;
        TTaskbarProgress*    m_pProgressBar;
        TTaskbarOverlayIcon* m_pOverlayIcon;

    public:
        explicit TTaskbarControl(QWidget* Parent);
        TTaskbarControl(const TTaskbarControl& other);
        virtual ~TTaskbarControl();
        TTaskbarControl& operator=(const TTaskbarControl& other);

        TTaskbarProgress* progressBar();
        TTaskbarOverlayIcon* overlayIcon();
};

//------------------------------------------------------------------------------

#endif // __TASKBARCONTROL__HPP__F4628C13_1BA4_4789_B085_DBEA3A3AEF1F__
