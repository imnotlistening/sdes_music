#-------------------------------------------------
#
# Project created by QtCreator 2012-04-23T11:32:54
#
#-------------------------------------------------

QT	+= core
QT	+= sql
QT	+= gui

TARGET = client
TEMPLATE = app

INCLUDEPATH += 	../stream/include\
				../qserialport/include/QtSerialPort
win32{
INCLUDEPATH +=	../GStreamer/v0.10.7/sdk/include/libxml2 \
                ../GStreamer/v0.10.7/sdk/include/glib-2.0 \
                ../GStreamer/v0.10.7/sdk/include/gstreamer-0.10 \
                ../GStreamer/v0.10.7/sdk/include
}				
		
SOURCES += 	src/main.cpp \
			src/ifacedb.cpp \
			src/tcstreamthread.cpp \
			src/tcstreamwrapper.cpp \
			src/tcclientctrl.cpp \
			src/IRInterface.cpp \
			src/tcsystemtray.cpp
			
HEADERS +=	src/tsuserdata.h \
			../stream/include/stream.h \
			src/ifacedb.h \
			src/tcstreamthread.h \
			src/tcstreamwrapper.h \
			src/tcclientctrl.h \
			src/IRInterface.h \
			src/tcsystemtray.h

LIBS += -L../stream/lib -lstream
LIBS += -L../qserialport/lib -lQtSerialPort1

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-0.10
}
win32 {
LIBS += \
		-l../GStreamer/v0.10.7/sdk/lib/gstreamer-0.10 \
        -l../GStreamer/v0.10.7/sdk/lib/gobject-2.0 \
        -l../GStreamer/v0.10.7/sdk/lib/glib-2.0 \
        -l../GStreamer/v0.10.7/sdk/lib/xml2
}

DESTDIR = ./bin
MOC_DIR = ./generatedfiles
OBJECTS_DIR = ./generatedfiles
