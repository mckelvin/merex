unix:LIBS += -lmarsyas

QT       += core gui

TARGET = MER_EX
TEMPLATE = app


SOURCES += main.cpp \
	mainwindow.cpp \
    extractor.cpp \
    marsyasbackend.cpp \
    playbackthread.cpp \
    backend_main.cpp

HEADERS  += mainwindow.h \
    config.h \
    marsyasbackend.h \
    extractor.h \
    playbackthread.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc



