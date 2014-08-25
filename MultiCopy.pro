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

SOURCES += main.cpp\
    Core/Sync/Synchronizer.cpp \
    Core/Sync/ProducerLocker.cpp \
    Core/Sync/SemaphoreEx.cpp \
    Core/Buffer/CircularBuffer.cpp \
    Core/Buffer/Buffer.cpp \
    Core/Common/CommonFn.cpp \
    Core/Threads/ThreadEx.cpp \
    GUI/Widgets/QListWidget2.cpp \
    GUI/Forms/SettingsForm.cpp \
    GUI/Forms/MultiCopyForm.cpp \
    GUI/Settings.cpp \
    Core/TimeCounter.cpp \
    GUI/Translator.cpp \
    Core/Buffer/BufferCell.cpp \
    Core/IO/DirEnumerator.cpp \
    Core/Threads/SizeCalculator.cpp \
    Core/Threads/TaskManager.cpp \
    Core/Threads/Reader.cpp \
    Core/Threads/Writer.cpp \
    GUI/Forms/TaskSettingsForm.cpp \
    Core/Task/TaskModel.cpp \
    Core/Task/Task.cpp \
    Core/Task/TaskStatus.cpp \
    Core/Task/GlobalStatistics.cpp \
    GUI/GUIErrorHandler.cpp \
    GUI/Widgets/QPushButton2.cpp \
    Core/Task/Command.cpp \
    GUI/Forms/ProgressForm.cpp \
    Core/Errors/LastActions.cpp \
    Core/Errors/ErrorHandler.cpp

HEADERS += \
    Core/Sync/Synchronizer.hpp \
    Core/Sync/ProducerLocker.hpp \
    Core/Sync/SemaphoreEx.hpp \
    Core/Buffer/CircularBuffer.hpp \
    Core/Buffer/Buffer.hpp \
    Core/Common/CommonFn.hpp \
    Core/Threads/ThreadEx.hpp \
    GUI/Widgets/QListWidget2.hpp \
    GUI/Forms/SettingsForm.hpp \
    GUI/Forms/MultiCopyForm.hpp \
    GUI/Settings.hpp \
    Core/TimeCounter.hpp \
    GUI/Translator.hpp \
    Core/Buffer/BufferCell.hpp \
    Core/IO/DirEnumerator.hpp \
    Core/Threads/SizeCalculator.hpp \
    Core/Threads/TaskManager.hpp \
    Core/Threads/Reader.hpp \
    Core/Threads/Writer.hpp \
    GUI/Forms/TaskSettingsForm.hpp \
    Core/Task/TaskModel.hpp \
    Core/Task/Task.hpp \
    Core/Task/TaskStatus.hpp \
    Core/Task/GlobalStatistics.hpp \
    GUI/Widgets/QPushButton2.hpp \
    GUI/GUIErrorHandler.hpp \
    Core/Task/Command.hpp \
    GUI/Forms/ProgressForm.hpp \
    Core/Errors/LastActions.hpp \
    Core/Errors/ErrorHandler.hpp \
    Core/Errors/ErrorsAndActions.hpp

FORMS += \
    GUI/Forms/SettingsForm.ui \
    GUI/Forms/ProgressForm.ui \
    GUI/Forms/MultiCopyForm.ui \
    GUI/Forms/TaskSettingsForm.ui

TRANSLATIONS += \
    GUI/i18n/MultiCopy.ru_RU.ts

OTHER_FILES += \
    Documentation/history.txt \
    GUI/MultiCopy.Win.rc \
    Documentation/TODO.txt

RESOURCES += \
    GUI/Forms/MultiCopy.qrc

win32 {
    RC_FILE += \
        GUI/MultiCopy.Win.rc
}

# DEFINES += _NO_FAST_FILE
# DEFINES += _NO_NONPAGED_MEM
