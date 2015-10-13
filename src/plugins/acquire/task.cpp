/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "task.hpp"
#include "iproxy.hpp"
#include "logging.hpp"
#include "masterobserver.hpp"
#include "oproxy.hpp"
#include "observer.hpp"
#include "sampleprocessor.hpp"
#include <acewrapper/udpeventreceiver.hpp>
#include <iostream>
#include <sstream>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adlog/logger.hpp>
#include <adportable/timer.hpp>
#include <acewrapper/orbservant.hpp>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/pugiwrapper.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <stdexcept>
#if defined _DEBUG
# include <iostream>
#endif

namespace acquire {

    class task::impl {
        task * this_;
    public:
        static std::unique_ptr< task > instance_;
        static std::once_flag flag;
        
        impl( task * p ) : this_( p )
                         , masterObserver_( std::make_shared< MasterObserver >() ) {
        }
        
        std::shared_ptr< MasterObserver > masterObserver_;
        
    };

    std::unique_ptr< task > task::impl::instance_ = 0;
    std::once_flag task::impl::flag;

    struct receiver_data {
//        bool operator == ( const receiver_data& ) const;
//        bool operator == ( const Receiver_ptr ) const;
//       bool operator == ( const ControlServer::Session_ptr ) const;
//        ControlServer::Session_var session_;
//        Receiver_var receiver_;
//        std::string token_;
//        size_t failed_;
//        receiver_data() : failed_( 0 ) {}
//        receiver_data( const receiver_data& t )
//            : session_(t.session_), receiver_(t.receiver_), token_( t.token_ ), failed_( t.failed_ ) {
 //       }
    };
}

using namespace acquire;

task *
task::instance()
{
    std::call_once( impl::flag, [](){ impl::instance_.reset( new task() ); } );
    return impl::instance_.get();
}

task::~task()
{
}

task::task()
{
}

adicontroller::SignalObserver::Observer *
task::masterObserver()
{
    return impl_->masterObserver_.get();
}

void
task::close()
{
}

bool
task::open()
{
    return true;
}

void
task::reset_clock()
{
}

bool
task::setConfiguration( const std::string& xml )
{
    return true;
}

bool
task::configComplete()
{
    return true;
}

bool
task::initialize()
{
    return false;
}

#if 0
bool
task::connect( ControlServer::Session_ptr session, Receiver_ptr receiver, const char * token )
{
    internal::receiver_data data;
    data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );
    data.token_ = token;
    
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( std::find(receiver_set_.begin(), receiver_set_.end(), data ) != receiver_set_.end() )
        return false;

    receiver_set_.push_back( data );
    
    Logging( L"A pair of session %1%, Receiver %2% from \"%3%\" has success connected"
             , EventLog::pri_INFO ) % static_cast< void * >( session ) % static_cast<void *>( receiver ) % token;

    return true;
}

bool
task::disconnect( ControlServer::Session_ptr session, Receiver_ptr receiver )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    do { // disconnecting proxies
        using adcontroller::iProxy;
        using adcontroller::oProxy;
        for ( auto& iproxy: iproxies_ )
            iproxy->disconnect();
        for ( auto& oproxy: oproxies_ )
            oproxy->disconnect();
    } while ( 0 );
    
    // receiver_vector_type::iterator it = std::remove( receiver_set_.begin(), receiver_set_.end(), data );
    auto it = std::remove_if( receiver_set_.begin(), receiver_set_.end(), [&](internal::receiver_data& t ){
            return t.receiver_->_is_equivalent( receiver ) && t.session_->_is_equivalent( session );
        });
    
    if ( it != receiver_set_.end() ) {
        receiver_set_.erase( it, receiver_set_.end() );
        return true;
    }
    return false;
}
#endif

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void
task::handle_observer_update_data( unsigned long parentId, unsigned long objId, long pos )
{
#if 0
    using SignalObserver::DataReadBuffer_var;

    // read data from source (actual) data observer via CORBA
    if ( DataReadBuffer_var rp = pMasterObserver_->handle_data( parentId, objId, pos ) ) {
        
        std::lock_guard< std::mutex > lock( mutex_ );

        for ( auto q: queue_ ) {
            q->pos_front( pos, objId );
            //q->strand().post( std::bind(&SampleProcessor::handle_data, q, objId, pos, rp ) ); // task::io_service
            const SignalObserver::DataReadBuffer& x = *rp;
            q->strand().post( [=] { q->handle_data( objId, pos, x ); } );
        }

    }
    pMasterObserver_->forward_observer_update_data( parentId, objId, pos );
#endif
}

void
task::handle_observer_update_method( unsigned long parentId, unsigned long objId, long pos )
{
#if 0
    pMasterObserver_->forward_observer_update_method( parentId, objId, pos );
#endif
}

void
task::handle_observer_update_events( unsigned long parentId, unsigned long objId, long pos, unsigned long events )
{
    // pMasterObserver_->forward_observer_update_events( parentId, objId, pos, events );
}


adicontroller::SignalObserver::Observer *
task::getObserver()
{
    return 0; // return master observer
}

std::shared_ptr< const SampleProcessor >
task::getCurrentSampleProcessor() const
{
//    std::lock_guard< std::mutex > lock( mutex_ );
//    if ( !queue_.empty() )
//        return queue_.front();
    return 0;
}
