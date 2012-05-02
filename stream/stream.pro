#-------------------------------------------------
#
# Project created by QtCreator 2012-04-23T11:32:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = stream
TEMPLATE = lib

DEFINES += STREAM_LIBRARY
INCLUDEPATH += 	../GStreamer/v0.10.7/sdk/include/libxml2 \
                ../GStreamer/v0.10.7/sdk/include/glib-2.0 \
                ../GStreamer/v0.10.7/sdk/include/gstreamer-0.10 \
                ../GStreamer/v0.10.7/sdk/include

SOURCES +=\
                src/stream_core.cpp \
                src/gst_stream.cpp

HEADERS +=\
        include/stream_global.h \
		include/stream.h

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-0.10
}
win32 {
LIBS += -l../GStreamer/v0.10.7/sdk/lib/gstreamer-0.10 \
        -l../GStreamer/v0.10.7/sdk/lib/gobject-2.0 \
        -l../GStreamer/v0.10.7/sdk/lib/glib-2.0 \
        -l../GStreamer/v0.10.7/sdk/lib/xml2
}

DESTDIR = ./lib
MOC_DIR = ./generatedfiles/desk
OBJECTS_DIR = ./generatedfiles/desk

#unix:!symbian {
#    maemo5 {
#        target.path = /opt/usr/lib
#    } else {
#        target.path = /usr/lib
#    }
#    INSTALLS += target
#}
