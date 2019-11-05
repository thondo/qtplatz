/**************************************************************************
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "session.hpp"
#include "singleton.hpp"
#include "waveformobserver.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adacquire/masterobserver.hpp>
#include <adacquire/receiver.hpp>
#include <adlog/logger.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>
#include <string>

namespace aqmd3 {

    struct session::impl {

        impl() : work_( io_service_ )
                 //, masterObserver_( std::make_shared< adacquire::MasterObserver >( "aqmd3.master.observer.ms-cheminfo.com" ) )
                 //, waveformObserver_( std::make_shared< WaveformObserver >() ) {
            {
                // masterObserver_->addSibling( waveformObserver_.get() );
        }

        static std::shared_ptr< session > instance_;

        std::mutex mutex_;

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        std::vector< std::thread > threads_;

        typedef std::pair< std::shared_ptr< adacquire::Receiver >, std::string > client_pair_t;
        std::vector< client_pair_t > clients_;

        //std::shared_ptr< adacquire::MasterObserver > masterObserver_;
        //std::shared_ptr< WaveformObserver > waveformObserver_;
        std::pair< std::string, std::string > server_address_;

        void reply_message( adacquire::Receiver::eINSTEVENT msg, uint32_t value ) {
            std::lock_guard< std::mutex > lock( mutex_ );
            for ( auto& r: clients_ )
                r.first->message( msg, value );
        }

        void reply_handler( const std::string& method, const std::string& reply) {
            if ( method == "InitialSetup" ) {
                reply_message( adacquire::Receiver::STATE_CHANGED
                               , ( reply == "success" ) ? adacquire::Instrument::eStandBy : adacquire::Instrument::eNotConnected | adacquire::Instrument::eErrorFlag );
            } else if ( method == "StateChanged" ) {
                if ( reply == "Stopped" )
                    reply_message( adacquire::Receiver::STATE_CHANGED, adacquire::Instrument::eStop ); // 9
                else if ( reply == "Running" )
                    reply_message( adacquire::Receiver::STATE_CHANGED, adacquire::Instrument::eRunning ); // 8
            } else if ( method == "DarkStarted" ) {
                reply_message( adacquire::Receiver::DARK_STARTED, 1 );
            } else if ( method == "DarkCanceled" ) {
                reply_message( adacquire::Receiver::DARK_CANCELED, 0 );
            } else if ( method == "DarkAcquired" ) {
                reply_message( adacquire::Receiver::DARK_ACQUIRED, 0 );
            } else {
                ADINFO() << "AQMD3: " << method << " = " << reply;
            }
        }

        // bool waveform_handler( const waveform * ch1, acqrscontrols::aqmd3::method& next ) {
        //     if ( masterObserver_ && waveformObserver_ ) {
        //         if ( ch1 ) {
        //             //auto pos = (*waveformObserver_) << ch1;
        //             //masterObserver_->dataChanged( waveformObserver_.get(), pos );
        //             return false; // no next method changed.
        //         }
        //     }
        //     return false;
        // }
    };

    std::shared_ptr< session > session::impl::instance_ = 0;
}

using namespace aqmd3;

session *
session::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [&]{ impl::instance_ = std::make_shared< session >(); } );
    return impl::instance_.get();
}

session::session() : impl_( new impl() )
{
}

session::~session()
{
    delete impl_;
}

std::string
session::software_revision() const
{
    return "3.2";
}

bool
session::setConfiguration( const std::string& json )
{
    boost::property_tree::ptree pt;
    std::istringstream in( json );
    boost::property_tree::read_json( in, pt );

    // if ( auto ip_addr = pt.get_optional< std::string >( "ip_address" ) ) {
    //     if ( auto port = pt.get_optional< std::string >( "port" ) ) {
    //         impl_->server_address_ = std::make_pair( ip_addr.get(), port.get() );
    //         ADDEBUG() << impl_->server_address_;
    //         singleton::instance()->open( impl_->server_address_.first.c_str(), impl_->server_address_.second.c_str() );
    //     }
    // }

    return true;
}

const char *
session::configuration() const
{
    return nullptr;
}

bool
session::configComplete()
{
    return true;
}

bool
session::connect( adacquire::Receiver * receiver, const std::string& token )
{
    auto ptr( receiver->shared_from_this() );

    if ( ptr ) {
        impl_->clients_.emplace_back( ptr, token );
        // singleton::instance()->connect( ptr );
        return true;
    }
    return false;
}

bool
session::disconnect( adacquire::Receiver * receiver )
{
    auto self( receiver->shared_from_this() );
    // singleton::instance()->connect( self );

    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    auto it = std::find_if( impl_->clients_.begin(), impl_->clients_.end(), [self]( const impl::client_pair_t& a ){
            return a.first == self; });

    if ( it != impl_->clients_.end() ) {
        impl_->clients_.erase( it );
        return true;
    }

    return false;
}

uint32_t
session::get_status()
{
    return 0;
}

adacquire::SignalObserver::Observer *
session::getObserver()
{
    return singleton::instance()->getObserver();
}

bool
session::initialize()
{
    return true;
}

bool
session::shutdown()
{
    singleton::instance()->close();
    return true;
}

bool
session::echo( const std::string& msg )
{
    return false;
}

bool
session::shell( const std::string& cmdline )
{
    return false;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
session::getControlMethod()
{
    return nullptr; // adacquire::ControlMethod::Method();
}

bool
session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > )
{
    return false;
}

bool
session::prepare_for_run( const std::string& json, arg_type atype )
{
    if ( atype != arg_json )
        return false;
    return true;
}

bool
session::event_out( uint32_t event )
{
    ADDEBUG() << "##### session::event_out( " << event << " )";
    //return impl_->digitizer_->peripheral_trigger_inject();
    return true;
}

bool
session::start_run()
{
    return true; // impl_->digitizer_->peripheral_run();
}

bool
session::suspend_run()
{
    return true;
}

bool
session::resume_run()
{
    return true;
}

bool
session::stop_run()
{
    return true; // impl_->digitizer_->peripheral_stop();
}

bool
session::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                             , adcontrols::ControlMethod::const_time_event_iterator begin
                             , adcontrols::ControlMethod::const_time_event_iterator end )
{
#ifndef NDEBUG
    ADDEBUG() << "TODO -- time_event_triger";
#endif
    return true;
}


bool
session::dark_run( size_t waitCount )
{
    return true; // impl_->digitizer_->peripheral_dark( waitCount );
}
