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

#include "QListWidget2.hpp"

#include <QMessageBox>
#include <QFileIconProvider>
#include <QStringBuilder>
#include <QDragEnterEvent>
#include <QUrl>
#include <QApplication>
#include <QDir>

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! "Укорачивание" длинных строк.
/*!
   Если строка Str длиннее чем MaxSymbols, возвращает строку длиной не более
   MaxSymbols, полученную из Str заменой лишних символов в середине на
   многоточие ("..."). Если MaxSymbols <= 3, вернёт только многоточие.
 */

QString ElideText(const QString& Str, int MaxSymbols)
{
    if (MaxSymbols <= 3)
        return "...";

    if (Str.length() <= MaxSymbols)
        return Str;

    int n = (MaxSymbols - 3) / 2;
    return Str.left(n) % QString("...") % Str.right(n);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                      Q L i s t W i d g e t 2
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

QListWidget2::QListWidget2(QWidget *parent) :
    QListWidget(parent),
    m_ShowIcons(false),
    m_ShowNetworkIcons(false),
    m_DirsOnly(false),
    m_CheckDirs(true),
    m_checkNetworkDirs(false)
{
    // Без задержанного сигнала окно приложения-источника перетаскивания
    // блокируется до завершения обработки или нажатия пользователем
    // кнопки в окне диалога.
    connect(this, SIGNAL(dropEventSignal(QStringList)),
            SLOT(dropEventSlot(QStringList)), Qt::QueuedConnection);
}

//------------------------------------------------------------------------------

void QListWidget2::addIcon(QListWidgetItem* pItem, const QString& FileName)
{
    if (m_ShowIcons && (m_ShowNetworkIcons || !isNetworkPath(FileName))) {
        QFileInfo FileInfo(FileName);
        if (FileInfo.exists()) {
            QFileIconProvider FileIconProvider;
            pItem->setIcon(FileIconProvider.icon(FileInfo));
        }
    }
    else
        pItem->setIcon(QIcon());
}

//------------------------------------------------------------------------------

void QListWidget2::updateIcons()
{
    for (int i = count() - 1; i >= 0; --i)
        addIcon(item(i), item(i)->text());
}

//------------------------------------------------------------------------------
//! Проверка возможности добавления списка элементов.
/*!
   Метод проверяет возможность добавления списка элементов.
   Вначале производится очистка входного списка от дублируемых элементов.
   Затем выполняется поиск элементов, уже присутствующих в экземпляре класса.
   Если таковые найдены, выводится диалоговое окно с сообщением о дубликатах
   и запросом дальнейших действий у пользователя. Если пользователь выбрал
   продолжение операции, возвращает true и очищает список от дублируемых
   элементов, если же пользователь отказался от добавления или все элементы уже
   присутствуют, возвращает false.

   \remarks Исходный список будет изменён! По меньшей мере, из него будут
     исключены дублируемые элементы.
   \remarks Пути должны быть абсолютными.
 */

bool QListWidget2::checkNewItems(QStringList* pList)
{
    const int MaxOutputNames = 4;  //!< Максимальное число выводимых имён.

    // Удаление дублируемых элементов.
    #ifdef Q_OS_WIN
        // Windows не различает регистр символов.
        for (int i = 0; i < pList->count() - 1; ++i)
            for (int j = i + 1; j < pList->count(); /* верно! */)
                if (pList->at(i).compare(pList->at(j), Qt::CaseInsensitive) == 0)
                    pList->removeAt(j);
                else
                    ++j;
    #else
        // В других ОС регистр имеет значение.
        pList->removeDuplicates();
    #endif

    // Текст сообщения пользователю об ошибках и предупреждениях.
    QString Message;

    // Удаление элементов, не являющихся каталогами.
    if (m_DirsOnly && m_CheckDirs)
    {
        QFileInfo FileInfo;
        QStringList NotDirs;
        for (QStringList::iterator I = pList->begin(); I != pList->end(); ++I)
        {
            if (m_checkNetworkDirs || !isNetworkPath(*I)) {
                FileInfo.setFile(*I);
                if (FileInfo.exists() && !FileInfo.isDir())
                    NotDirs << *I;
            }
        }
        for (int i = NotDirs.count() - 1; i >= 0; --i)
            pList->removeAll(NotDirs[i]);

        if (NotDirs.count() > 0) {
            if (NotDirs.count() <= 1) {
                Message += tr("This element is not folder and won't be added.") + "\n\n" +
                           NotDirs[0];
            }
            else {
                Message += tr("Some elements is not folders and won't be added.") + "\n\n";
                for (int i = 0; i < qMin(NotDirs.count(), MaxOutputNames); ++i)
                    Message += ElideText(NotDirs[i], 40) + "\n";

                if (NotDirs.count() > MaxOutputNames)
                    Message += "...";

            }
            Message += "\n\n";
        }
    }

    // Поиск уже присутствующих элементов.
    QStringList Dup;
    for (QStringList::iterator I = pList->begin(); I != pList->end(); ++I)
    {
        for (int j = 0; j < count(); ++j)
        {
            if (item(j)->text().compare(*I
                                        #ifdef Q_OS_WIN
                                            , Qt::CaseInsensitive
                                        #endif
                ) == 0) {
                Dup << *I;
                break;
            }
        }
    }

    if (Dup.count() > 0)
    {
        for (int i = Dup.count() - 1; i >= 0; --i)
            pList->removeAll(Dup[i]);

        if (Dup.count() <= 1) {
            Message += tr("This element already added.") + "\n\n" +
                       Dup[0] + "\n\n";
        }
        else {
            if (!pList->isEmpty())
                Message += tr("Some elements already added.") + "\n\n";
            else
                Message += tr("All elements already added.") + "\n\n";

            for (int i = 0; i < qMin(Dup.count(), MaxOutputNames); ++i)
                Message += ElideText(Dup[i], 40) + "\n";

            if (Dup.count() > MaxOutputNames)
                Message += "...";

            Message += "\n\n";
        }
    }

    // Строим сообщение пользователю.
    if (!Message.isEmpty())
    {
        QMessageBox::StandardButton  DefButton;
        QMessageBox::StandardButtons Buttons;
        if (!pList->isEmpty()) {
            Message += tr("Continue?");
            DefButton = QMessageBox::Yes;
            Buttons   = QMessageBox::Yes | QMessageBox::No;
        }
        else {
            Message += tr("Operation can not be completed.");
            DefButton = QMessageBox::Ok;
            Buttons   = DefButton;
        }
        QMessageBox::StandardButton B = QMessageBox::warning(
                    this,
                    QApplication::applicationName(),
                    Message,
                    Buttons,
                    DefButton);
        if (B == QMessageBox::No)
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//! Проверка возможности добавления элемента.
/*!
   Метод возвращает false если добавляемый элемент уже присутствует в списке
   и true если он отсутствует.
 */

bool QListWidget2::checkNewItem(const QString& Item)
{
    QStringList List(Item);
    return checkNewItems(&List) && !List.isEmpty();
}

//------------------------------------------------------------------------------
/*!
   Перегруженный вариант функции. Возвращает список элементов, готовый для
   добавления. Если добавление отменено, возвращает пустой список.
 */

QStringList QListWidget2::checkNewItems(const QStringList& List)
{
    QStringList Result = List;
    if (checkNewItems(&Result))
        return Result;
    return QStringList();
}

//------------------------------------------------------------------------------

void QListWidget2::dropEventSlot(QStringList Files)
{
    checkAndAddItems(Files);
}

//------------------------------------------------------------------------------

void QListWidget2::dragEnterEvent(QDragEnterEvent* Event)
{
    if (Event->mimeData()->hasUrls())
        Event->acceptProposedAction();
}

//------------------------------------------------------------------------------

void QListWidget2::dragMoveEvent(QDragMoveEvent* Event)
{
    if (Event->mimeData()->hasUrls())
        Event->acceptProposedAction();
}

//------------------------------------------------------------------------------

void QListWidget2::dropEvent(QDropEvent* Event)
{
    QList<QUrl> urls = Event->mimeData()->urls();
    if (!urls.isEmpty())
    {
        QStringList Files;
        for (int i = 0; i < urls.count(); ++i)
            Files.append(QDir::toNativeSeparators(urls.at(i).toLocalFile()));
        if (!Files.isEmpty())
        {
            // Снимаем блокировку источника перетаскивания.
            emit dropEventSignal(Files);
        }
    }
}

//------------------------------------------------------------------------------

void QListWidget2::keyPressEvent(QKeyEvent* Event)
{
    // Кнопка "Delete"
    if (Event->key() == Qt::Key_Delete && Event->modifiers() == 0) {
        Event->accept();
        deleteSelected();
        emit listChanged();
        return;
    }

    // Комбинация "Ctrl+A"
    if (Event->key() == Qt::Key_A && Event->modifiers() == Qt::ControlModifier)
    {
        Event->accept();
        selectAll();
        emit listChanged();
        return;
    }

}

//------------------------------------------------------------------------------

void QListWidget2::addItem(const QString& Name)
{
    QStringList List(Name);
    addItems(List);
}

//------------------------------------------------------------------------------

void QListWidget2::addItems(const QStringList& Names)
{
    if (Names.count() > 0)
    {
        for (int i = 0; i < Names.count(); ++i)
        {
            QListWidgetItem* pItem = new QListWidgetItem(QDir::toNativeSeparators(Names[i]));
            //pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
            QListWidget::addItem(pItem);
            addIcon(pItem, Names[i]);
        }
        emit listChanged();
    }
}

//------------------------------------------------------------------------------

bool QListWidget2::checkAndAddItem(const QString& Name)
{
    if (checkNewItem(Name)) {
        addItem(Name);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------

bool QListWidget2::checkAndAddItems(const QStringList& Names)
{
    QStringList List = Names;
    return checkAndAddItems(&List);
}

//------------------------------------------------------------------------------

bool QListWidget2::checkAndAddItems(QStringList* pList)
{
    if (checkNewItems(pList)) {
        addItems(*pList);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------

bool QListWidget2::canMoveSelection(int Delta)
{
    if (Delta == 0)
        return false;
    if (count() <= 0)
        return false;

    QList<QListWidgetItem*> Selected = selectedItems();
    if (Selected.isEmpty())
        return false;
    if (Selected.count() == count())
        return false;

    if (Delta < 0) {
        int Index = -1;
        for (int i = 0; i < count(); ++i) {
            Q_ASSERT(item(i) != NULL);
            if (item(i)->isSelected()) {
                Index = i;
                break;
            }
        }
        Q_ASSERT(Index >= 0);
        if (Index + Delta < 0)
            return false;
    }
    else {
        Q_ASSERT(Delta > 0);
        int Index = -1;
        for (int i = count() - 1; i >= 0; --i) {
            Q_ASSERT(item(i) != NULL);
            if (item(i)->isSelected()) {
                Index = i;
                break;
            }
        }
        Q_ASSERT(Index >= 0);
        if (Index + Delta >= count())
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//! Перемещение выделенных элементов на Delta позиций.
/*!
   \remarks Положительное значение Delta означает перемещение вниз,
     отрицательное - вверх.
 */

void QListWidget2::moveSelected(int Delta)
{
    if (!canMoveSelection(Delta)) {
        qWarning("QListWidget2::moveSelected. Cannot moving selection on %i.",
                 Delta);
        return;
    }

    QListWidgetItem* pCurrent = currentItem();
    QList<QListWidgetItem*> SelectedItems = selectedItems();

    Q_ASSERT(Delta != 0);

    int Begin = (Delta < 0) ? 0 : count() - 1;
    int End   = (Delta < 0) ? count() : -1;
    int Step  = (Delta < 0) ? +1 : -1;

    for (int i = Begin; i != End; i = i + Step)
    {
        QListWidgetItem* pItem = item(i);
        Q_ASSERT(pItem != NULL);
        pItem->setSelected(false);
        if (SelectedItems.contains(pItem)) {
            pItem = takeItem(i);
            insertItem(i + Delta, pItem);
        }
    }

    setCurrentItem(pCurrent);
    for (int i = 0; i < count(); ++i)
    {
        QListWidgetItem* pItem = item(i);
        pItem->setSelected(SelectedItems.contains(pItem));
    }
}

//------------------------------------------------------------------------------

bool QListWidget2::canMoveUpSelected()
{
    return canMoveSelection(-1);
}

//------------------------------------------------------------------------------

bool QListWidget2::canMoveDownSelected()
{
    return canMoveSelection(+1);
}

//------------------------------------------------------------------------------

bool QListWidget2::showIcons() const
{
    return m_ShowIcons;
}

//------------------------------------------------------------------------------

void QListWidget2::setShowIcons(bool Show)
{
    if (m_ShowIcons != Show) {
        m_ShowIcons = Show;
        updateIcons();
    }
}

//------------------------------------------------------------------------------

bool QListWidget2::dirsOnly() const
{
    return m_DirsOnly;
}

//------------------------------------------------------------------------------

void QListWidget2::setDirsOnly(bool DirsOnly)
{
    m_DirsOnly = DirsOnly;
}

//------------------------------------------------------------------------------

bool QListWidget2::showNetworkIcons() const
{
    return m_ShowNetworkIcons;
}

//------------------------------------------------------------------------------

void QListWidget2::setShowNetworkIcons(bool Show)
{
    if (m_ShowNetworkIcons != Show) {
        m_ShowNetworkIcons = Show;
        updateIcons();
    }
}

//------------------------------------------------------------------------------

bool QListWidget2::checkDirs() const
{
    return m_CheckDirs;
}

//------------------------------------------------------------------------------

void QListWidget2::setCheckDirs(bool Check)
{
    m_CheckDirs = Check;
}

//------------------------------------------------------------------------------

bool QListWidget2::checkNetworkDirs() const
{
    return m_checkNetworkDirs;
}

//------------------------------------------------------------------------------

void QListWidget2::setCheckNetworkDirs(bool Check)
{
    m_checkNetworkDirs = Check;
}

//------------------------------------------------------------------------------
//! Добавление элементов к списку.

void QListWidget2::toStringList(QStringList* pList) const
{
    for (int i =  0; i < count(); ++i)
        pList->append(item(i)->text());
}

//------------------------------------------------------------------------------

QStringList QListWidget2::toStringList() const
{
    QStringList Result;
    toStringList(&Result);
    return Result;
}

//------------------------------------------------------------------------------

void QListWidget2::moveUpSelected()
{
    moveSelected(-1);
/*    if (count() <= 0) {
        qWarning("QListWidget2::moveUpSelected. List is Empty.");
        return;
    }

    if (count() == 1) {
        qWarning("QListWidget2::moveUpSelected. Single element cannot be moved.");
        return;
    }

    Q_ASSERT(count() > 1);

    if (item(0)->isSelected()) {
        qWarning("QListWidget2::moveUpSelected. First element cannot be moved up.");
        return;
    }

    // Сохраняем выделенные элементы.
    QList<QListWidgetItem*> SelectedItems = selectedItems();
    if (SelectedItems.isEmpty()) {
        qWarning("QListWidget2::moveUpSelected. No selected items.");
        return;
    }

    // Сохраняем текущий элемент.
    QListWidgetItem* pCurrent = currentItem();

    for (int i = 1; i < count(); ++i)
    {
        QListWidgetItem* pItem = item(i);
        Q_ASSERT(pItem != NULL);
        if (SelectedItems.contains(pItem)) {
            pItem = takeItem(i);
            insertItem(i - 1, pItem);
        }
    }

    for (int i = 0; i < count(); ++i)
    {
        QListWidgetItem* pItem = item(i);
        if (pItem == pCurrent)
            setCurrentItem(pItem);

        pItem->setSelected(SelectedItems.contains(pItem));
    }
*/
}

//------------------------------------------------------------------------------

void QListWidget2::moveDownSelected()
{
    moveSelected(+1);
}

//------------------------------------------------------------------------------

void QListWidget2::deleteSelected()
{
    QList<QListWidgetItem*> SelectedItems = selectedItems();

    if (!SelectedItems.isEmpty()) {
        for (int i = SelectedItems.count() - 1; i >= 0; --i)
            delete SelectedItems.at(i);
    }
    else {
        qWarning("QListWidget2::deleteSelected. No selected items.");
    }
}

//------------------------------------------------------------------------------
