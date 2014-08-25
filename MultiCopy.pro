#*******************************************************************************
#              Copyright (С) 2012 Юрий Владимирович Круглов
#
#   Эта программа является свободным программным обеспечением. Вы можете
#   распространять и/или  модифицировать её согласно условиям Стандартной
#   Общественной Лицензии GNU, опубликованной Организацией Свободного
#   Программного Обеспечения, версии 3, либо по Вашему желанию, любой более
#   поздней версии.
#
#   Эта программа распространяется в надежде на то, что окажется полезной, но
#   БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, даже без подразумеваемых гарантий ОКУПАЕМОСТИ или
#   СООТВЕТСТВИЯ КОНКРЕТНЫМ ЦЕЛЯМ.
#   Подробнее - см. Стандартной Общественную Лицензию GNU.
#
#   Вы должны были получить копию Основной Общественной Лицензии GNU вместе с
#   этой программой. При её отсутствии обратитесь на сайт
#   http://www.gnu.org/licenses/.
#
#*******************************************************************************
#
#                   Copyright (C) 2012 Yuri V. Krugloff
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the Free
#   Software Foundation, either version 3 of the License, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
#   more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#******************************************************************************/

include(Core/AppInstances/AppInstances.pri)
include(Core/FastIO/FastIO.pri)
#include(profiler.pri)

QT += core gui

CONFIG(debug, debug|release){
    TARGET = MultiCopy_d
} else {
    TARGET = MultiCopy
    DEFINES += QT_NO_WARNING_OUTPUT \
               QT_NO_DEBUG_OUTPUT \
               QT_NO_DEBUG
}

TEMPLATE = app

DESTDIR = ../bin

INCLUDEPATH += GUI/Widgets

HEADERS += \
    Core/Buffer/Buffer.hpp \
    Core/Buffer/BufferCell.hpp \
    Core/Buffer/CircularBuffer.hpp \
    Core/Common/CommonFn.hpp \
    Core/Common/SharedMemory.hpp \
    Core/Errors/ErrorHandler.hpp \
    Core/Errors/ErrorsAndActions.hpp \
    Core/Errors/LastActions.hpp \
    Core/IO/DirEnumerator.hpp \
    Core/Sync/ProducerLocker.hpp \
    Core/Sync/SemaphoreEx.hpp \
    Core/Sync/Synchronizer.hpp \
    Core/Task/Command.hpp \
    Core/Task/GlobalStatistics.hpp \
    Core/Task/Task.hpp \
    Core/Task/TaskModel.hpp \
    Core/Task/TaskStatus.hpp \
    Core/Threads/Reader.hpp \
    Core/Threads/SizeCalculator.hpp \
    Core/Threads/TaskManager.hpp \
    Core/Threads/ThreadEx.hpp \
    Core/Threads/Writer.hpp \
    Core/Resources.hpp \
    Core/TimeCounter.hpp \
    GUI/Forms/MultiCopyForm.hpp \
    GUI/Forms/ProgressForm.hpp \
    GUI/Forms/SettingsForm.hpp \
    GUI/Forms/TaskSettingsForm.hpp \
    GUI/Widgets/QListWidget2.hpp \
    GUI/Widgets/QPushButton2.hpp \
    GUI/GUIErrorHandler.hpp \
    GUI/Settings.hpp \
    GUI/Translator.hpp

SOURCES += main.cpp\
    Core/Buffer/Buffer.cpp \
    Core/Buffer/BufferCell.cpp \
    Core/Buffer/CircularBuffer.cpp \
    Core/Common/CommonFn.cpp \
    Core/Errors/ErrorHandler.cpp \
    Core/Errors/LastActions.cpp \
    Core/IO/DirEnumerator.cpp \
    Core/Sync/ProducerLocker.cpp \
    Core/Sync/SemaphoreEx.cpp \
    Core/Sync/Synchronizer.cpp \
    Core/Task/Command.cpp \
    Core/Task/GlobalStatistics.cpp \
    Core/Task/Task.cpp \
    Core/Task/TaskModel.cpp \
    Core/Task/TaskStatus.cpp \
    Core/Threads/Reader.cpp \
    Core/Threads/SizeCalculator.cpp \
    Core/Threads/TaskManager.cpp \
    Core/Threads/ThreadEx.cpp \
    Core/Threads/Writer.cpp \
    Core/TimeCounter.cpp \
    GUI/Forms/MultiCopyForm.cpp \
    GUI/Forms/ProgressForm.cpp \
    GUI/Forms/SettingsForm.cpp \
    GUI/Forms/TaskSettingsForm.cpp \
    GUI/Widgets/QListWidget2.cpp \
    GUI/Widgets/QPushButton2.cpp \
    GUI/GUIErrorHandler.cpp \
    GUI/Settings.cpp \
    GUI/Translator.cpp

FORMS += \
    GUI/Forms/MultiCopyForm.ui \
    GUI/Forms/ProgressForm.ui \
    GUI/Forms/SettingsForm.ui \
    GUI/Forms/TaskSettingsForm.ui

TRANSLATIONS += \
    GUI/i18n/MultiCopy.ru_RU.ts

OTHER_FILES += \
    Documentation/history.txt \
    Documentation/TODO.txt \
    GUI/MultiCopy.Win.rc \
    GUI/MultiCopy.Win64.rc

RESOURCES += \
    GUI/Forms/Resources.qrc

contains(QMAKE_HOST.arch, x86_64) {
    RESOURCES += GUI/Forms/MultiCopy64.qrc
    win32 {
        RC_FILE += GUI/MultiCopy.Win64.rc
    }
} else {
    RESOURCES += GUI/Forms/MultiCopy.qrc
    win32 {
        RC_FILE += GUI/MultiCopy.Win.rc
    }
}

# DEFINES += _NO_NONPAGED_MEM
