QT += core gui widgets sql charts
CONFIG += c++17
TARGET = StudentManager
TEMPLATE = app

SOURCES += \
    main.cpp \
    database/DbManager.cpp \
    models/TableModel.cpp \
    ui/DbConfigDialog.cpp \
    ui/LoginDialog.cpp \
    ui/MainWindow.cpp \
    ui/ClassPage.cpp \
    ui/TeacherPage.cpp \
    dialogs/ClassEditDialog.cpp \
    dialogs/TeacherEditDialog.cpp

HEADERS += \
    database/DbManager.h \
    models/TableModel.h \
    ui/DbConfigDialog.h \
    ui/LoginDialog.h \
    ui/MainWindow.h \
    ui/ClassPage.h \
    ui/TeacherPage.h \
    dialogs/ClassEditDialog.h \
    dialogs/TeacherEditDialog.h
