QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FaceRecognition_Client
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 console

SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp

HEADERS += \
        src/mainwindow.h

FORMS += \
        src/mainwindow.ui

INCLUDEPATH += $$PWD/../OpenCV/include          # opencv 4.5.5

# moc rcc ui obj 编译文件
CONFIG(debug, debug|release) {
        OBJECTS_DIR =     $$PWD/tmp/Debug
        RCC_DIR =         $$PWD/tmp/Debug
        UI_DIR  =         $$PWD/tmp/Debug
        MOC_DIR =         $$PWD/tmp/Debug
} else {
        OBJECTS_DIR =     $$PWD/tmp/Release
        RCC_DIR =         $$PWD/tmp/Release
        UI_DIR  =         $$PWD/tmp/Release
        MOC_DIR =         $$PWD/tmp/Release
}

# DESTDIR
CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/x64/Debug
        LIBS += -L$$PWD/../OpenCV/lib/ -lopencv_world455d
} else {
        DESTDIR = $$PWD/x64/Release
        LIBS += -L$$PWD/../OpenCV/lib/ -lopencv_world455
}

QMAKE_CXXFLAGS += /MP

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
