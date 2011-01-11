/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "adcontroller.h"
#pragma warning (disable: 4996)
#include "ace/Init_ACE.h"
#pragma warning (default: 4996)

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "xmlwrapperd.lib")
#     pragma comment(lib, "adplugind.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "adinterface.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "xmlwrapper.lib")
#     pragma comment(lib, "adplugin.lib")
#  endif
#endif

#pragma warning (disable: 4996)
# include <ace/SOCK_Dgram_Mcast.h>
# include <ace/Service_Config.h>
# include <ace/Sched_Params.h>
# include <ace/Thread_Manager.h>
# include <ace/Process_Manager.h>
# include <ace/OS.h>
# include <tao/Utils/ORB_Manager.h>
#pragma warning (default: 4996)

#include "signal_handler.h"
#include <signal.h>
#include <acewrapper/orbservant.h>

#include <iostream>
#include <fstream>

#include <acewrapper/constants.h>
#include <boost/smart_ptr.hpp>
#include "manager_i.h"
#include "ibrokermanager.h"

using namespace acewrapper;

static int debug_flag = 0;
static bool __aborted = false;
static bool __own_thread = false;

static Receiver * __preceiver_debug;
std::string __ior_session;

//-----------------------------------------------

adController::~adController()
{
}

adController::adController()
{
}

adController::operator bool () const
{ 
	return true;
}

void
adController::_abort_server()
{
	__aborted = true;
	adController::_deactivate();
}

bool
adController::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr )
{
/*
	if ( ! orb ) {
		int ac = 0;
		orb = CORBA::ORB_init( ac, 0 );
	}
*/
	ORBServant< adcontroller::manager_i > * pServant = adcontroller::singleton::manager::instance();
    pServant->initialize( orb, poa, mgr );
/*
	ORBServantManager * pMgr = new ORBServantManager( orb );
	pMgr->init( 0, 0 );
	pServant->setServantManager( pMgr );
*/
	return true;
}

void
adController::initial_reference( const char * iorBroker )
{
    adcontroller::singleton::manager::instance()->broker_manager_ior( iorBroker );
}

const char *
adController::activate()
{
	ORBServant< adcontroller::manager_i > * pServant = adcontroller::singleton::manager::instance();
	pServant->activate();
    return pServant->ior().c_str();
}

bool
adController::deactivate()
{
	return adController::_deactivate();
}

bool
adController::_deactivate()
{
	adcontroller::singleton::iBrokerManager::instance()->manager_terminate();
    adcontroller::singleton::manager::instance()->deactivate();
	return true;
}

int
adController::run()
{
   ACE_Sched_Params fifo_sched_params( ACE_SCHED_FIFO, 
				       ACE_Sched_Params::priority_min( ACE_SCHED_FIFO ),
				       ACE_SCOPE_PROCESS );
   
   //<---- set real time priority class for this process
   if ( debug_flag == 0 ) {
	   if ( ACE_OS::sched_params(fifo_sched_params) == -1 ) {
		   if ( errno == EPERM || errno == ENOTSUP ) 
			   ACE_DEBUG((LM_DEBUG, "Warning: user's not superuser, so we'll run in the theme-shared class\n"));
		   else
			   ACE_ERROR_RETURN((LM_ERROR, "%p\n", "ACE_OS::sched_params()"), -1);
	   }
   } else {
	   std::cerr << "==================================================" << std::endl;
	   std::cerr << "====== running normal priority for debug =========" << std::endl;
	   std::cerr << "==================================================" << std::endl;
   }
   //-------------> end priority code
   return 0;
}

void
adController::abort_server()
{
	adController::_abort_server();
}

/////////////////////
__declspec(dllexport) adplugin::orbLoader * instance()
{
	return new adController;
}

