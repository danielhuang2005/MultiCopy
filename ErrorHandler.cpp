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

#include "ErrorHandler.hpp"

#include <QMessageBox>
#include <QApplication>
#include <QPushButton>
#include <QSet>
#include <QMap>
#include <QDir>

#include "ProgressForm.hpp"

//------------------------------------------------------------------------------
//! Кнопка с дополнительными данными.

class QPushButton2 : public QPushButton
{
    private :
        TErrorHandler::Action m_Action;  //!< Действие.
    public :
        //! Конструктор.
        explicit QPushButton2(TErrorHandler::Action Action, QWidget* Parent = NULL)
            : QPushButton(Parent), m_Action(Action)
            {}
        //! Деструктор.
        virtual ~QPushButton2() {}
        //! Действие, связанное с кнопкой.
        inline TErrorHandler::Action action() const
            { return m_Action; }
        //! Установка действия, связанного с кнопкой.
        inline void setAction(TErrorHandler::Action Action)
            { m_Action = Action; }
};

//------------------------------------------------------------------------------

typedef QMap<TErrorHandler::Error, QString> TErrorText;

typedef QSet<TErrorHandler::Action> TActionSet;
typedef QMap<TErrorHandler::Error, TActionSet> TActionMap;

TActionMap fillActionMap()
{
    TActionMap Map;
    TActionSet Set;

    // eOpenFile
    Set << TErrorHandler::aRetry
        << TErrorHandler::aSkip
        << TErrorHandler::aSkipAll
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eOpenFile, Set);

    // eReadFile
    Set.clear();
    Set << TErrorHandler::aRetry
        << TErrorHandler::aSkip
        << TErrorHandler::aSkipAll
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eReadFile, Set);

    // eMakeDir
    Set.clear();
    Set << TErrorHandler::aRetry
        << TErrorHandler::aSkip
        << TErrorHandler::aSkipAll
        << TErrorHandler::aCancelDest
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eMakeDir, Set);

    // eAlreadyExists
    Set.clear();
    Set << TErrorHandler::aOverwrite
        << TErrorHandler::aOverwriteAll
        << TErrorHandler::aSkip
        << TErrorHandler::aSkipAll
        << TErrorHandler::aCancelDest
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eAlreadyExists, Set);

    // eCreateFile
    Set.clear();
    Set << TErrorHandler::aRetry
        << TErrorHandler::aSkip
        << TErrorHandler::aSkipAll
        << TErrorHandler::aCancelDest
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eCreateFile, Set);

    // eWriteFile
    Set.clear();
    Set << TErrorHandler::aRetry
        << TErrorHandler::aSkip
        << TErrorHandler::aSkipAll
        << TErrorHandler::aCancelDest
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eWriteFile, Set);

    // eNoFreeSpace
    Set.clear();
    Set << TErrorHandler::aRetry
        << TErrorHandler::aIgnore
        << TErrorHandler::aIgnoreAll
        << TErrorHandler::aCancelDest
        << TErrorHandler::aCancelCurrentJob
        /*<< TErrorHandler::aCancelAllJobs*/;
    Map.insert(TErrorHandler::eNoFreeSpace, Set);

    return Map;
}

//------------------------------------------------------------------------------
//! Конструктор.

TErrorHandler::TErrorHandler(TProgressFormPrivate* Parent)
    : QObject(Parent), m_LastAction(aNoAction)
{
    /* Qt позволяет создавать виджеты только в главном потоке приложения.
       Поскольку обработчик ошибок будет вызван из других потоков, вывести на
       экран диалоговое окно он не сможет. Поэтому используется механизм
       сигналов и слотов. Одно из окон имеет слот, вызывающий метод этого
       класса, ответственный за вывод диалгового окна. Поскольку сгенерировавший
       сигнал поток должен ожидать завершения работы вызванных методов,
       сигнал и слот соединяются с использованием параметра
       Qt::BlockingQueuedConnection*/
    connect(this, SIGNAL(showMessageBox(TErrorHandler*)),
            Parent, SLOT(showMessageBox(TErrorHandler*)),
            Qt::BlockingQueuedConnection);
}

//------------------------------------------------------------------------------
//! Деструктор.

TErrorHandler::~TErrorHandler()
{
}

//------------------------------------------------------------------------------

QString TErrorHandler::errorText() const
{
    switch (m_ErrorData.Code)
    {
        case eOpenFile :
            return tr("Error openig file");
        case eReadFile :
            return tr("Error reading file");
        case eMakeDir :
            return tr("Error creating folder");
        case eAlreadyExists :
            return tr("File already exists");
        case eCreateFile :
            return tr("Error creating file");
        case eWriteFile :
            return tr("Error writing file");
        case eNoFreeSpace :
            return tr("Not enough free space");
        default :
            return tr("Unknown error at processing file");
    }
}

//------------------------------------------------------------------------------

QPushButton2* TErrorHandler::newButton(const TErrorHandler::Action A) const
{
    QPushButton2* pBtn = new QPushButton2(A);

    switch (A)
    {
        case TErrorHandler::aOverwrite :
            pBtn->setText(tr("&Overwrite"));
            pBtn->setToolTip(tr("Overvrite current file"));
            break;
        case TErrorHandler::aOverwriteAll :
            pBtn->setText(tr("O&verwrite All"));
            pBtn->setToolTip(tr("Overwrite all files without question"));
            break;
        case TErrorHandler::aRetry :
            pBtn->setText(tr("&Retry"));
            pBtn->setToolTip(tr("Retry operation"));
            break;
        case TErrorHandler::aIgnore :
            pBtn->setText(tr("&Ignore"));
            pBtn->setToolTip(tr("Ignore this warning and continue"));
            break;
        case TErrorHandler::aIgnoreAll :
            pBtn->setText(tr("I&gnore All"));
            pBtn->setToolTip(tr("Ignore this warning and continue"));
            break;
        case TErrorHandler::aSkip :
            pBtn->setText(tr("&Skip"));
            pBtn->setToolTip(tr("Skip current file"));
            break;
        case TErrorHandler::aSkipAll :
            pBtn->setText(tr("Skip &All"));
            pBtn->setToolTip(tr("Skip all files without questions"));
            break;
        case TErrorHandler::aCancelDest :
            pBtn->setText(tr("Cancel &Dest"));
            pBtn->setToolTip(tr("Cancel copying to this destination"));
            break;
        case TErrorHandler::aCancelCurrentJob :
            pBtn->setText(tr("&Cancel"));
            pBtn->setToolTip(tr("Cancel job"));
            break;
        case TErrorHandler::aCancelAllJobs :
            pBtn->setText(tr("Cancel A&ll"));
            pBtn->setToolTip(tr("Cancel all jobs"));
            break;
        default :
            ;
    }
    return pBtn;
}

//------------------------------------------------------------------------------

void TErrorHandler::messageBox(QWidget* Parent/*, int WritersCount*/)
{
    static const TActionMap  ActionsMap  = fillActionMap();

    QMessageBox Box(Parent);
    Box.setIcon(QMessageBox::Warning);
    Box.setWindowTitle(QApplication::applicationName());
    QString Msg = errorText() + "\n\n" +
            QDir::toNativeSeparators(m_ErrorData.FileName);
    if (!m_ErrorData.Message.isEmpty())
        Msg += "\n\n" + m_ErrorData.Message;
    Box.setText(Msg);

    // Добавление кнопок в диалог.
    const TActionSet& Actions = ActionsMap[m_ErrorData.Code];
    Q_ASSERT(!Actions.isEmpty());
    for (TActionSet::const_iterator I = Actions.constBegin();
         I != Actions.constEnd(); ++I)
    {
        // Скрываем кнопку отмены текущего назначения, если поток записи
        // только один.
        if ((*I == aCancelDest) && (m_ErrorData.DestsCount < 2))
            continue;

        QPushButton2* pBtn = newButton(*I);
        Box.addButton(pBtn, QMessageBox::ActionRole);
    }

    Box.exec();
    QPushButton2* pClicked = dynamic_cast<QPushButton2*>(Box.clickedButton());
    Q_ASSERT(pClicked != NULL);
    m_LastAction = pClicked->action();

    // Созданные кнопки разрушаются диалогом, поэтому удалять их оператором
    // delete не нужно.
}

//------------------------------------------------------------------------------

void TErrorHandler::clear()
{
    m_LastActions.clear();
    m_LastAction = aNoAction;
}

//------------------------------------------------------------------------------

TErrorHandler::Action TErrorHandler::error(const TErrorHandler::ErrorData& ErrorData)
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    if ((m_LastAction == aCancelAllJobs) || (m_LastAction == aCancelCurrentJob))
        return m_LastAction;

    // Проверка ранее выбранных действий.
    if (m_LastActions.contains(ErrorData.Code))
    {
        Action A = m_LastActions.value(ErrorData.Code);
        if ((A == aOverwriteAll) || (A == aSkipAll) || (A == aIgnoreAll))
        {
            return A;
        }
    }

    // Запрос действия у пользователя.
    m_ErrorData = ErrorData;
    emit showMessageBox(this);
    return m_LastActions[ErrorData.Code] = m_LastAction;
}

//------------------------------------------------------------------------------

TErrorHandler::Action TErrorHandler::lastAction() const
{
    QMutexLocker Locker(&m_Mutex);
    Q_UNUSED(Locker);

    return m_LastAction;
}

//------------------------------------------------------------------------------
