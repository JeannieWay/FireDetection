#-------------------------------------------------
#
# Project created by QtCreator 2015-09-23T16:12:45
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FireDetectionSystem
TEMPLATE = app

#openCV设置

INCLUDEPATH += E:/Qt/opencv/opencv/build/include
LIBS += E:/Qt/opencv/opencv/build/x86/mingw/bin/*.dll
#end opencv

SOURCES += main.cpp\
        mainframe.cpp \
    videoprocessor.cpp \
    src/emailaddress.cpp \
    src/mimeattachment.cpp \
    src/mimecontentformatter.cpp \
    src/mimefile.cpp \
    src/mimehtml.cpp \
    src/mimeinlinefile.cpp \
    src/mimemessage.cpp \
    src/mimemultipart.cpp \
    src/mimepart.cpp \
    src/mimetext.cpp \
    src/quotedprintable.cpp \
    src/smtpclient.cpp \
    sysconfigdialog.cpp \
    switchbutton.cpp \
    sysconfiginfo.cpp

HEADERS  += \
    videoprocessor.h \
    mainframe.h \
    src/emailaddress.h \
    src/mimeattachment.h \
    src/mimecontentformatter.h \
    src/mimefile.h \
    src/mimehtml.h \
    src/mimeinlinefile.h \
    src/mimemessage.h \
    src/mimemultipart.h \
    src/mimepart.h \
    src/mimetext.h \
    src/quotedprintable.h \
    src/smtpclient.h \
    src/SmtpMime \
    sysconfigdialog.h \
    switchbutton.h \
    sysconfiginfo.h

FORMS    += mainframe.ui \
    sysconfigdialog.ui

RESOURCES += \
    images.qrc

RC_FILE += myico.rc
