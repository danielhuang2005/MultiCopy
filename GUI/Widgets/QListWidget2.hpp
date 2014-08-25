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

#ifndef QLISTWIDGET2_HPP
#define QLISTWIDGET2_HPP

#include <QListWidget>

//------------------------------------------------------------------------------

class QListWidget2 : public QListWidget
{
    Q_OBJECT
    private :
        bool m_ShowIcons;
        bool m_ShowNetworkIcons;
        bool m_DirsOnly;
        bool m_CheckDirs;
        bool m_checkNetworkDirs;

        void addIcon(QListWidgetItem* pItem, const QString& FileName);
        void updateIcons();

        bool checkNewItems(QStringList* pList);
        bool checkNewItem(const QString& Item);
        QStringList checkNewItems(const QStringList& List);
    private slots :
        void dropEventSlot(QStringList Files);
    protected :
        virtual void dragEnterEvent(QDragEnterEvent *Event);
        virtual void dragMoveEvent(QDragMoveEvent *Event);
        virtual void dropEvent(QDropEvent *Event);
    public:
        explicit QListWidget2(QWidget *parent = 0);

        void addItem(const QString& Name);
        void addItems(const QStringList& Names);
        bool checkAndAddItem(const QString& Name);
        bool checkAndAddItems(const QStringList& Names);
        bool checkAndAddItems(QStringList* pList);

        bool showIcons() const;
        void setShowIcons(bool Show);
        bool dirsOnly() const;
        void setDirsOnly(bool DirsOnly);
        bool showNetworkIcons() const;
        void setShowNetworkIcons(bool Show);
        bool checkDirs() const;
        void setCheckDirs(bool Check);
        bool checkNetworkDirs() const;
        void setCheckNetworkDirs(bool Check);

        void toStringList(QStringList* pList) const;
        QStringList toStringList() const;
    signals :
        void dropEventSignal(QStringList Files);
        void listChanged();
};

//------------------------------------------------------------------------------

#endif // QLISTWIDGET2_HPP
