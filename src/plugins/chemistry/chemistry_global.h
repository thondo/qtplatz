#ifndef CHEMISTRY_GLOBAL_H
#define CHEMISTRY_GLOBAL_H

#include <QtGlobal>

#if defined(CHEMISTRY_LIBRARY)
#  define CHEMISTRYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CHEMISTRYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CHEMISTRY_GLOBAL_H

