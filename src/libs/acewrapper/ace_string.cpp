// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ace_string.hpp"
#include <sstream>
#include <ace/INET_Addr.h>

using namespace acewrapper;

namespace acewrapper {

    template<>
    basic_string<char>::basic_string( const ACE_INET_Addr& addr )
    {
	ACE_TCHAR tbuf[1024];
	memset(tbuf, 0, sizeof(tbuf));
	addr.addr_to_string( tbuf, sizeof(tbuf)/sizeof(tbuf[0]) );
	std::basic_ostringstream<char> o;
	o << tbuf;
	t_ = o.str();
    }
    
    template<>
    basic_string<wchar_t>::basic_string( const ACE_INET_Addr& addr )
    {
	ACE_TCHAR tbuf[1024];
	memset(tbuf, 0, sizeof(tbuf));
	addr.addr_to_string( tbuf, sizeof(tbuf)/sizeof(tbuf[0]) );
	std::basic_ostringstream<wchar_t> o;
	o << tbuf;
	t_ = o.str();
    }
}; // namespace acewrapper
