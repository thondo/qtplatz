/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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
#include "masterobserver.hpp"
#include "waveformobserver.hpp"
#include <ap240/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adicontroller/receiver.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <boost/asio.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>

namespace adi = adicontroller;

namespace ap240controller { namespace Instrument {

        struct Session::impl {

            impl() : work_( io_service_ )
                   , masterObserver_( std::make_shared< MasterObserver >() )
                   , waveformObserver_( std::make_shared< WaveformObserver >() ) {

                masterObserver_->addSibling( waveformObserver_.get() );

            }
            
            static std::once_flag flag_, flag2_, flag3_;
            static std::shared_ptr< Session > instance_;
            static std::mutex mutex_;

            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            std::vector< std::thread > threads_;

            typedef std::pair< std::shared_ptr< adi::Receiver >, std::string > client_pair_t;
            std::vector< client_pair_t > clients_;
            inline std::mutex& mutex() { return mutex_; }

            std::shared_ptr< ap240::digitizer > digitizer_;
            std::shared_ptr< MasterObserver > masterObserver_;
            std::shared_ptr< WaveformObserver > waveformObserver_;
            
            void reply_message( adi::Receiver::eINSTEVENT msg, uint32_t value ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                for ( auto& r: clients_ )
                    r.first->message( msg, value );
            }

            void reply_handler( const std::string& method, const std::string& reply) {
                if ( method == "InitialSetup" )
                    reply_message( adi::Receiver::STATE_CHANGED, ( reply == "success" ) ? adi::Instrument::eStandBy : adi::Instrument::eOff );
                ADDEBUG() << "===== ap240 reply ===== " << method << " reply: " << reply;
            }
            
            bool waveform_handler( const ap240controls::waveform * ch1, const ap240controls::waveform * ch2, ap240controls::method& next ) {
                if ( masterObserver_ && waveformObserver_ ) {
                    if ( ch1 || ch2 ) {
                        auto pair = std::make_pair( ( ch1 ? ch1->shared_from_this() : 0 ), ( ch2 ? ch2->shared_from_this() : 0 ) );
                        auto pos = (*waveformObserver_) << pair;
                        masterObserver_->dataChanged( waveformObserver_.get(), pos );
                        return false; // no next method changed.
                    }
                }
                return false;
            }
        };

        std::once_flag Session::impl::flag_;
        std::once_flag Session::impl::flag2_;
        std::once_flag Session::impl::flag3_;
        std::mutex Session::impl::mutex_;
        std::shared_ptr< Session > Session::impl::instance_;
        
    }
}

using namespace ap240controller::Instrument;

Session *
Session::instance()
{
    std::call_once( impl::flag_, [&] () { impl::instance_ = std::make_shared< Session >(); } );
    return impl::instance_.get();
}

Session::Session() : impl_( new impl() )
{
    ADDEBUG() << "##### Session ctor #####";
}

Session::~Session()
{
    delete impl_;
}

std::string
Session::software_revision() const
{
    return "3.2";
}

bool
Session::setConfiguration( const std::string& xml )
{
    ADDEBUG() << "##### Session::setConfiguration #####";
    return true;
}

bool
Session::configComplete()
{
    ADDEBUG() << "##### Session::configComplete #####";    
    return true;    
}
            
bool
Session::connect( adi::Receiver * receiver, const std::string& token )
{
    ADDEBUG() << "##### Session::connect token=" << token << " ######";
    
    auto ptr( receiver->shared_from_this() );
    if ( ptr ) {
        std::call_once( impl::flag2_, [&] () {
                impl_->threads_.push_back( adportable::asio::thread( [=]() { impl_->io_service_.run(); } ) );
            });
        
        do {
            std::lock_guard< std::mutex > lock( impl_->mutex() );
            impl_->clients_.push_back( std::make_pair( ptr, token ) );
        } while ( 0 );

        impl_->io_service_.post( [this] () { impl_->reply_message( adi::Receiver::CLIENT_ATTACHED, uint32_t( impl_->clients_.size() ) ); } );

        return true;
    }
    return false;
}

bool
Session::disconnect( adicontroller::Receiver * receiver )
{
    ADDEBUG() << "##### Session::disconnect #####";
    
    auto self( receiver->shared_from_this() );
    
    std::lock_guard< std::mutex > lock( impl_->mutex() );
    auto it = std::find_if( impl_->clients_.begin(), impl_->clients_.end(), [self]( const impl::client_pair_t& a ){
            return a.first == self; });

    if ( it != impl_->clients_.end() ) {
        impl_->clients_.erase( it );
        return true;
    }
    
    return false;
}
      
uint32_t
Session::get_status()
{
    return 0;
}

adicontroller::SignalObserver::Observer *
Session::getObserver()
{
    ADDEBUG() << "##### Session::getObserver #####";
    return impl_->masterObserver_.get();
}
      
bool
Session::initialize()
{
    ADDEBUG() << "##### Session::initialize #####";    
    std::call_once( impl::flag3_, [&] () {
            std::lock_guard< std::mutex > lock( impl::mutex_ );
            impl_->digitizer_ = std::make_shared< ap240::digitizer >();
            using namespace std::placeholders;
            impl_->digitizer_->connect_reply( std::bind( &impl::reply_handler, impl_, _1, _2 ) );
            impl_->digitizer_->connect_waveform( std::bind( &impl::waveform_handler, impl_, _1, _2, _3 ) );
        } );
    return impl_->digitizer_->peripheral_initialize();
}

bool
Session::shutdown()
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    return impl_->digitizer_ && impl_->digitizer_->peripheral_terminate();
}

bool
Session::echo( const std::string& msg )
{
    return false;
}

bool
Session::shell( const std::string& cmdline )
{
    return false;    
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
Session::getControlMethod()
{
    return 0; // adicontroller::ControlMethod::Method();
}

bool
Session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    ADDEBUG() << "##### Session::prepare_for_run #####";
    if ( m ) {
        auto it = m->find( m->begin(), m->end(), "ap240" );
        if ( it != m->end() ) {
            ap240controls::method method;
            if ( it->get<>( *it, method ) )
                return impl_->digitizer_->peripheral_prepare_for_run( method );
        }
    }
    return false;
}
    
bool
Session::event_out( uint32_t event )
{
    return true;
}

bool
Session::start_run()
{
    return true;
}

bool
Session::suspend_run()
{
    return true;
}

bool
Session::resume_run()
{
    return true;
}

bool
Session::stop_run()
{
    return true;
}
