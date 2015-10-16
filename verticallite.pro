#-------------------------------------------------
#
# Project created by QtCreator 2012-06-01T11:05:15
#
#-------------------------------------------------

QT       += core gui


TARGET = verticallite
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

SOURCES += main.cpp\
        mainwindow.cpp \
    dialogconnect.cpp \
    loodsmansystem.cpp \
    treeloodsmanmodel.cpp \
    treeitem.cpp \
    converttype.cpp \
    application.cpp \
    midasdata.cpp \
    typeloodsmanmodel.cpp \
    linkloodsmanmodel.cpp \
    statloodsmanmodel.cpp \
    attrloodsmanmodel.cpp \
    attreditloodsmanmodel.cpp \
    attrlinkkey.cpp \
    propeditordelegate.cpp \
    treepropview.cpp \
    unitsloodsmanmodel.cpp

HEADERS  += mainwindow.h \
    dialogconnect.h \
    converttype.h \
    loodsmansystem.h \
    treeloodsmanmodel.h \
    treeitem.h \
    application.h \
    midasdata.h \
    typeloodsmanmodel.h \
    linkloodsmanmodel.h \
    statloodsmanmodel.h \
    treenode.h \
    attrloodsmanmodel.h \
    attreditloodsmanmodel.h \
    attrlinkkey.h \
    propeditordelegate.h \
    treepropview.h \
    unitsloodsmanmodel.h

FORMS    += mainwindow.ui \
    dialogconnect.ui

RESOURCES +=

