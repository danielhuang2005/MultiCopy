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

#include "FileInfoEx.hpp"

//------------------------------------------------------------------------------

#include <QFlags>
#include <QDir>

#if defined(Q_OS_WIN)
    #include "Functions_win.hpp"
    #include <windows.h>
#else

#endif

#include "FileInfoEx_p.hpp"

#include "Core/Common/CommonFn.hpp"

//------------------------------------------------------------------------------
//! Установка параметров объекта файловой системы.
/*!
   Функция устанавливает параметры объекта с именем FileName, перечисленные в
   поле Options структуры pFileStat. Возвращает перечень успешно установленных
   параметров.
 */

TFileStatOptions SetFileStat(const QString& FileName, const TFileStat* pFileStat)
{
    Q_ASSERT(pFileStat != NULL);

    TFileStatOptions Result;

    #ifdef Q_OS_WIN
        QString wFileName = PathToLongWinPath(FileName);
        if (pFileStat->Options.testFlag(fsoTime))
        {
            HANDLE hFile = CreateFileW((LPCWSTR)wFileName.utf16(),
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_FLAG_BACKUP_SEMANTICS,
                                       NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                qWarning("SetFileStat. Cannot open object \"%s\" for writing: %s",
                         qPrintable(FileName),
                         qPrintable(GetSystemErrorString()));
            }
            else {
                if (!SetFileTime(hFile,
                                 &pFileStat->CreationTime,
                                 &pFileStat->LastAccessTime,
                                 &pFileStat->LastWriteTime))
                {
                    qWarning("SetFileStat. Cannot set date/time for object \"%s\": %s",
                             qPrintable(FileName),
                             qPrintable(GetSystemErrorString()));
                }
                else {
                    Result |= fsoTime;
                }
                if (!CloseHandle(hFile)) {
                    qWarning("SetFileStat. Error close handle for object \"%s\": %s",
                             qPrintable(FileName),
                             qPrintable(GetSystemErrorString()));
                }
            }
        }

        if (pFileStat->Options.testFlag(fsoAttr))
        {
            if (SetFileAttributesW((LPCWSTR)wFileName.utf16(), pFileStat->Attr) != 0) {
                Result |= fsoAttr;
            }
            else {
                qWarning("SetFileStat. Cannot set attributes for object \"%s\": %s",
                         qPrintable(FileName),
                         qPrintable(GetSystemErrorString()));
            }
        }
    #else
        QByteArray File = FileName.toLocal8Bit();
        if (pFileStat->Options.testFlag(fsoTime))
        {
            utimbuf Utimbuf;
            Utimbuf.actime  = pFileStat->AccessTime;
            Utimbuf.modtime = pFileStat->ModificationTime;
            if (utime(File.data(), &Utimbuf) != 0) {
                qWarning("SetFileStat. Cannot set time for object \"%s\": %s",
                         File.data(), qPrintable(GetSystemErrorString()));
            }
            else {
                Result |= fsoTime;
            }
        }


        if (pFileStat->Options.testFlag(fsoAttr))
        {
            if (chmod(File.data(), pFileStat->Mode) != 0) {
                qWarning("SetFileStat. Cannot set permissions for file \"%s\": %s",
                         File.data(), qPrintable(GetSystemErrorString()));
            }
            else {
                Result |= fsoAttr;
            }
        }
    #endif

    return Result;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                          T F i l e S t a t
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Конструктор.

TFileStat::TFileStat()
{
    clear();
}

//------------------------------------------------------------------------------
//! Очистка.

void TFileStat::clear()
{
    #ifdef Q_OS_WIN
        CreationTime.dwLowDateTime    = 0;
        CreationTime.dwHighDateTime   = 0;
        LastAccessTime.dwLowDateTime  = 0;
        LastAccessTime.dwHighDateTime = 0;
        LastWriteTime.dwLowDateTime   = 0;
        LastWriteTime.dwHighDateTime  = 0;
        Attr = 0;
    #else
        AccessTime       = 0;
        ModificationTime = 0;
        Mode             = 0;
    #endif
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                   T F i l e I n f o E x D a t a
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifdef Q_OS_WIN

//! Установка информации из структуры WIN32_FIND_DATAW.
/*!
   \remarks Имя не изменяется!
 */

void TFileInfoExData::set(const WIN32_FIND_DATAW* _FindData)
{
    if (_FindData != NULL)
        FindData = *_FindData;

    TypeFlags = ffExists;
    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        TypeFlags |= ffDir;
    else
        TypeFlags |= ffFile;

    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        TypeFlags |= ffHidden;

    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        TypeFlags |= ffSystem;

    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        TypeFlags |= ffReparsePoint;
        if (FindData.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT)
            TypeFlags |= ffMountPoint | ffLink;
        else if (FindData.dwReserved0 == IO_REPARSE_TAG_SYMLINK)
            TypeFlags |= ffSymLink | ffLink;
        AnalyzedFlags |= faLinkAnalyzed;
    }
}

//------------------------------------------------------------------------------
//! Установка имени _Name и информации из структуры WIN32_FIND_DATAW.

void TFileInfoExData::set(const QString& _Name, const WIN32_FIND_DATAW* _FindData)
{
    clear();
    prepareName(_Name);
    set(_FindData);
}

//------------------------------------------------------------------------------
//! Инициализация.

void TFileInfoExData::init(const QString& _DirName, const WIN32_FIND_DATAW* _FindData)
{
    clear();
    const WCHAR* cFileName = _FindData ? _FindData->cFileName : FindData.cFileName;
    set(AddWithSeparator(_DirName, QString::fromWCharArray(cFileName)), _FindData);
}

#else

//! Установка информации из структуры stat64.

void TFileInfoExData::set(const struct stat64* _Stat)
{
    if (_Stat != NULL)
        Stat = *_Stat;

    TypeFlags = ffExists;

    switch (Stat.st_mode & S_IFMT)
    {
        case S_IFIFO :
            TypeFlags |= ffNamedPipe | ffSystem;
            break;
        case S_IFCHR :
            TypeFlags |= ffChrDev | ffSystem;
            break;
        case S_IFDIR :
            TypeFlags |= ffDir;
            break;
        case S_IFBLK  :
            TypeFlags |= ffBlockDev | ffSystem;
            break;
        case S_IFREG :
            TypeFlags |= ffFile;
            break;
        case S_IFLNK :
            TypeFlags |= ffLink;
            break;
        case S_IFSOCK :
            TypeFlags |= ffSocket | ffSystem;
            break;
        default :
            qWarning("TFileInfoExData::set. Unknown type (0x%x) of object \"%s\".",
                     Stat.st_mode & S_IFMT, NativeName.data());
    }

    int i = Name.lastIndexOf(QDir::separator()) + 1;
    QStringRef FileName = Name.midRef(i);
    if (!FileName.isEmpty()) {
        if (FileName.length() >= 2 && FileName[0] == '.' && FileName[1] != '.')
            TypeFlags |= ffHidden;
    }
    else {
        qWarning("TFileInfoExData::set. Empty object name for \"%s\"",
                 qPrintable(Name));
    }
}

#endif

//------------------------------------------------------------------------------
//! Конструктор.

TFileInfoExData::TFileInfoExData()
{
    clear();
}

//------------------------------------------------------------------------------
//! Очистка.

void TFileInfoExData::clear()
{
    Name.clear();
    LinkTarget.clear();
    ResolvedName.clear();
    TypeFlags = 0;
    AnalyzedFlags = 0;
    #if defined(Q_OS_WIN)
        LongWinName.clear();
        ZeroMemory(&FindData, sizeof(FindData));
    #else
        NativeName.clear();
        memset(&Stat, 0, sizeof(Stat));
    #endif
}

//------------------------------------------------------------------------------
//! Подготовка имени.

void TFileInfoExData::prepareName(const QString& _Name)
{
    Name = QDir::toNativeSeparators(_Name);
    while (Name.endsWith(QDir::separator()) && Name.length() > 1)
        Name.chop(1);

    #ifdef Q_OS_WIN
        if (Name.endsWith(':'))
            LongWinName = Name;
        else {
            LongWinName = PathToLongWinPath(Name);
        }
        Q_ASSERT(!LongWinName.endsWith('\\'));
    #else
        NativeName = Name.toLocal8Bit();
    #endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//                      T F i l e I n f o E x
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! Получение информации об объекте с именем Name.

void TFileInfoEx::getInfo(const QString& Name)
{
    m_Data->clear();
    if (Name.isEmpty())
        return;

    m_Data->prepareName(Name);

    #if defined(Q_OS_WIN)
        if (isNetworkPath(m_Data->Name))
        {
            m_Data->TypeFlags |= ffNetwork;
            QStringList Path = m_Data->Name.split('\\', QString::SkipEmptyParts);
            // Path[0] - имя сервера, Path[1] - имя папки, далее - элементы пути.

            if (Path.count() < 2) {
                // Имя сервера без папки. Ошибка.
                m_Data->AnalyzedFlags = faMask;
            }
            else if (Path.count() == 2) {
                // Корень сетевой папки. О ней почти ничего узнать не удастся.
                m_Data->TypeFlags |= ffNetShare;
                if (isNetworkShareExists(Path[0], Path[1]))
                {
                    // Для совместимости считаем корень сетевой папки каталогом.
                    m_Data->TypeFlags |= ffExists | ffDir;
                }
            }
            else {
                // Path.count() >= 3
                if (getFileFindData(m_Data->LongWinName, &m_Data->FindData))
                    m_Data->set();
                else
                    m_Data->AnalyzedFlags |= faMask;
            }
        }
        else if (m_Data->Name.endsWith(':'))
        {
            // Корневой каталог диска.
            WIN32_FILE_ATTRIBUTE_DATA Data;
            if (getFileAttributesData(m_Data->LongWinName, &Data))
            {
                // Пользуемся тем, что начало структуры WIN32_FIND_DATA
                // совпадает со структурой WIN32_FILE_ATTRIBUTE_DATA
                memcpy(&m_Data->FindData, &Data, sizeof(Data));
                m_Data->set();
            }
            m_Data->AnalyzedFlags |= faMask;
        }
        else {
            // Не корневой каталог. Можно использовать getFileFindData.
            if (getFileFindData(m_Data->LongWinName, &m_Data->FindData))
                m_Data->set();
            else
                m_Data->AnalyzedFlags = faMask;
        }
    #else
        if (::stat64(m_Data->NativeName.data(), &m_Data->Stat) == 0) {
            m_Data->set();
        }
        else {
            m_Data->AnalyzedFlags = faMask;
            qWarning("TFileInfoEx::getInfo. stat64 error on object \"%s\": %s",
                     m_Data->NativeName.data(), qPrintable(GetSystemErrorString()));
        }
    #endif
}

//------------------------------------------------------------------------------
//! Разыменование ссылки.

void TFileInfoEx::resolveLink() const
{
    if (!m_Data->AnalyzedFlags.testFlag(faLinkTarget) ||
        !m_Data->AnalyzedFlags.testFlag(faLinkAnalyzed))
    {
        #if defined(Q_OS_WIN)
            if (m_Data->FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                // Точка повтороной обработки.
                Q_ASSERT(m_Data->AnalyzedFlags.testFlag(faLinkAnalyzed));
                m_Data->LinkTarget = resolveReparsePoint(m_Data->LongWinName);
            }
            else if (m_Data->Name.endsWith(".lnk", Qt::CaseInsensitive))
            {
                // Ярлык Windows.
                Q_ASSERT(!m_Data->AnalyzedFlags.testFlag(faLinkTarget));
                m_Data->LinkTarget = resolveLnkFile(m_Data->LongWinName);
                if (!m_Data->LinkTarget.isEmpty())
                    m_Data->TypeFlags |= ffLink | ffLnkFile;
            }
        #else
            // TODO : Реализовать для Linux.
        #endif
        m_Data->AnalyzedFlags |= faLinkAnalyzed | faLinkTarget;
    }
}

//------------------------------------------------------------------------------
//! Полное разыменование ссылки.

void TFileInfoEx::fullResolveLink() const
{
    if (!m_Data->AnalyzedFlags.testFlag(faLinkResolved)) {
        QStringList Names;
        TFileInfoEx Info = *this;
        Names.append(name());
        while (Info.isLink()) {
            QString Target = Info.linkTarget();
            if (Names.contains(Target)) {
                m_Data->ResolvedName.clear();
                m_Data->AnalyzedFlags |= faLinkCyclic;
                m_Data->TypeFlags     |= ffCyclicLink;
                break;
            }
            else {
                m_Data->ResolvedName = Target;
                Info.setName(Target);
            }
        }
        m_Data->AnalyzedFlags |= faLinkResolved;
    }
}

//------------------------------------------------------------------------------
//! Анализ на циклическую ссылку.

void TFileInfoEx::analyzeCyclicLink() const
{
    if (!m_Data->AnalyzedFlags.testFlag(faLinkCyclic)) {
        if (isLink()) {
            QString Target = fullyResolvedLink();
            if (Target.isEmpty()) {
                m_Data->AnalyzedFlags |= faLinkCyclic;
            }
            else {
                TFileInfoEx Info(Target);
                if (Info.isDir()) {
                    if (!Target.endsWith(QDir::separator()))
                        Target += QDir::separator();
                    if (m_Data->Name.startsWith(Target, Qt::CaseInsensitive))
                        m_Data->TypeFlags |= ffCyclicLink;
                }
            }
        }
        m_Data->AnalyzedFlags |= faLinkCyclic;
    }
}

//------------------------------------------------------------------------------
//! Конструктор.

TFileInfoEx::TFileInfoEx()
    : m_Data(new TFileInfoExData())
{
}

//------------------------------------------------------------------------------
//! Копирующий конструктор.

TFileInfoEx::TFileInfoEx(const TFileInfoEx& other)
    : m_Data(other.m_Data)
{
}

//------------------------------------------------------------------------------
//! Конструктор.
/*!
   Конструирует экземпляр класса, указывающий на объект файловой системы с
   именем Name.
 */

TFileInfoEx::TFileInfoEx(const QString& Name)
    : m_Data(new TFileInfoExData())
{
    getInfo(Name);
}

//------------------------------------------------------------------------------
//! Оператор присваивания.

TFileInfoEx& TFileInfoEx::operator=(const TFileInfoEx& other)
{
    m_Data = other.m_Data;
    return *this;
}

//------------------------------------------------------------------------------
//! Деструктор.

TFileInfoEx::~TFileInfoEx()
{
}

//------------------------------------------------------------------------------
//! Имя объекта.
/*!
   \sa setName
 */

QString TFileInfoEx::name() const
{
    return m_Data->Name;
}

//------------------------------------------------------------------------------
//! Краткое имя объекта.
/*!
   Метод возвращает имя объекта в содержащем его каталоге (имя без пути).

   \sa name
 */

QString TFileInfoEx::fileName() const
{
    int i = m_Data->Name.lastIndexOf(QDir::separator());
    if (i >= 0)
        return m_Data->Name.mid(i + 1);

    return m_Data->Name;
}

//------------------------------------------------------------------------------
//! Установка имени объекта.
/*!
   \sa name
 */

void TFileInfoEx::setName(const QString& Name)
{
    if (m_Data->Name != Name)
        getInfo(Name);
}

//------------------------------------------------------------------------------
//! Обновление информации об объекте.
/*!
   Метод заново запрашивает у файловой системе информацию об объекте.
 */

void TFileInfoEx::refresh()
{
    getInfo(m_Data->Name);
}

//------------------------------------------------------------------------------
//! Очистка.

void TFileInfoEx::clear()
{
    m_Data->clear();
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект существует.
/*!
   \remarks Более точно: метод возвращает true, если объект _существовал_ в
     момент создания экземпляра класса или установки нового имени объекта
     методом \c setName.
 */

bool TFileInfoEx::exists() const
{
    return m_Data->TypeFlags.testFlag(ffExists);
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является файлом.

bool TFileInfoEx::isFile() const
{
    return m_Data->TypeFlags.testFlag(ffFile);
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является каталогом.

bool TFileInfoEx::isDir() const
{
    return m_Data->TypeFlags.testFlag(ffDir);
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является скрытым.

bool TFileInfoEx::isHidden() const
{
    return m_Data->TypeFlags.testFlag(ffHidden);
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является системным.
/*!
   \remarks Для Windows проверяется наличие флага System.
 */

bool TFileInfoEx::isSystem() const
{
    return m_Data->TypeFlags.testFlag(ffSystem);
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является ссылкой.
/*!
   \remarks Для Windows анализируются lnk-файлы и точки повторной обработки
     (Reparse Point).

   \sa isCyclicLink
 */

bool TFileInfoEx::isLink() const
{
    if (!m_Data->AnalyzedFlags.testFlag(faLinkAnalyzed))
        resolveLink();
    return m_Data->TypeFlags.testFlag(ffLink);
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является ярлыком Windows.
/*!
   Метод вернёт true только в случае, когда объект является ярлыком Windows
   (файлом с расширением lnk) и этот ярлык действителен, т.е. удаётся определить
   объект, на который он ссылается. Однако, объект-назначение может и не
   существовать.

   \remarks В операционных системах, отличных от Windows, всегда вернёт false.
 */

bool TFileInfoEx::isShortcut() const
{
    #ifdef Q_OS_WIN
        return isLink() && // Должно быть первым, иначе не будет флага ffLnkFile!
               m_Data->TypeFlags.testFlag(ffLnkFile);
    #else
        return false;
    #endif
}

//------------------------------------------------------------------------------
//! Метод возвращает true, если объект является циклической ссылкой.
/*!
   \remarks Ссылка называется циклической, если процесс её разыменования в итоге
     приводит либо к исходному объекту, либо к каталогу, в котором он лежит.
     Во избежание зацикливания при просмотре каталогов обрабатывать такие
     объекты необходимо очень аккуратно.

   \sa isLink
 */

bool TFileInfoEx::isCyclicLink() const
{
    analyzeCyclicLink();
    return m_Data->TypeFlags.testFlag(ffCyclicLink);
}

//------------------------------------------------------------------------------
//! Размер объекта (байт).
/*!
   \remarks Для несуществующих объектов размер всегда ноль.
   \remarks Для каталогов размер всегда ноль.
 */

qint64 TFileInfoEx::size() const
{
    #if defined Q_OS_WIN
        ULARGE_INTEGER uLI;
        uLI.u.LowPart  = m_Data->FindData.nFileSizeLow;
        uLI.u.HighPart = m_Data->FindData.nFileSizeHigh;
        return uLI.QuadPart;
    #else
        return m_Data->Stat.st_size;
    #endif
}

//------------------------------------------------------------------------------
//! Параметры объекта.
/*!
   Метод возвращает параметры текущего объекта. Поле Options результирующей
   структуры типа TFileStat будет содержать флаги параметров, которые удалось
   получить.

   \arg What Флаги параметров, которые необходимо получить.
 */

TFileStat TFileInfoEx::stat(TFileStatOptions What) const
{
    TFileStat Stat;

    #if defined Q_OS_WIN
        Stat.Options = What;
        if (What.testFlag(fsoTime)) {
            Stat.CreationTime   = m_Data->FindData.ftCreationTime;
            Stat.LastAccessTime = m_Data->FindData.ftLastAccessTime;
            Stat.LastWriteTime  = m_Data->FindData.ftLastWriteTime;
        }
        if (What.testFlag(fsoAttr)) {
            Stat.Attr = m_Data->FindData.dwFileAttributes;
        }
    #else
        Stat.Options = What;
        if (What.testFlag(fsoTime)) {
            Stat.AccessTime       = m_Data->Stat.st_atime;
            Stat.ModificationTime = m_Data->Stat.st_mtime;
        }
        if (What.testFlag(fsoAttr)) {
            Stat.Mode = m_Data->Stat.st_mode;
        }
    #endif

    return Stat;
}

//------------------------------------------------------------------------------
//! Установка параметров для другого объекта.
/*!
   Метод устанавливает для объекта с именем Target статистику данного объекта.
   В параметре What передаются флаги, указывающие, какие именно параметры
   необходимо установить. Возвращает флаги успешно установленных параметров.
 */

TFileStatOptions TFileInfoEx::copyStatTo(const QString& Target,
                                         TFileStatOptions What) const
{
    TFileStat Stat = stat();
    Stat.Options = What;
    return SetFileStat(Target, &Stat);
}

//------------------------------------------------------------------------------
//! Установка параметров для текущего объекта.
/*!
   Метод устанавливает для текущего объекта параметров, переданные в структуре
   Stat. Поле Options структуры указывает, какие именно параметры необходимо
   установить. Возвращает флаги успешно установленных параметров.

   \remarks Если объект не существует, ничего не делает.

   \remarks Повторно запрашивает у файловой системы информацию об объекте после
     изменения параметров.
 */

TFileStatOptions TFileInfoEx::setStat(const TFileStat& Stat)
{
    if (exists() && Stat.Options != 0) {
        TFileStatOptions Result = SetFileStat(m_Data->Name, &Stat);
        refresh();
        return Result;
    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
//! Объект, на который указывает ссылка.
/*!
   Метод возвращает имя объекта, на который указывает ссылка.

   \remarks Если объект не является ссылкой, возвращается пустая строка.
   \remarks Объект с возвращённым именем также может оказаться ссылкой.

   \sa isLink, fullyResolvedLink.
 */

QString TFileInfoEx::linkTarget() const
{
    resolveLink();
    return m_Data->LinkTarget;
}

//------------------------------------------------------------------------------
//! Полностью разыменованное имя.
/*!
   Метод возвращает полностью разыменованное имя объекта, т.е. разыменовывает
   все ссылки, доходя до элемента, не являющегося ссылкой. Если ссылка
   циклическая, вернёт пустую строку.

   \remarks Если объект не является ссылкой, возвращается пустая строка.
   \remarks Объект с возвращённым именем может не существовать.

   \sa isLink, isCyclicLink, linkTarget.
 */

QString TFileInfoEx::fullyResolvedLink() const
{
    fullResolveLink();
    return m_Data->ResolvedName;
}

//------------------------------------------------------------------------------
//! Проверка существования объекта с именем Name.

bool TFileInfoEx::exists(const QString& Name)
{
    return TFileInfoEx(Name).exists();
}

//------------------------------------------------------------------------------
//! Установка статистической информации.
/*!
   Метод устанавливает статистическую информацию, содержащуюся в структуре Stat
   для объекта с именем Name. Поле Options структуры указывает, какую именно
   статистику необходимо установить. Возвращает перечень успешно установленных
   параметров.
 */

TFileStatOptions TFileInfoEx::setStat(const QString& Name, const TFileStat& Stat)
{
    return SetFileStat(Name, &Stat);
}

//------------------------------------------------------------------------------
