//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "IBrokerManager.h"
#include "ibroker.h"
#include <boost/noncopyable.hpp>
#include <acewrapper/mutex.hpp>
#include <acewrapper/reactorthread.h>
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>
#include <acewrapper/timerhandler.h>
#include <acewrapper/eventhandler.h>
#include "message.h"
#include "constants.h"
#include "marshal.hpp"

//////////////////////////

using namespace adcontroller;

namespace adcontroller {
	namespace internal {

		class TimeReceiver {
		public:
			TimeReceiver() {}
			int handle_input( ACE_HANDLE ) { return 0; }
			int handle_timeout( const ACE_Time_Value& tv, const void * arg) {
				return singleton::iBrokerManager::instance()->handle_timeout( tv, arg );
			}
			int handle_close( ACE_HANDLE, ACE_Reactor_Mask ) { return 0; }
		};

	}
}

///////////////////////////////////////////////////////////////////

IBrokerManager::~IBrokerManager()
{
    terminate();
	ACE_Thread_Manager::instance()->wait();
	delete pBroker_;
	delete reactor_thread_;
}

IBrokerManager::IBrokerManager() : pBroker_(0)
                                 , reactor_thread_(0) 
								 , timerHandler_(0) 
{
	reactor_thread_ = new acewrapper::ReactorThread();
	acewrapper::ReactorThread::spawn( reactor_thread_ );
	pBroker_ = new iBroker( 5 );
}

bool
IBrokerManager::initialize()
{
	if ( timerHandler_ == 0 ) {
		acewrapper::scoped_mutex_t<> lock( mutex_ );
		if ( timerHandler_ == 0 ) {
			timerHandler_ = new acewrapper::EventHandler< acewrapper::TimerReceiver<internal::TimeReceiver> >();
			ACE_Reactor * reactor = singleton::iBrokerManager::instance()->reactor();
			reactor->schedule_timer( timerHandler_, 0, ACE_Time_Value(3), ACE_Time_Value(3) );
		}
		pBroker_->open();
		return true;
	}
	return false;
}

void
IBrokerManager::terminate()
{
	if ( timerHandler_ ) {
		acewrapper::scoped_mutex_t<> lock( mutex_ );
		if ( timerHandler_ ) {
            timerHandler_->cancel( reactor(), timerHandler_ );
            timerHandler_->wait();
            delete timerHandler_;
			timerHandler_ = 0;
		}
	}
	pBroker_->close();
    reactor_thread_->terminate();
}

ACE_Reactor *
IBrokerManager::reactor()
{ 
	return reactor_thread_ ? reactor_thread_->get_reactor() : 0;
}

int
IBrokerManager::handle_timeout( const ACE_Time_Value& tv, const void * )
{
    using namespace adcontroller;

	if ( pBroker_ )
        pBroker_->putq( adcontroller::marshal< ACE_Time_Value >::put( tv, constants::MB_TIME_VALUE ) );

	return 0;
}
