#-------------------------------------------------
#
# Project created by QtCreator 2018-03-24T13:55:19
#
#-------------------------------------------------

QT       += core gui sql

QT += widgets

TARGET = EVChargerDemo
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES +=\
        KevDemoMainWindow.cpp \
        KevDemoVLCDecoder.cpp \
        KevDemoROIBlock.cpp \
        KevDemoDatabase.cpp \
        KevDemoVBCDecoder.cpp \
        KevDemoCameraPreview.cpp \
        KevDemoVBCReader.cpp \
        KevDemoEVehicle.cpp \
        KevDemoEVCharger.cpp \
        KevDemoServerConn.cpp \
    KevDemoMain.cpp

HEADERS  += KevDemoMainWindow.h \
            KevDemoConfig.h \
            KevDemoVLCDecoder.h \
            KevDemoROIBlock.h \
            KevDemoDatabase.h \
            KevDemoVBCDecoder.h \
            KevDemoCameraPreview.h \
            KevDemoVBCReader.h \
            KevDemoEVehicle.h \
            KevDemoEVCharger.h \
            KevDemoServerConn.h

FORMS    += KevDemoMainWindow.ui

INCLUDEPATH += /usr/local/include

LIBS += -lopencv_core        \
        -lopencv_highgui     \
        -lopencv_video       \
        -lopencv_imgcodecs   \
        -lopencv_imgproc     \
        -lopencv_videoio     \
        -lopencv_features2d  \
        -lopencv_calib3d     \
        -lwiringPi

RESOURCES += \
        KevDemoResource.qrc

