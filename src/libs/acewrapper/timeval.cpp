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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "timeval.hpp"
# pragma warning (disable: 4996)
#  include <ACE/Time_Value.h>
#  include <ACE/High_Res_Timer.h>
#  include <ACE/OS_NS_sys_time.h>
# pragma warning (default: 4996)
#include <sstream>
#include <string>
#include <time.h>
#include <iomanip>

namespace acewrapper {

    // CAUTION:  do not use this function for general string variables
    // because this is not locale safe.  Only it works with Latein-1 text
    static std::wstring to_wstring( const std::string& sstr )
    {
        std::wstring wstr( sstr.size() + 1, L'\0' );
        std::copy( sstr.begin(), sstr.end(), wstr.begin() );
        return wstr;
    }

    void gettimeofday(time_t& tv_sec, long& tv_usec)
    {
        // ACE_Time_Value tv = ACE_High_Res_Timer::gettimeofday_hr();
        ACE_Time_Value tv = ACE_OS::gettimeofday();
        tv_sec = tv.sec();
        tv_usec = tv.usec();
    }
  
    std::string to_string( time_t tm ) 
    {
        char tbuf[64];
        ctime_s(tbuf, sizeof(tbuf), &tm);
        *::strrchr( tbuf, '\n' ) = '\0';
        return std::string( tbuf );
    }

    std::string to_string( unsigned long long sec, unsigned long usec )
    {
        std::ostringstream o;
        o << to_string( time_t(sec) ) << " " << std::fixed << std::setw(7) << std::setfill('0') << std::setprecision(3) << double( usec ) / 1000.0;
        std::string ret = o.str();
        return ret;
    }

    std::string to_string( const ACE_Time_Value& tv ) 
    {
        return to_string( tv.sec(), tv.usec() );
    }

    std::wstring to_wstring( const ACE_Time_Value& tv )
    {
        return to_wstring( to_string( tv ) );
    }

    std::wstring to_wstring( unsigned long long sec, unsigned long usec )
    {
        return to_wstring( to_string( sec, usec ) );
    }

}
