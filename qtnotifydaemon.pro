
HEADERS += notifywidget.h dbus.h qclicklabel.h notifyarea.h
TEMPLATE = app
TARGET = qtnotifydaemon
DEPENDPATH += .
INCLUDEPATH += .
QT += widgets dbus x11extras
LIBS += -lX11
RESOURCES += images.qrc

# Input
SOURCES += main.cpp notifywidget.cpp dbus.cpp notifyarea.cpp message.cpp
