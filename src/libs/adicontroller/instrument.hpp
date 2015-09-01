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

#pragma once

#include "controlmethod.hpp"
#include "adicontroller_global.hpp"
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } }

namespace adicontroller {
    
    namespace SignalObserver { class Observer; }
    namespace SampleBroker { class SampleSequence; }
    
    class Receiver;
    
    namespace Instrument {
        
        enum eInstStatus : unsigned uint32_t {
            eNothing
            , eNotConnected             //= 0x00000001,  // no instrument := no driver software loaded
            , eOff                      //= 0x00000002,  // software driver can be controled, but hardware is currently off
            , eInitializing             //= 0x00000003,  // startup initializing (only at the begining after startup)
            , eStandBy                  //= 0x00000004,  // instrument is stand by state
            , ePreparingForRun          //= 0x00000005,  // preparing for next method (parameters being be set value)
            , eReadyForRun              //= 0x00000006,  // method is in initial state, ready to run (INIT RUN, MS HTV is ready)
            , eWaitingForContactClosure //= 0x00000007,  //
            , eRunning                  //= 0x00000008,  // method is in progress
            , eStop                     //= 0x00000009,  // stop := detector is not monitoring, pump is off
        };

#if defined _MSC_VER
        class Session;
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::weak_ptr < Session > ;
#endif
        
        class ADICONTROLLERSHARED_EXPORT Session : public std::enable_shared_from_this < Session > {

            virtual void * _narrow_workaround( const char * type_name ) { return 0; }

        public:

            Session();
            virtual ~Session();

            virtual std::shared_ptr< Session > pThis() { return shared_from_this(); }

            template<typename T> T* _narrow() {
                T* p( 0 );
                try { p = dynamic_cast<T*>( this ); } catch ( ... ) { /* ignore */ }
                if ( !p )
                    p = reinterpret_cast <T*>( _narrow_workaround( typeid( T ).name() ) );
                return p;
            }

            virtual std::string software_revision() const = 0;  // ex. L"1.216"

            virtual bool setConfiguration( const std::string& xml ) = 0 ;
            virtual bool configComplete() = 0;
            
            virtual bool connect( Receiver * receiver, const std::string& token ) = 0;
            virtual bool disconnect( Receiver * receiver ) = 0;
      
            virtual uint32_t get_status() = 0;
            virtual SignalObserver::Observer * getObserver() = 0;
      
            virtual bool initialize() = 0;

            virtual bool shutdown() = 0;  // shutdown server
            virtual bool echo( const std::string& msg ) = 0;
            virtual bool shell( const std::string& cmdline ) = 0;

            virtual std::shared_ptr< const adcontrols::ControlMethod::Method > getControlMethod() = 0;
            virtual bool prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m ) = 0;
    
            virtual bool event_out( uint32_t event ) = 0;
            virtual bool start_run() = 0;
            virtual bool suspend_run() = 0;
            virtual bool resume_run() = 0;
            virtual bool stop_run() = 0;
        };


    };
    
} // namespace adicontroler
