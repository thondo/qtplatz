/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#pragma once

#include <string>
#include <sstream>
#include "adlog_global.hpp"
#include <compiler/diagnostic_push.h>
#include <compiler/disable_dll_interface.h>

namespace boost { namespace system { class error_code; } }

namespace adlog {

    class ADLOGSHARED_EXPORT logger {
    public:
        logger( const char * file = 0, int line = 0, int pri = 0 );
        ~logger();
        template<typename T> inline logger& operator << ( const T& t ) { o_ << t; return *this; }
        template<> logger& operator << ( const std::wstring& );
        template<> logger& operator << ( const boost::system::error_code& );
		logger& operator << ( const wchar_t *);

    private:
        int pri_;
        int line_;
        std::string file_;
        std::ostringstream o_;
    };

    enum { LOG_TRACE, LOG_INFO, LOG_WARN, LOG_ERROR };

}
#include <compiler/diagnostic_pop.h>

# define ADLOG(x)  adlog::logger(__FILE__, __LINE__,(x))
# define ADTRACE() adlog::logger(__FILE__, __LINE__,adlog::LOG_TRACE)
# define ADINFO()  adlog::logger(__FILE__, __LINE__,adlog::LOG_INFO)
# define ADWARN()  adlog::logger(__FILE__, __LINE__,adlog::LOG_WARN)
# define ADERROR() adlog::logger(__FILE__, __LINE__,adlog::LOG_ERROR)
