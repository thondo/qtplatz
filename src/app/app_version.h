/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef APP_VERSION_H
#define APP_VERSION_H

namespace Core {
namespace Constants {

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

#define IDE_VERSION 3.2.81
#define IDE_VERSION_STR STRINGIFY(IDE_VERSION)

#define IDE_VERSION_MAJOR 3
#define IDE_VERSION_MINOR 2
#define IDE_VERSION_RELEASE 81

const char * const IDE_VERSION_LONG      = IDE_VERSION_STR;
const char * const IDE_AUTHOR            = "MS-Cheminformatics";
const char * const IDE_YEAR              = "2014";

#ifdef IDE_VERSION_DESCRIPTION
const char * const IDE_VERSION_DESCRIPTION_STR = STRINGIFY(IDE_VERSION_DESCRIPTION);
#else
const char * const IDE_VERSION_DESCRIPTION_STR = "";
#endif

#ifdef IDE_REVISION
const char * const IDE_REVISION_STR      = STRINGIFY(IDE_REVISION);
#else
const char * const IDE_REVISION_STR      = "";
#endif

// changes the path where the settings are saved to
#ifdef IDE_SETTINGSVARIANT
const char * const IDE_SETTINGSVARIANT_STR      = STRINGIFY(IDE_SETTINGSVARIANT);
#else
const char * const IDE_SETTINGSVARIANT_STR      = "qtplatz";
#endif

#ifdef IDE_COPY_SETTINGS_FROM_VARIANT
const char * const IDE_COPY_SETTINGS_FROM_VARIANT_STR = STRINGIFY(IDE_COPY_SETTINGS_FROM_VARIANT);
#else
const char * const IDE_COPY_SETTINGS_FROM_VARIANT_STR = "MS-Cheminfomatics";
#endif


#undef IDE_VERSION
#undef IDE_VERSION_STR
#undef STRINGIFY
#undef STRINGIFY_INTERNAL

} // Constants
} // Core

#endif //APP_VERSION_H
