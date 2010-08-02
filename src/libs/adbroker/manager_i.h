// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adinterface/brokerS.h"
#include <acewrapper/orbservant.h>
#include <map>
#include <string>
#include "logger_i.h"
#include "session_i.h"
#include <boost/smart_ptr.hpp>

class manager_i;

namespace singleton {
	namespace broker {
		typedef ACE_Singleton< acewrapper::ORBServant< manager_i >, ACE_Recursive_Thread_Mutex > manager;
	}
}

class manager_i : public virtual POA_Broker::Manager {

public:
    manager_i(void);
    ~manager_i(void);

    void shutdown();
    Broker::Session_ptr getSession( const CORBA::WChar * );
    Broker::Logger_ptr getLogger();
private:
    typedef std::map< std::wstring, boost::shared_ptr< broker::session_i > > session_map_type;
    session_map_type session_list_;
    boost::scoped_ptr< broker::logger_i > logger_i_;
};


