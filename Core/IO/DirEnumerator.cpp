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

#include "DirEnumerator.hpp"

#include <QDir>
#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! Анализ текущего элемента.
/*!
   \remarks Метод может вызывать startDir.
 */

TDirEnumerator::TAnalyseResult TDirEnumerator::analyze()
{
    if ((m_Params.filter.testFlag(FollowShortcuts) && m_FileInfoEx.isShortcut()) ||
        (m_FileInfoEx.isLink() && !m_FileInfoEx.isShortcut()))
    {
        // Следуем по ссылкам.
        if (!m_FileInfoEx.isCyclicLink())
        {
            // Разыменовываем ссылку. Если ссылка неправильная
            // (ссылается на отсутствующий файл), canonicalFilePath
            // вернёт пустую строку.
            QString ResolvedPath = m_FileInfoEx.fullyResolvedLink();
            if (!ResolvedPath.isEmpty())
            {
                QString OrigName = m_FileInfoEx.fileName();
                bool isLinkFile = m_FileInfoEx.isFile();
                m_FileInfoEx.setName(ResolvedPath);

                // Здесь все ссылки уже разыменованы.
                if (m_FileInfoEx.isDir())
                {
                    // Каталог.
                    if (isLinkFile) {
                        // Для ссылок-файлов подставляем имя каталога
                        // назначения.
                        startDir();
                    }
                    else {
                        // Для ссылок-каталогов используем имя ссылки.
                        startDir(&OrigName);
                    }
                    if (m_Params.filter.testFlag(Dirs))
                    {
                        // Если каталоги нужно выдавать, прерываем обработку.
                        return arOK;
                    }
                    else {
                        // Требуем следующий элемент.
                        return arNext;
                    }
                }
                else {
                    //Файл. Прерываем обработку.
                    return arOK;
                }
            }
            else {
                // Ссылка неправильная. Требуем следующий элемент.
                return arBad;
            }
        }
        else {
            // Циклическую ссылку пропускаем и требуем следующий элемент.
            return arBad;
        }
    }
    else {
        // По ссылкам не следуем.
        if (m_FileInfoEx.isDir())
        {
            // Инициализируем просмотр нового каталога.
            startDir();

            if (m_Params.filter.testFlag(Dirs))
            {
                // Если каталоги нужно выдавать, прерываем обработку.
                return arOK;
            }
            else {
                // Требуем следующий элемент.
                return arNext;
            }
        }
        else {
            // Это не каталог. Прерываем обработку.
            return arOK;
        }
    }
}

//------------------------------------------------------------------------------
//! Запуск перечисления элементов каталога.

bool TDirEnumerator::start()
{
    finish();  // Защита от повторного запуска.

    // Вначале проверяем существование объекта.
    m_FileInfoEx.setName(m_Params.startPath);
    if (!m_FileInfoEx.exists())
        return false;

    /* Инициализация фильтра для QDirIterator. Флаг QDir::Dirs здесь не
       устанавливается, поскольку его необходимость зависит от глубины
       вложения просматриваемого подкаталога. */
    if (m_Params.filter.testFlag(Files))
        m_DirFilters |= TDirIterator::Files;
    if (m_Params.filter.testFlag(Hidden))
        m_DirFilters |= TDirIterator::Hidden;
    if (m_Params.filter.testFlag(System))
        m_DirFilters |= TDirIterator::System;

    m_RelPath.clear();
    m_RelPathValid = true;
    TAnalyseResult AnalyzeResult = analyze();

    if (AnalyzeResult != arBad)
    {
        Q_ASSERT(m_InIterators.count() <= 1);
        Q_ASSERT(m_OutIterators.isEmpty());

        m_Iterators += m_InIterators;
        m_InIterators.clear();
        m_RelPathValid = false;

        if (AnalyzeResult == arNext)
            m_Started = next();
        else
            m_Started = true;
    }

    return m_Started;
}

//------------------------------------------------------------------------------
//! Начало перечисления элементов каталога.
/*!
   \arg pAltName Указатель на строку с альтернативным именем каталога (имя,
     полученное перечислителем, будет заменено на альтернативное).
 */

void TDirEnumerator::startDir(const QString* pAltName)
{
    TDirIterator::TFilters Filters = m_DirFilters;
    if ((m_Params.subdirsDepth < 0) ||
        (m_Iterators.count() + m_InIterators.count() < m_Params.subdirsDepth))
    {
        Filters |= TDirIterator::Dirs;
    }

    QString DirName;  // Имя каталога.
    if (pAltName != NULL && !pAltName->isEmpty())
        DirName = *pAltName;
    else
        DirName = m_FileInfoEx.fileName();

    QString RelDirName = relPath();  // Относительное имя каталога.
    for (TIteratorsStack::const_iterator It = m_InIterators.constBegin();
         It != m_InIterators.constEnd(); ++It)
    {
        AddWithSeparator(&RelDirName, It->Name);
    }
    AddWithSeparator(&RelDirName, DirName);

    TDirIterator* pI = new TDirIterator(m_FileInfoEx.name(), Filters);
    TIterator It(pI, DirName, RelDirName, m_FileInfoEx);
    m_InIterators.push(It);
}

//------------------------------------------------------------------------------
//! Завершение перечисления элементов каталога.

void TDirEnumerator::endDir()
{
    Q_ASSERT(!m_Iterators.isEmpty() || !m_InIterators.isEmpty());

    if (!m_InIterators.isEmpty()) {
        // Если есть новые итераторы, очищаем сначала их.
        m_InIterators.pop();
    }
    else {
        // Если новых итераторов нет, начинаем очищать основные.
        Q_ASSERT(!m_Iterators.isEmpty());

        TIterator It = m_Iterators.pop();
        m_RelPathValid = false;
        // QDirIterator уже не нужен. Сразу очищаем память.
        It.Iterator.clear();
        m_OutIterators.push_front(It);
    }
}

//------------------------------------------------------------------------------
//! Конструктор.

TDirEnumerator::TDirEnumerator()
    : m_Started(false),
      m_RelNameWithRoot(false),
      m_RelPathValid(false)
{
}

//------------------------------------------------------------------------------
//! Деструктор.

TDirEnumerator::~TDirEnumerator()
{
    finish();
}

//------------------------------------------------------------------------------
//! Запуск перечисления с указанными параметрами.

bool TDirEnumerator::start(const TDirEnumerator::TParams *pParams)
{
    if (pParams) {
        m_Params = *pParams;
        return start();
    }

    return false;
}

//------------------------------------------------------------------------------
//! Запуск перечисления с указанными параметрами.
/*!
   \overload
 */

bool TDirEnumerator::start(const TDirEnumerator::TParams& Params)
{
    m_Params = Params;
    return start();
}

//------------------------------------------------------------------------------
//! Запуск перечисления с указанными параметрами.
/*!
   \overload
 */

bool TDirEnumerator::start(const QString& StartPath,
                           const TDirEnumerator::TFilters Filter,
                           const TFileStatOptions Options,
                           const int SubdirsDepth)
{
    m_Params.startPath      = StartPath;
    m_Params.filter         = Filter;
    m_Params.dirStatOptions = Options;
    m_Params.subdirsDepth   = SubdirsDepth;

    return start();
}

//------------------------------------------------------------------------------
//! Получение следующего элемента.

bool TDirEnumerator::next()
{
    // Перед поиском следующего элемента чистим списки итераторов.
    m_InIterators.clear();
    m_OutIterators.clear();

    while (!m_Iterators.isEmpty() || !m_InIterators.isEmpty())
    {
        TDirIterator* pI = NULL;
        if (!m_InIterators.isEmpty())
            pI = m_InIterators.top().Iterator.data();
        else
            pI = m_Iterators.top().Iterator.data();
        Q_ASSERT(pI != NULL);

        if (pI->next())
        {
            m_FileInfoEx = pI->info();

            if (analyze() == arOK) {
                // Если элемент нам подходит, выходим.
                break;
            }
        }
        else {
            // Следующего элемента нет. Завершаем обработку каталога...
            endDir();
            // ... и уходим на следующую итерацию.
        }
    }

    // Обрабатываем списки итераторов.
    if (!m_InIterators.isEmpty()) {
        m_Iterators += m_InIterators;
        m_RelPathValid = false;
    }

    // Если список итераторов непуст, мы что-то нашли. Если пуст, мы вышли
    // из всех каталогов.
    return !m_Iterators.isEmpty();
}

//------------------------------------------------------------------------------
//! Завершение процесса перечисления.

void TDirEnumerator::finish()
{
    m_Iterators.clear();
    m_InIterators.clear();
    m_OutIterators.clear();
    m_FileInfoEx.clear();
    m_Started = false;
}

//------------------------------------------------------------------------------
//! Полное имя элемента (включает путь).

QString TDirEnumerator::name() const
{
    return m_FileInfoEx.name();
}

//------------------------------------------------------------------------------
//! Относительный путь элемента.
/*!
   Метод возвращает относительный путь текущего элемента перечисления. Путь
   строится относительно стартового каталога. Имя элемента в путь не
   включается.

   \arg WithRoot Если true, то в относительный путь добавляется имя корневого
     каталога, если false - не добавляется. Подробнее см. в описании метода
     \c setRelNameWithRoot.

   \sa relPath, startDirPath, name, relName, relNameWithRoot
 */

QString TDirEnumerator::relPath(bool WithRoot) const
{
    QString Result;
    if (!m_Iterators.isEmpty()) {
        if (WithRoot)
            Result = m_Iterators[0].Name;
        for (int i = 1, c = m_Iterators.count(); i < c; ++i)
            AddWithSeparator(&Result, m_Iterators[i].Name);
    }
    return Result;
}

//------------------------------------------------------------------------------
//! Относительный путь элемента.
/*!
   Метод возвращает относительный путь текущего элемента перечисления. Путь
   строится относительно стартового каталога. Имя элемента в путь не
   включается.

   \sa relPath(bool), startDirPath, relName, relNameWithRoot
 */

QString TDirEnumerator::relPath() const
{
    if (!m_RelPathValid) {
        m_RelPath = relPath(m_RelNameWithRoot);
        m_RelPathValid = true;
    }
    return m_RelPath;
}

//------------------------------------------------------------------------------
//! Относительный путь и имя элемента.
/*!
   Метод возвращает относительный путь с именем текущего элемента перечисления.
   Путь строится относительно стартового каталога.

   \arg WithRoot Если true, то в относительный путь добавляется имя корневого
     каталога, если false - не добавляется. Подробнее см. в описании метода
     \c setRelNameWithRoot.

   \sa relName, startDirPath, relPath(bool)
 */

QString TDirEnumerator::relName(bool WithRoot) const
{
    QString Path = relPath(WithRoot);
    if (m_FileInfoEx.isFile())
    {
        if (!Path.isEmpty() && !Path.endsWith(QDir::separator()))
            Path += QDir::separator();
        Path += m_FileInfoEx.fileName();
    }
    return Path;
}

//------------------------------------------------------------------------------
//! Относительный путь и имя элемента.
/*!
   Метод возвращает относительный путь с именем текущего элемента перечисления.
   Путь строится относительно стартового каталога.

   \sa startDirPath, relPath, relNameWithRoot
 */

QString TDirEnumerator::relName() const
{
    return relName(m_RelNameWithRoot);
}

//------------------------------------------------------------------------------
//! Относительное имя каталога для текущего элемента.
/*!
   \remarks При неверном индексе i возвращает пустую строку.

   \sa subdirCount, relName, relPath, relNameWithRoot
 */

QString TDirEnumerator::subdirRelName(int i) const
{
    if ((0 <= i) && (i < m_Iterators.count()))
        return m_Iterators[i].RelName;
    else
        return QString();
}

//------------------------------------------------------------------------------
//! Относительный путь и имя "каталога входа".
/*!
   \remarks При неверном индексе i возвращает пустую строку.

   \sa subdirInCount, subdirsInPath, relNameWithRoot
 */

QString TDirEnumerator::subdirInRelName(int i) const
{
    if ((0 <= i) && (i < m_InIterators.count()))
        return m_InIterators[i].RelName;
    else
        return QString();
}

//------------------------------------------------------------------------------
//! Относительный путь и имя "каталога выхода".
/*!
   \remarks При неверном индексе i возвращает пустую строку.

   \sa subdirOutCount, subdirsOutPath, relNameWithRoot
 */

QString TDirEnumerator::subdirOutRelName(int i) const
{
    if ((0 <= i) && (i < m_OutIterators.count()))
        return m_OutIterators[i].RelName;
    else
        return QString();
}

//------------------------------------------------------------------------------
//! Полное имя "каталога входа".
/*!
   Метод возвращает имя "каталога входа", построенное относительно каталога,
   который был текущим на предыдущей итерации перечисления. Имя строится из
   имён всех "каталогов входа", разделённых символом-разделителем каталогов.
 */

QString TDirEnumerator::subdirsInPath() const
{
    QString Result;
    for (int i = 0; i < m_InIterators.count(); ++i)
        AddWithSeparator(&Result, m_InIterators[i].Name);
    return Result;
}

//------------------------------------------------------------------------------
//! Полное имя "каталога выхода".
/*!
   Метод возвращает имя "каталога выхода", построенное относительно каталога,
   который был текущим на предыдущей итерации перечисления. Имя строится из
   имён всех "каталогов выхода", разделённых символом-разделителем каталогов.
 */

QString TDirEnumerator::subdirsOutPath() const
{
    QString Result;
    for (int i = 0; i < m_OutIterators.count(); ++i)
        AddWithSeparator(&Result, m_OutIterators[i].Name);
    return Result;
}

//------------------------------------------------------------------------------
//! Информация о каталоге с индексом i.
/*!
   \remarks Если индекс неверный, будет возвращена пустая информация.

   \sa subdirCount, subdirInfoPtr
 */

TFileInfoEx TDirEnumerator::subdirInfo(int i) const
{
    if ((0 <= i) && (i < m_Iterators.count()))
        return m_Iterators[i].Info;
    else
        return TFileInfoEx();
}

//------------------------------------------------------------------------------
//! Информация о "каталоге входа" с индексом i.
/*!
   \remarks Если индекс неверный, будет возвращена пустая информация.

   \sa subdirInCount, subdirInInfoPtr
 */

TFileInfoEx TDirEnumerator::subdirInInfo(int i) const
{
    if ((0 <= i) && (i < m_InIterators.count()))
        return m_InIterators[i].Info;
    else
        return TFileInfoEx();
}

//------------------------------------------------------------------------------
//! Информация о "каталоге выхода" с индексом i.
/*!
   \remarks Если индекс неверный, будет возвращена пустая информация.

   \sa subdirOutCount, subdirOutInfoPtr
 */

TFileInfoEx TDirEnumerator::subdirOutInfo(int i) const
{
    if ((0 <= i) && (i < m_OutIterators.count()))
        return m_OutIterators[i].Info;
    else
        return TFileInfoEx();
}

//------------------------------------------------------------------------------
//! Статистика каталога с индексом i.
/*!
   \remarks Если индекс неверный, будет возвращена пустая статистика.

   \sa subdirCount, subdirStatPtr
 */

TFileStat TDirEnumerator::subdirStat(int i) const
{
    if ((0 <= i) && (i < m_Iterators.count()))
        return m_Iterators[i].Info.stat(m_Params.dirStatOptions);
    else
        return TFileStat();
}

//------------------------------------------------------------------------------
//! Статистика "каталога входа" с индексом i.
/*!
   \remarks Если индекс неверный, будет возвращена пустая статистика.

   \sa subdirInCount, subdirInStatPtr
 */

TFileStat TDirEnumerator::subdirInStat(int i) const
{
    if ((0 <= i) && (i < m_InIterators.count()))
        return m_InIterators[i].Info.stat(m_Params.dirStatOptions);
    else
        return TFileStat();
}

//------------------------------------------------------------------------------
//! Статистика "каталога выхода" с индексом i.
/*!
   \remarks Если индекс неверный, будет возвращена пустая статистика.

   \sa subdirOutCount, subdirOutStatPtr
 */

TFileStat TDirEnumerator::subdirOutStat(int i) const
{
    if ((0 <= i) && (i < m_OutIterators.count()))
        return m_OutIterators[i].Info.stat(m_Params.dirStatOptions);
    else
        return TFileStat();
}

//------------------------------------------------------------------------------
//! Список имён каталогов.

QStringList TDirEnumerator::subdirPaths() const
{
    QStringList Result;

    for (int i = 0; i < m_Iterators.count(); ++i)
        Result.append(m_Iterators[i].Name);
    return Result;
}

//------------------------------------------------------------------------------
//! Список имён "каталогов входа".

QStringList TDirEnumerator::subdirInPaths() const
{
    QStringList Result;
    for (int i = 0; i < m_InIterators.count(); ++i)
        Result.append(m_InIterators[i].Name);
    return Result;
}

//------------------------------------------------------------------------------
//! Список имён "каталогов выхода".

QStringList TDirEnumerator::subdirOutPaths() const
{
    QStringList Result;
    for (int i = 0; i < m_OutIterators.count(); ++i)
        Result.append(m_OutIterators[i].Name);
    return Result;
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если перечисление было запущено.
/*!
   Метод вернёт true, если перечисление элементов было успешно запущено, т.е.
   успешно выполнен любой из вариантов метода \c start. Если перечисление
   завершено (больше нет элементов), метод всё равно вернёт true. Чтобы метод
   возвращал false, вызовите \c finished.

   \sa isFinished, start. finish
 */

bool TDirEnumerator::isStarted() const
{
    return m_Started;
}

//------------------------------------------------------------------------------
//! Параметры перечисления.
/*!
   \sa TParams, start(TParams)
 */

TDirEnumerator::TParams TDirEnumerator::params() const
{
    return m_Params;
}

//------------------------------------------------------------------------------
//! Путь к стартовому каталогу.
/*!
   \sa TParams, start(TParams)
 */

QString TDirEnumerator::startDirPath() const
{
    return m_Params.startPath;
}

//------------------------------------------------------------------------------
//! Глубина обхода подкаталогов (-1 - неограниченно).
/*!
   \sa TParams, start(TParams)
 */

int TDirEnumerator::subdirsDepth() const
{
    return m_Params.subdirsDepth;
}

//------------------------------------------------------------------------------
//! Фильтры перечисления.
/*!
   \sa TParams, start(TParams)
 */

TDirEnumerator::TFilters TDirEnumerator::filter() const
{
    return m_Params.filter;
}

//------------------------------------------------------------------------------
//! Получаемая информация о подкаталогах.
/*!
   \sa TParams, start(TParams)
 */

TFileStatOptions TDirEnumerator::dirStatOptions() const
{
    return m_Params.dirStatOptions;
}

//------------------------------------------------------------------------------
//! Информация о текущем элементе.
/*!
   \sa infoPtr
 */

TFileInfoEx TDirEnumerator::info() const
{
    return m_FileInfoEx;
}

//------------------------------------------------------------------------------
//! Указатель на информацию о текущем элементе.
/*!
   \sa info
 */

const TFileInfoEx* TDirEnumerator::infoPtr() const
{
    return &m_FileInfoEx;
}

//------------------------------------------------------------------------------
//! Число каталогов в пути к данному элементу (включая корневой).

int TDirEnumerator::subdirCount() const
{
    return m_Iterators.count();
}
//------------------------------------------------------------------------------
//! Число "новых" каталогов.
/*!
   Метод возвращает число каталогов, в которые был произведён вход при
   получении текущего элемента.
 */

int TDirEnumerator::subdirInCount() const
{
    return m_InIterators.count();
}

//------------------------------------------------------------------------------
//! Число "старых" каталогов.
/*!
   Метод возвращает число каталогов, из которых был произведён выход при
   получении текущего элемента.
 */

int TDirEnumerator::subdirOutCount() const
{
    return m_OutIterators.count();
}

//------------------------------------------------------------------------------
//! Флаг добавления в относительный путь имени корневого каталога.
/*!
   \sa setRelNameWithRoot
 */

bool TDirEnumerator::relNameWithRoot() const
{
    return m_RelNameWithRoot;
}

//------------------------------------------------------------------------------
//! Установка флага добавления в относительный путь имени корневого каталога.
/*!
   Если этот параметр равен false, методы \c relPath, \c relName,
   \c subdirInRelName, \c subdirOutRelName вернут относительный путь
   до текущего элемента, если true - методы добавят в путь имя начального
   каталога. Например, если перечислитель запущен со стартовым каталогом
   C:/Path/Dir/, а сейчас мы находимся в его подкаталоге Dir1/Dir2, то
   при значении этого флага true relPath вернёт Dir/Dir1/Dir2, а при значении
   false - Dir1/Dir2.
*/

void TDirEnumerator::setRelNameWithRoot(bool Flag)
{
    if (m_RelNameWithRoot != Flag) {
        m_RelNameWithRoot = Flag;
        m_RelPathValid = false;
    }
}

//------------------------------------------------------------------------------
