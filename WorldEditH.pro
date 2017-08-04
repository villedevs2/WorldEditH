# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = WorldEditH
DESTDIR = ../x64/Release
QT += core xml xmlpatterns opengl widgets gui
CONFIG += release
DEFINES += WIN64 QT_DLL QT_OPENGL_LIB QT_WIDGETS_LIB QT_XML_LIB QT_XMLPATTERNS_LIB
INCLUDEPATH += ./GeneratedFiles \
    . \
    ./GeneratedFiles/Release \
    $(ProjectDir)glm
LIBS += -lopengl32 \
    -lglu32
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
include(WorldEditH.pri)
win32:RC_FILE = WorldEditH.rc