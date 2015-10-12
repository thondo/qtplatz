/**************************************************************************
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this 
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

#include "task.hpp"
#include "document.hpp"
#include <workaround/boost/asio.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <adportable/binary_serializer.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adicontroller/sampleprocessor.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/fwd.hpp> // matrix forward decl
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>
#include <QRect>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>
#include <thread>


namespace acquire {

    static std::once_flag flag1;
    namespace so = adicontroller::SignalObserver;

    struct data_status {
        uint32_t pos_;
        uint32_t pos_origin_;
        int device_version_;
        uint32_t posted_data_count_;
        std::atomic< bool > plot_ready_;
        std::chrono::steady_clock::time_point tp_data_handled_;
        std::chrono::steady_clock::time_point tp_plot_handled_;
        data_status() : pos_(-1), pos_origin_(0), device_version_(0), posted_data_count_(0), plot_ready_(false) {
        }
        data_status( const data_status& t ) : pos_( t.pos_ ), pos_origin_( t.pos_origin_ )
                                            , device_version_( t.device_version_ )
                                            , posted_data_count_( t.posted_data_count_ )
                                            , tp_data_handled_( t.tp_data_handled_ )
                                            , tp_plot_handled_( t.tp_plot_handled_ ) {
        }
    };
    
    class task::impl {
    public:
        impl() : worker_stopping_( false )
               , strand_( io_service_ )
               , strand2_( io_service_ )
               , work_( io_service_ )
               , traceAccessor_( std::make_shared< adcontrols::TraceAccessor >() )
               , software_inject_triggered_(false) {

        }

        static std::atomic< task * > instance_;
        static std::mutex mutex_;
            
        boost::asio::io_service io_service_;
        boost::asio::io_service::strand strand_;
        boost::asio::io_service::strand strand2_;
        boost::asio::io_service::work work_;
            
        std::vector< std::thread > threads_;
        adportable::semaphore sema_;
        std::atomic< bool > worker_stopping_;
        std::chrono::steady_clock::time_point tp_uptime_;
        std::chrono::steady_clock::time_point tp_inject_;
            
        std::map< boost::uuids::uuid, data_status > data_status_;

        std::shared_ptr< adcontrols::TraceAccessor > traceAccessor_;
        std::atomic< bool > software_inject_triggered_;
            
        QRect cellRect_;
        std::condition_variable cv_;
            
        std::deque < std::shared_ptr< adicontroller::SampleProcessor > > acquireingSamples_;
        std::deque < std::shared_ptr< adicontroller::SampleProcessor > > processingSamples_;

        void worker_thread();
        bool finalize();
        void readData( adicontroller::SignalObserver::Observer *, uint32_t pos );

        void inject_triggered() {
            tp_inject_ = std::chrono::steady_clock::now();
        }
        
        template<typename Rep, typename Period> Rep uptime() const {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>( std::chrono::steady_clock::now() - tp_uptime_ ).count();
        }

        template<typename Rep, typename Period> Rep timeSinceInject() const {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>( std::chrono::steady_clock::now() - tp_inject_ ).count();
        }
#if 0
        void initSampleProcessor() {
            auto sampleRun = std::make_shared< adcontrols::SampleRun >( *document::instance()->sampleRun() );
            auto cm = std::make_shared< adcontrols::ControlMethod::Method >( *document::instance()->getControlMethod() );
            if ( auto sp = std::make_shared< adicontroller::SampleProcessor >( io_service_, sampleRun, cm ) ) {
                //sp->prepare_storage( masterobserver );
            }
        }

        inline std::shared_ptr< mpxcontroller::SampleProcessor > getSampleProcessor() {
            std::lock_guard< std::mutex > lock( mutex_ );
            if ( !sampleProcessors_.empty() )
                return  sampleProcessors_.front();
            return 0;
        }
#endif            
    };

    std::atomic< task * > task::impl::instance_( 0 );
    std::mutex task::impl::mutex_;

}

using namespace acquire;

task::task() : impl_( new impl() )
{
}

task::~task()
{
    delete impl_;
}

bool
task::initialize()
{
    std::call_once( flag1, [=] () {
            
            impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->worker_thread(); } ) );
            
            unsigned nCores = std::max( unsigned( 3 ), std::thread::hardware_concurrency() ) - 1;
            while( nCores-- )
                impl_->threads_.push_back( adportable::asio::thread( [=] { impl_->io_service_.run(); } ) );

        } );

    return true;
}

bool
task::finalize()
{
    return impl_->finalize();
}


bool
task::impl::finalize()
{
    worker_stopping_ = true;
    sema_.signal();
    
    io_service_.stop();
    for ( auto& t : threads_ )
        t.join();
    
    return true;
}

void
task::instInitialize( adicontroller::Instrument::Session * session )
{
    ADDEBUG() << "===== instInitialize =====";

    auto self( session->shared_from_this() );
    if ( self ) {
        impl_->io_service_.post( [self] () { self->initialize(); } );
    }
}

void
task::onDataChanged( adicontroller::SignalObserver::Observer * so, uint32_t pos )
{
    // on SignalObserver::Observer masharing (sync with device data-reading thread)

    auto self( so->shared_from_this() );
    
    if ( self )
        impl_->io_service_.post( impl_->strand_.wrap( [=]{ impl_->readData( so, pos ); } ) );

}

task *
task::instance()
{
    task * tmp = impl::instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( impl::mutex_ );
        tmp = impl::instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new task();
            std::atomic_thread_fence( std::memory_order_release );
            impl::instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

void
task::post( std::vector< std::future<bool> >& futures )
{
    bool processed( false );
    static std::mutex m;
    static std::condition_variable cv;

    impl_->io_service_.post( [&] () {

            std::vector< std::future<bool> > xfutures;
            for ( auto& future : futures )
                xfutures.push_back( std::move( future ) );

            { std::lock_guard< std::mutex > lk( m ); processed = true; }  cv.notify_one();

            std::for_each( xfutures.begin(), xfutures.end(), [] ( std::future<bool>& f ) { f.get(); } );
        });

    std::unique_lock< std::mutex > lock( m );
    cv.wait( lock, [&processed] { return processed; } );
}

//////////////
void
task::impl::worker_thread()
{
    do {
        sema_.wait();
        if ( worker_stopping_ )
            return;
    } while ( true );
}

// strand::wrap
void
task::impl::readData( adicontroller::SignalObserver::Observer * so, uint32_t pos )
{
    auto self( so->shared_from_this() );

    if ( self ) {

#if 0        
        auto& status = data_status_[ so->objid() ];
        status.posted_data_count_++;

        if ( status.pos_ == uint32_t( -1 ) ) {
            status.pos_ = pos;
            status.pos_origin_ = pos;
        }
    
        if ( so->objid() == malpix_observer ) {
            std::shared_ptr< adicontroller::SignalObserver::DataReadBuffer > rb;
            do {
                if ( ( rb = so->readData( status.pos_++ ) ) )
                    handle_malpix_data( status, rb );
            } while ( rb && status.pos_ < pos );
            
        } else if ( so->objid() == ap240_observer ) {
            std::shared_ptr< adicontroller::SignalObserver::DataReadBuffer > rb;
            do {
                if ( ( rb = so->readData( status.pos_++ ) ) )
                    handle_ap240_data( status, rb );
            } while ( rb && status.pos_ <= pos );
        } else {
            std::string name = so->objtext();
            auto uuid = so->objid();
        }
#endif
    }
}

void
task::prepare_next_sample( adicontroller::SignalObserver::Observer * masterObserver
                           , std::shared_ptr< adcontrols::SampleRun > srun, const adcontrols::ControlMethod::Method& cm )
{
    auto method = std::make_shared< adcontrols::ControlMethod::Method >( cm );
    if ( auto proc = std::make_shared< adicontroller::SampleProcessor >( impl_->io_service_, srun, method ) ) {

        proc->prepare_storage( masterObserver );
        emit document::instance()->sampleRunChanged();

    }

}

