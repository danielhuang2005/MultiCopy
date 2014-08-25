#-------------------------------------------------
#
# Project created by QtCreator 2011-08-25T10:29:07
#
#-------------------------------------------------

QT       += core gui

TARGET = MultiCopy
TEMPLATE = app


SOURCES += main.cpp\
    Synchronizer.cpp \
    Buffer.cpp \
    CircularBuffer.cpp \
    MultiCopyForm.cpp \
    Settings.cpp \
    SettingsForm.cpp \
    ProgressForm.cpp \
    ErrorHandler.cpp \
    ThreadEx.cpp \
    RWCalculator.cpp \
    TimeCounter.cpp \
    Translator.cpp \
    FileReader.cpp \
    FileWriter.cpp \
    ControlThread.cpp

HEADERS  += \
    Synchronizer.hpp \
    Buffer.hpp \
    CircularBuffer.hpp \
    MultiCopyForm.hpp \
    Settings.hpp \
    SettingsForm.hpp \
    ProgressForm.hpp \
    ErrorHandler.hpp \
    ThreadEx.hpp \
    RWCalculator.hpp \
    TimeCounter.hpp \
    Translator.hpp \
    FileReader.hpp \
    FileWriter.hpp \
    ControlThread.hpp

FORMS    += \
    MultiCopyForm.ui \
    SettingsForm.ui \
    ProgressForm.ui

TRANSLATIONS += \
    MultiCopy.ru_RU.ts

OTHER_FILES += \
    MultiCopy.Win.rc \
    history.txt

RESOURCES += \
    MultiCopy.qrc

win32 {
    RC_FILE += \
        MultiCopy.Win.rc
}

