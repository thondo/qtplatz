//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "manager_i.h"
#include "session_i.h"
#include "logger_i.h"

using namespace adbroker;

adbroker::manager_i::manager_i(void) 
{
}

adbroker::manager_i::~manager_i(void)
{
}

void
adbroker::manager_i::shutdown()
{
    PortableServer::POA_var poa = adbroker::singleton::manager::instance()->getServantManager()->root_poa();
    if ( logger_i_ )
        poa->deactivate_object( logger_i_->oid() );
}

Broker::Session_ptr
manager_i::getSession( const CORBA::WChar * token )
{
    PortableServer::POA_var poa = ::adbroker::singleton::manager::instance()->getServantManager()->root_poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    session_map_type::iterator it = session_list_.find( token );
    if ( it == session_list_.end() ) 
        session_list_[ token ].reset( new adbroker::session_i() );

    CORBA::Object_ptr obj = poa->servant_to_reference( session_list_[ token ].get() );
    try {
        return Broker::Session::_narrow( obj );
    } catch ( CORBA::Exception& ) {
        return 0;
    }
}

Broker::Logger_ptr
manager_i::getLogger()
{
    PortableServer::POA_var poa = ::adbroker::singleton::manager::instance()->getServantManager()->root_poa();
    if ( CORBA::is_nil( poa ) )
        return 0;

    if ( ! logger_i_ ) {
        logger_i_.reset( new broker::logger_i( poa ) );
        PortableServer::ObjectId * oid = poa->activate_object( logger_i_.get() );
        logger_i_->oid( *oid );
    }

	CORBA::Object_var obj = poa->servant_to_reference( logger_i_.get() );
    try {
        return Broker::Logger::_narrow( obj );
    } catch ( CORBA::Exception& ) {
        return 0;
    }
}

void
manager_i::register_ior( const char * name, const char * ior )
{
	iorMap_[ name ] = ior;
}

char *
manager_i::ior( const char * name )
{
	std::map< std::string, std::string >::iterator it = iorMap_.find( name );
    if ( it != iorMap_.end() )
		return CORBA::string_dup( it->second.c_str() );
	return 0;
}
