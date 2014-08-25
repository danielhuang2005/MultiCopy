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

#include "SizeCalculator.hpp"

//------------------------------------------------------------------------------
//! Основная функция потока.

void TSizeCalculator::run()
{
    clearStateFlags();
    emit begin();
    calculateAll();
    if (isCancelled())
        emit cancelled();
    emit end(m_TaskSize);
}

//------------------------------------------------------------------------------
//! Вычисление объёма одного элемента.

void TSizeCalculator::calculateOne(const TDirEnumerator::TParams& Params)
{
    Q_ASSERT(m_pDirEnum != NULL);

    if (Params.startPath.isEmpty()) {
        qWarning("TSizeCalculator::calculateOne called with empty source.");
        return;
    }

    if (m_pDirEnum->start(Params))
    {
        do {
            pausePoint();
            if (isCancelled()) {
                break;
            }
            const TFileInfoEx* pInfo = m_pDirEnum->infoPtr();
            Q_ASSERT(pInfo != NULL);

            if (pInfo->isDir())
            {
                ++m_TaskSize.DirsCount;
            }
            else {
                ++m_TaskSize.FilesCount;
                m_TaskSize.TotalSize += pInfo->size();
            }
        } while (m_pDirEnum->next());

        m_pDirEnum->finish();
    }
    else {
        qWarning("TSizeCalculator::calculateOne. Bad source (\"%s\").",
                 qPrintable(Params.startPath));
    }
}

//------------------------------------------------------------------------------
//! Вычисление объёма всех элементов.

void TSizeCalculator::calculateAll()
{
    Q_ASSERT(m_pDirEnum != NULL);

    m_TaskSize.clear();

    if (m_Task.isNull()) {
        qWarning("Attempt to calculate size of null task.");
        return;
    }

    const TTaskSettings* pTaskSettings = &m_Task->TaskSettings;

    // Инициализация неизменяющихся параметров перечислителя.
    // TODO: Такой же код в коде класса TReader
    TDirEnumerator::TParams Params;
    Params.filter = TDirEnumerator::Files;
    if (pTaskSettings->CopyEmptyDirs)
        Params.filter |= TDirEnumerator::Dirs;
    if (pTaskSettings->CopyHidden)
        Params.filter |= TDirEnumerator::Hidden;
    if (pTaskSettings->CopySystem)
        Params.filter |= TDirEnumerator::System;
    if (pTaskSettings->FollowShortcuts)
        Params.filter |= TDirEnumerator::FollowShortcuts;
    Params.dirStatOptions = 0;
    Params.subdirsDepth = pTaskSettings->SubDirsDepth;

    const QStringList* pSrcList = &m_Task->SrcList;
    for (int i = 0; i < pSrcList->count(); ++i)
    {
        Params.startPath = pSrcList->at(i);
        calculateOne(Params);
        if (isCancelled())
            break;
    }
}

//------------------------------------------------------------------------------
//! Конструктор.

TSizeCalculator::TSizeCalculator()
    : m_pDirEnum(new TDirEnumerator())
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TSizeCalculator::~TSizeCalculator()
{
    delete m_pDirEnum;
}

//------------------------------------------------------------------------------
//! Установка задания.
/*!
   Метод устанавливает задание, размер которого будет подсчитан при запуске
   потока на выполнение методом \c QThread::start.
 */

void TSizeCalculator::setTask(const TSharedConstTask& Task)
{
    if (isRunning())
        qWarning("TSizeCalculator::init is called on running thread. Ignored.");
    else
        m_Task = Task;
}

//------------------------------------------------------------------------------
