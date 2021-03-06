// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "eventlog_helper.hpp"
#include "eventlogC.h"
#include <adportable/utf.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <chrono>

using namespace adinterface;
using namespace adinterface::EventLog;

namespace adinterface { namespace EventLog {

        template<> LogMessageHelper& LogMessageHelper::operator % (const std::wstring & t) {
            msg_.args.length( msg_.args.length() + 1 );
            msg_.args[ msg_.args.length() - 1 ]
                = CORBA::string_dup( adportable::utf::to_utf8( t ).c_str() );
            return *this;
        }
        
    }
}

std::wstring
adinterface::EventLog::LogMessageHelper::toString( const ::EventLog::LogMessage& msg )
{
    auto fmt = adportable::utf::to_wstring( msg.format.in() );
    CORBA::ULong narg = msg.args.length();

    boost::wformat format( fmt );

    for ( CORBA::ULong i = 0; i < narg; ++i )
        format % adportable::utf::to_wstring( msg.args[i].in() );

    std::wostringstream o;
    try {
        o << format;
    } catch ( boost::exception& ) {
        return fmt;
    }
    return o.str();
}

LogMessageHelper::LogMessageHelper( const std::wstring& format
                                   , unsigned long pri
                                    , const std::wstring& msgId
                                    , const std::wstring& srcId )
{
	msg_.priority = pri;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	msg_.tv.sec = time_t( std::chrono::duration_cast< std::chrono::seconds >(duration).count() );
	msg_.tv.usec = long( std::chrono::duration_cast< std::chrono::microseconds >( duration ).count() - (msg_.tv.sec * 1000000) );

    if ( ! msgId.empty() )
        msg_.msgId = CORBA::string_dup( adportable::utf::to_utf8( msgId ).c_str() );
    if ( ! srcId.empty() )
        msg_.srcId = CORBA::string_dup( adportable::utf::to_utf8( srcId ).c_str() );
    if ( ! format.empty() )
        msg_.format = CORBA::string_dup( adportable::utf::to_utf8( format ).c_str() );
}

LogMessageHelper::LogMessageHelper( const LogMessageHelper& t ) : msg_( t.msg_ )
{
}

LogMessageHelper&
LogMessageHelper::format( const std::wstring& fmt )
{
    msg_.format = CORBA::string_dup( adportable::utf::to_utf8( fmt ).c_str() );
    return *this;
}

