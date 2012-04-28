#ifndef STREAM_GLOBAL_H
#define STREAM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(STREAM_LIBRARY)
#  define STREAMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define STREAMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // STREAM_GLOBAL_H
