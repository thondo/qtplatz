// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

namespace adfs {

    class exception {
    public:
        exception( const std::string& msg, const char * cat ) : message(msg), category(cat) {}
        std::string message;
        std::string category;
    };

    class sqlite;
    class Folium;
    class Folder;

    class filesystem {
        sqlite * db_;
    public:
        ~filesystem();
        filesystem();
        bool create( const wchar_t * filename, size_t alloc = 0, size_t page_size = 8192 );
        bool mount( const wchar_t * filename );
        bool close();
        //
        Folder addFolder( const wchar_t * path );
    private:
        bool prealloc( size_t size );
    };

} // adfs

#endif // FILESYSTEM_H
