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

#ifndef __DIRENUMERATOR__HPP__
#define __DIRENUMERATOR__HPP__

//------------------------------------------------------------------------------

#include <QStack>
#include <QSharedPointer>

#include "Core/FastIO/FileInfoEx.hpp"
#include "Core/FastIO/DirIterator.hpp"

//------------------------------------------------------------------------------
//! Класс перечислителя элементов каталога.
/*!
   Класс является аналогом класса QDirIterator, но содержит дополнительную
   функциональность. Принцип использования класса следующий:

   \code
     TDirEnumerator DE;
     TDirEnumerator::TParams P;
     // Установка свойств.
     if (DE.start(P)) {
         do {
             // Обработка полученного элемента.
         } while(DE.next());
     }
     DE.finish();
   \endcode

   Одно из основных отличий данного класса от QDirIterator в возможности
   перечисления файлов в подкаталогах указанного каталога с возможностью
   указания глубины входа в подкаталоги. За эту возможность отвечает член
   \c subdirsDepth структуры \c TParams. В случае установки его в -1
   глубина входа в подкаталоги будет неограниченна, в 0 - будут перечислены
   только файлы в текущем каталоге, 1 - в текущем и во всех дочерних и т.д.
   Поскольку часто нужно знать, в какие каталоги был произведён вход, а из
   каких мы уже вышли, класс предоставляет такую информацию. Для этого
   предназначены методы с префиксами subdir, subdirIn, subdirOut. Методы
   с префиксом subdir предоставляют информацию о текущем дереве каталогов,
   методы с префиксом subdirIn - о каталогах, в которые был произведён вход при
   получении текущего элемента, а методы с префиксом subdirOut - о каталогах,
   из которых при получении текущего элемента был произведён выход. Если
   какая-либо ветвь файловой системы не содержит искомых элементов, то в списках
   каталогов информации об этой ветви не будет.
 */

class TDirEnumerator
{
    public:
        //! Флаги, управляющие перечислением элементов.
        enum TFilter {
            Files           = 0x0001,  //!< Файлы.
            Dirs            = 0x0002,  //!< Каталоги.
            Hidden          = 0x0004,  //!< Скрытые элементы.
            System          = 0x0008,  //!< Системные элементы.
            FollowShortcuts = 0x0010   //!< Следовать по ярлыкам Windows.
        };
        typedef QFlags<TFilter> TFilters;

        //! Структура с параметрами работы перечислителя.
        struct TParams {
            QString          startPath;       //!< Начальный каталог.
            TFilters         filter;          //!< Фильтр элементов.
            TFileStatOptions dirStatOptions;  //!< Получаемая информация о подкаталогах.
            int              subdirsDepth;    //!< Глубина обхода подкаталогов
                                              //!< (-1 - неограниченно).
            //! Конструктор.
            TParams() : filter(Files), subdirsDepth(0) { }
        };

    private :
        //! Возможные результаты функции \c analyze.
        enum TAnalyseResult {
            arOK,    //!< Анализ успешно завершён.
            arNext,  //!< Элемент верный, но не удовлетворяет требованиям фильтра.
            arBad    //!< Элемент ошибочный (циклическая ссылка и т.п.).
        };

        //! Общий автоматический указатель на QDirIterator.
        typedef QSharedPointer<TDirIterator> TSharedIterator;

        //! Перечислитель элементов каталога.
        struct TIterator {
            TSharedIterator Iterator;   //!< Итератор каталога.
            QString         Name;       //!< Имя каталога (без пути!)
            QString         RelName;    //!< Относительное имя каталога.
            TFileInfoEx     Info;       //!< Информация о каталоге.

            //! Конструктор пустого элемента.
            TIterator() { }
            //! Конструктор.
            TIterator(TDirIterator* It, const QString& Name,
                      const QString& RelName, const TFileInfoEx& Info)
                : Iterator(It), Name(Name), RelName(RelName), Info(Info) { }
            //! Деструктор.
            ~TIterator() { }
        };
        typedef QStack<TIterator> TIteratorsStack;

        TParams                m_Params;          //!< Параметры работы.
        TDirIterator::TFilters m_DirFilters;      //!< Фильтр.
        TIteratorsStack        m_Iterators;       //!< Итераторы.
        TIteratorsStack        m_InIterators;     //!< "Новые" итераторы.
        TIteratorsStack        m_OutIterators;    //!< "Старые" итераторы.
        TFileInfoEx            m_FileInfoEx;      //!< Информация о текущем элементе.
        bool                   m_Started;         //!< Флаг запуска перечислителя.
        bool                   m_RelNameWithRoot; //!< Относительный путь с корнем.
        mutable QString        m_RelPath;         //!< Относительный путь.
        mutable bool           m_RelPathValid;    //!< Флаг валидности относительного пути.

        TAnalyseResult analyze();
        bool start();
        void startDir(const QString* pAltName = NULL);
        void endDir();

        // Скрываем копирующий конструктор и оператор присваивания.
        Q_DISABLE_COPY(TDirEnumerator)

    public:
        TDirEnumerator();
        ~TDirEnumerator();

        bool start(const TParams* pParams);
        bool start(const TParams& Params);
        bool start(const QString& StartPath,
                   const TFilters Filter = TFilters(Files),
                   const TFileStatOptions Options = TFileStatOptions(),
                   const int SubdirsDepth = -1);
        bool next();
        void finish();

        TFileInfoEx info() const;
        const TFileInfoEx* infoPtr() const;

        QString name() const;
        QString relPath(bool WithRoot) const;
        QString relPath() const;
        QString relName(bool WithRoot) const;
        QString relName() const;
        QString subdirRelName(int i) const;
        QString subdirInRelName(int i) const;
        QString subdirOutRelName(int i) const;

        bool isStarted() const;

        TParams params() const;
        QString startDirPath() const;
        int subdirsDepth() const;
        TFilters filter() const;
        TFileStatOptions dirStatOptions() const;

        bool relNameWithRoot() const;
        void setRelNameWithRoot(bool Flag);
        int subdirCount() const;
        int subdirInCount() const;
        int subdirOutCount() const;
        QString subdirsInPath() const;
        QString subdirsOutPath() const;
        TFileInfoEx subdirInfo(int i) const;
        TFileInfoEx subdirInInfo(int i) const;
        TFileInfoEx subdirOutInfo(int i) const;
        TFileStat subdirStat(int i) const;
        TFileStat subdirInStat(int i) const;
        TFileStat subdirOutStat(int i) const;
        QStringList subdirPaths() const;
        QStringList subdirInPaths() const;
        QStringList subdirOutPaths() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TDirEnumerator::TFilters)

//------------------------------------------------------------------------------

#endif // __DIRENUMERATOR__HPP__
