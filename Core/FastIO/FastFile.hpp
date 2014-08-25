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

#ifndef __FASTFILE__HPP__55EF0A7A_D675_4D11_B523_F04184A36AFF__
#define __FASTFILE__HPP__55EF0A7A_D675_4D11_B523_F04184A36AFF__

#include <QtGlobal>
#include <QString>
//#include <QIODevice>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

//------------------------------------------------------------------------------
//! Класс для быстрых операций с файлами в Windows.
/*!
   Класс оптимизирован для последовательной работы.

   \remarks Все методы класса выполняют те же функции, что и соответствующие
     методы класса QFile.
 */

class TFastFile
{
    private :
        QString m_FileName;      //!< Имя файла.
        QString m_ErrorString;   //!< Строка с сообщением об ошибке.
        bool    m_DirectAccess;  //!< Флаг прямого доступа к файлу (без кэша).
        int     m_BlockSize;     //!< Размер блока при прямом доступе.
        #ifdef Q_OS_WIN
            HANDLE m_Handle;    //!< Дескриптор файла.
        #else
            int m_fd;           //!< Дескриптор файла.
        #endif

        void setErrorString();
    public:
        //! Режимы открытия файла.
        enum OpenModeFlag {
            omRead  = 0,  //!< Чтение.
            omWrite = 1   //!< Запись.
        };
        typedef QFlags<OpenModeFlag> OpenMode;

        TFastFile();
        ~TFastFile();

        QString fileName() const;
        void setFileName(const QString& FileName);
        bool open(OpenMode Mode, bool DirectAccess = false, int BlockSize = -1);
        void close();
        QString errorString() const;
        bool isOpen() const;
        qint64 read(char* data, qint64 maxSize);
        qint64 write(const char* data, qint64 maxSize);
        qint64 pos();
        bool seek(qint64 pos);
        bool resize(qint64 Size);
        qint64 size();
        bool remove();

        //! Прямой доступ к файлу (без кэша).
        inline bool directAccess() const { return m_DirectAccess; }
        //! Размер блока при прямом доступе.
        /*!
           \remarks Если прямой доступ отключен, метод возвращает -1.
         */
        inline int blockSize() const { return m_BlockSize; }

        static bool remove(const QString& FileName, QString* pErrorString = NULL);
        static bool resize(const QString& FileName, qint64 Size);
};

//------------------------------------------------------------------------------

#endif // __FASTFILE__HPP__55EF0A7A_D675_4D11_B523_F04184A36AFF__
