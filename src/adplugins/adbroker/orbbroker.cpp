/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "orbbroker.hpp"
#include "adbroker.hpp"
#include "manager_i.hpp"
#include <acewrapper/orbservant.hpp>
#include <adplugin/orbservant.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbfactory.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <adlog/logger.hpp>
#include <adinterface/brokerC.h>
#include <adinterface/brokerclientC.h>
#include <boost/exception/all.hpp>


using namespace adbroker;

orbBroker::~orbBroker()
{
}

orbBroker::orbBroker()
{
}

bool
orbBroker::orbmgr_init( int ac, char * av [] ) const
{
    if ( adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance() ) {
        pMgr->init( ac, av );
        pMgr->spawn();
		return true;
    }
	return false;
}

void
orbBroker::orbmgr_shutdown()
{
    return adorbmgr::orbmgr::instance()->shutdown();
}

bool
orbBroker::orbmgr_fini()
{
    return adorbmgr::orbmgr::instance()->fini();
}

bool
orbBroker::orbmgr_wait()
{
    return adorbmgr::orbmgr::instance()->wait();
}

adplugin::orbServant *
orbBroker::create_instance() const
{
    return new adBroker;
}

adplugin::orbServant *
orbBroker::operator()( adplugin::plugin * plugin ) const
{
    struct error : virtual boost::exception, virtual std::exception {};
    typedef boost::error_info< struct tag_errmsg, std::string > info;

    if ( plugin ) {

        adorbmgr::orbmgr * pMgr = adorbmgr::orbmgr::instance();
        if ( adplugin::orbFactory * factory = plugin->query_interface< adplugin::orbFactory >() ) {

            if ( adplugin::orbServant * orbServant = factory->create_instance() ) {

                orbServant->initialize( pMgr->orb(), pMgr->root_poa(), pMgr->poa_manager() );
                std::string ior;
                try {
                    ior = orbServant->activate();
                    ADTRACE() << ior;
                }
                catch ( CORBA::Exception& ex ) {
                    BOOST_THROW_EXCEPTION( error() << info( ex._info().c_str() ) );
                }
                catch ( ... ) {
                    ADERROR() << "Exception at orbServant::activate call";
                    BOOST_THROW_EXCEPTION( error() << info( boost::current_exception_diagnostic_information() ) );
                }
                if ( !ior.empty() ) {

                    CORBA::Object_var obj = pMgr->orb()->string_to_object( ior.c_str() );

                    if ( CORBA::is_nil( obj ) ) {
                        ADERROR() << "can't get object reference for ior: " << ior;
                        return 0;
                    }

                    BrokerClient::Accessor_var accessor;
                    try {
                        accessor = BrokerClient::Accessor::_narrow( obj );
                    } catch ( CORBA::Exception& ex ) {
                        ADERROR() << "CORBA::Exception when narrow CORBA::Object to BrokerClient::Accessor obj = " << obj.in();
                        BOOST_THROW_EXCEPTION( error() << info( ex._info().c_str() ) );
                    } catch ( ... ) {
                        ADERROR() << "Exception when narrow CORBA::Object to BrokerClient::Accessor obj = " << obj.in();
                        BOOST_THROW_EXCEPTION( error() << info( boost::current_exception_diagnostic_information() ) );
                    }

                    if ( !CORBA::is_nil( accessor ) ) {

                        // set adpluginspec -- it includes lookup if required
                        Broker::Manager_var mgr = adbroker::manager_i::instance()->impl()._this();
                        accessor->setBrokerManager( mgr.in() );
                        accessor->adpluginspec( plugin->clsid(), plugin->adpluginspec() );

                        try {
                            mgr->register_object( orbServant->object_name(), obj );
                        } catch ( CORBA::Exception& ex ) {
                            factory->release( orbServant );
                            BOOST_THROW_EXCEPTION( error() << info( ex._info().c_str() ) );
                        } catch ( ... ) {
                            ADERROR() << "register_object";
                            BOOST_THROW_EXCEPTION( error() << info( boost::current_exception_diagnostic_information() ) );
                        }
                    }
                    return orbServant;
                }

            }
            else {
                ADTRACE() << plugin->clsid() << " nil instance created.";
            }

        }
        else {
            ADTRACE() << plugin->clsid() << " has on adplugin::orbFactory class.";
        }
    }

    return 0;
}

