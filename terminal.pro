QT += widgets serialport printsupport


TARGET = terminal
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    qcustomplot.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    qcustomplot.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc
CONFIG+= static
