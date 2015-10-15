// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <boost/noncopyable.hpp>
#include <adportable/configuration.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <workaround/boost/asio.hpp>
#include <adportable/asio/thread.hpp>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace pugi { class xml_document; }
namespace EventLog { struct LogMessage; }
namespace adcontrols { namespace ControlMethod { class Method; } class SampleRun; }
namespace adicontroller { namespace SignalObserver { class Observer; } }
namespace acewrapper { class udpEventReceiver; }

namespace acquire {

    class iProxy;
    class oProxy;
    class observer_i;
    class iTaskManager;
    class SampleProcessor;

    namespace internal {  struct receiver_data;  }

    class task {
        ~task();
        task();

        task( const task& ) = delete;
        task& operator = ( const task& ) = delete;

        class impl;
        impl * impl_;
        friend std::unique_ptr < task >::deleter_type ;
        
    public:
        static task * instance();

        adicontroller::SignalObserver::Observer * masterObserver();
        
        bool open();
        void close();

        void post( std::vector< std::future<bool> >& futues );

	//  instrument communication methods below
        void reset_clock();
        bool initialize();  // initialize hardware 
        // bool connect( ControlServer::Session_ptr, Receiver_ptr, const char * token );
        // bool disconnect( ControlServer::Session_ptr, Receiver_ptr );
        bool setConfiguration( const std::string& xml );
        bool configComplete();
        bool initialize_configuration();
	
        //
        // ControlServer::eStatus getStatusCurrent();
        // ControlServer::eStatus getStatusBeing(); 
        bool observer_update_data( unsigned long parentId, unsigned long objid, long pos );
        bool observer_update_method( unsigned long parentId, unsigned long objid, long pos );
        bool observer_update_event( unsigned long parentId, unsigned long objid, long pos, unsigned long ev );
	
        typedef std::vector<internal::receiver_data> receiver_vector_type;
        //inline receiver_vector_type::iterator receiver_begin() { return receiver_set_.begin(); };
        //inline receiver_vector_type::iterator receiver_end()   { return receiver_set_.end(); };
        
        void register_failed( receiver_vector_type::iterator& );
        void commit_failed();
	
        adicontroller::SignalObserver::Observer * getObserver();
        
    private:
        void handle_observer_update_data( unsigned long parentId, unsigned long objId, long pos );
        void handle_observer_update_method( unsigned long parentId, unsigned long objId, long pos );
        void handle_observer_update_events( unsigned long parentId, unsigned long objId, long pos, unsigned long events );
    public:
        void notify_message( unsigned long msgid, unsigned long value );
        void handle_message( std::wstring name, unsigned long msgid, unsigned long value );
        void handle_eventlog( EventLog::LogMessage );
        void handle_echo( std::string );
        void handle_prepare_for_run( std::shared_ptr< adcontrols::ControlMethod::Method >
                                     , std::shared_ptr< adcontrols::SampleRun > );
        void handle_start_run();
        void handle_resume_run();
        void handle_stop_run();
        void handle_event_out( unsigned long value );
        void post_stop_run(); // <-- from sampleprocessor
        std::shared_ptr< const SampleProcessor > getCurrentSampleProcessor() const;
        void notify_inject( const SampleProcessor *, unsigned long objId, long pos, uint64_t usec );
	// 
    public:
        typedef std::shared_ptr< iProxy > iproxy_ptr;
        typedef std::shared_ptr< oProxy > oproxy_ptr;
        
        typedef std::vector< std::shared_ptr<iProxy> > iproxy_vector_type;
        typedef std::vector< std::shared_ptr<oProxy> > oproxy_vector_type;
        
    private:
#if 0
        adportable::Configuration config_;
        receiver_vector_type receiver_set_;
        receiver_vector_type receiver_failed_;
        
        std::vector< std::shared_ptr< iProxy > > iproxies_;
        std::vector< std::shared_ptr< oProxy > > oproxies_;
	
        std::shared_ptr< observer_i > pMasterObserver_;
	
        ::ControlServer::eStatus status_current_;
        ::ControlServer::eStatus status_being_;

		std::deque< std::shared_ptr< SampleProcessor > > queue_;

        void initiate_timer();
        void handle_timeout( const boost::system::error_code& );

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::deadline_timer timer_;
        boost::asio::io_service::strand strand_;
        boost::asio::io_service::strand strand2_;
        size_t interval_;

        std::vector< adportable::asio::thread > threads_;
        std::shared_ptr< adcontrols::ControlMethod::Method > ctrlMethod_;
        std::shared_ptr< adcontrols::SampleRun > sampleRun_;
        std::unique_ptr< acewrapper::udpEventReceiver > udpReceiver_;
#endif
    };

} // namespace adcontroller
