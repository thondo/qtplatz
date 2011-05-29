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

#include "apiposix.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdlib>
#include <pwd.h>
#include <adportable/string.hpp>

namespace adfs {
    namespace detail {

        template<> std::string
        posixapi::get_login_name()
        {
            return std::string();
        }
        
        template<> std::wstring
        posixapi::get_login_name()
        {
            uid_t uid = geteuid();
            struct passwd * pw = getpwuid( uid );
            return adportable::string::convert( pw->pw_name );
        }
        
        std::wstring
        posixapi::create_uuid()
        {
            const boost::uuids::uuid id = boost::uuids::random_generator()();
            return boost::lexical_cast<std::wstring>(id);
        }

    };
};
