/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "digitizer.hpp"
#include "simulator.hpp"
#include "sampleprocessor.hpp"
#include <adportable/string.hpp>
#include "safearray.hpp"
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>
#include <atomic>

#import "GlobMgr.dll"          no_namespace // VISA-COM I/O functionality
#import "IviDriverTypeLib.dll" no_namespace
#import "AgMD2.dll" no_namespace

#ifdef ERR
# undef ERR
#endif
#ifdef TERR
# undef TERR
#endif

#define ERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_ERROR)<<e.Description()<<", "<<e.ErrorMessage(); error_reply(e,m); } while(0)
#define TERR(e,m) do { adlog::logger(__FILE__,__LINE__,adlog::LOG_ERROR)<<e.Description()<<", "<<e.ErrorMessage(); task.error_reply(e,m); } while(0)

namespace u5303a {

    class simulator;

    namespace detail {

        enum DeviceType { Simulate, UserFDK };

        template<DeviceType> struct device {
            static bool initial_setup( task&, const method& );
            static bool setup( task&, const method& );
            static bool acquire( task& );
            static bool waitForEndOfAcquisition( task&, int timeout );
            static bool readData( task&, waveform& );
        };

        class task {
            task();
            ~task();
        public:
            static task * instance();
            
            inline boost::asio::io_service& io_service() { return io_service_; }
            
            void terminate();
            bool initialize();
            // bool prepare_for_run( const adcontrols::ControlMethod& );
            bool prepare_for_run( const u5303a::method& );
            bool run();
            bool stop();
            bool trigger_inject_out();

            void connect( digitizer::command_reply_type f );
            void disconnect( digitizer::command_reply_type f );
            void connect( digitizer::waveform_reply_type f );
            void disconnect( digitizer::waveform_reply_type f );
            void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr );

            inline IAgMD2Ex2Ptr& spDriver() { return spDriver_; }
            inline simulator * simulator() { return simulator_; }
            inline const method& method() const { return method_; }
            inline const identify& ident() const { return *ident_; }
            void error_reply( const _com_error& e, const std::string& );

        private:
            static task * instance_;
            static std::mutex mutex_;

            IAgMD2Ex2Ptr spDriver_;
            std::vector< adportable::asio::thread > threads_;
            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            boost::asio::io_service::strand strand_;
            bool simulated_;
            u5303a::method method_;
            u5303a::simulator * simulator_;
            uint32_t serialnumber_;
            std::atomic<int> acquire_post_count_;
            uint64_t inject_timepoint_;

            std::deque< std::shared_ptr< SampleProcessor > > queue_;
            std::vector< digitizer::command_reply_type > reply_handlers_;
            std::vector< digitizer::waveform_reply_type > waveform_handlers_;
            std::shared_ptr< identify > ident_;
            std::shared_ptr< adportable::TimeSquaredScanLaw > scanlaw_;

            bool handle_initial_setup( int nDelay, int nSamples, int nAverage );
            bool handle_terminating();
            bool handle_acquire();
            bool handle_prepare_for_run( const u5303a::method );
            bool handle_protocol( const u5303a::method );            
            bool acquire();
            bool waitForEndOfAcquisition( int timeout );
            bool readData( waveform& );
        };

    }
}

using namespace u5303a;
using namespace u5303a::detail;

task * task::instance_ = 0;
std::mutex task::mutex_;


digitizer::digitizer()
{
}

digitizer::~digitizer()
{
}

bool
digitizer::peripheral_terminate()
{
    task::instance()->terminate();
    return true;
}

bool
digitizer::peripheral_prepare_for_run( const adcontrols::ControlMethod& m )
{
    using adcontrols::controlmethod::MethodItem;

    adcontrols::ControlMethod cm( m );
    cm.sort();
    auto it = std::find_if( cm.begin(), cm.end(), [] ( const MethodItem& mi ){ return mi.modelname() == "u5303a"; } );
    if ( it != cm.end() ) {
        u5303a::method m;
        if ( adportable::serializer< u5303a::method >::deserialize( m, it->data(), it->size() ) ) {
            return task::instance()->prepare_for_run( m );
        }
    }
    return false;
}

bool
digitizer::peripheral_prepare_for_run( const u5303a::method& m )
{
    return task::instance()->prepare_for_run( m );
}

bool
digitizer::peripheral_initialize()
{
    return task::instance()->initialize();
}

bool
digitizer::peripheral_run()
{
    return task::instance()->run();
}

bool
digitizer::peripheral_stop()
{
    return task::instance()->stop();
}

bool
digitizer::peripheral_trigger_inject()
{
    return task::instance()->trigger_inject_out();
}

void
digitizer::connect_reply( command_reply_type f )
{
    task::instance()->connect( f );
}

void
digitizer::disconnect_reply( command_reply_type f )
{
    task::instance()->disconnect( f );
}

void
digitizer::connect_waveform( waveform_reply_type f )
{
    task::instance()->connect( f );
}

void
digitizer::disconnect_waveform( waveform_reply_type f )
{
    task::instance()->disconnect( f );
}

void
digitizer::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > ptr )
{
    task::instance()->setScanLaw( ptr );
}

////////////////////

task::task() : work_( io_service_ )
             , strand_( io_service_ )
             , simulated_( false )
             , simulator_( 0 )
             , serialnumber_( 0 )
             , acquire_post_count_( 0 )
{
    threads_.push_back( adportable::asio::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
    io_service_.post( strand_.wrap( [&] { ::CoInitialize( 0 ); } ) );
    io_service_.post( strand_.wrap( [&] { spDriver_.CreateInstance(__uuidof(AgMD2)); } ) );
}

task::~task()
{
    delete simulator_;
}

task *
task::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new task();
    }
    return instance_;
}

bool
task::initialize()
{
    ADTRACE() << "u5303a digitizer initializing...";
    io_service_.post( strand_.wrap( [&] { handle_initial_setup( 32, 1024 * 10, 2 ); } ) );
    return true;
}

bool
task::prepare_for_run( const u5303a::method& m )
{
    ADTRACE() << "u5303a::task::prepare_for_run";
    ADTRACE() << "\tfront_end_range: " << m.front_end_range << "\tfrontend_offset: " << m.front_end_offset
              << "\text_trigger_level: " << m.ext_trigger_level
              << "\tsamp_rate: " << m.samp_rate
              << "\tnbr_of_samples: " << m.nbr_of_s_to_acquire
              << "\tnbr_of_average: " << m.nbr_of_averages
              << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.delay_to_first_sample )
              << "\tinvert_signal: " << m.invert_signal
              << "\tnsa: " << m.nsa;
    
    io_service_.post( strand_.wrap( [&] { handle_prepare_for_run(m); } ) );
    if ( acquire_post_count_ == 0 ) {
        acquire_post_count_++;
        io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );
	}
    return true;
}


bool
task::run()
{
    // std::lock_guard< std::mutex > lock( mutex_ );
	if ( queue_.empty() ) {
        queue_.push_back( std::make_shared< SampleProcessor >( io_service_ ) );
        queue_.back()->prepare_storage( 0 ); //pMasterObserver_->_this() );
    }
	// status_current_ = ControlServer::ePreparingForRun;
	// status_being_ = ControlServer::eReadyForRun;
    return true;
}

bool
task::stop()
{
    return true;
}

bool
task::trigger_inject_out()
{
    return true;
}

void
task::terminate()
{
    io_service_.post( [&] { ::CoUninitialize(); } );
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();
    threads_.clear();
}


///////////////////////////////////////
         //// Parameters ////
//////////////////////////////////////
//////////////////////////////////////

bool
task::handle_initial_setup( int nDelay, int nSamples, int nAverage )
{
	(void)nAverage;
	(void)nSamples;
	(void)nDelay;
	// u5303a::method m;
	BSTR strResourceDesc = bstr_t(L"PXI4::0::0::INSTR");
    
    // If desired, use 'DriverSetup= CAL=0' to prevent digitizer from doing a SelfCal (~1 seconds) each time
    // it is initialized or reset which is the default behavior. By default set to false.
    // CUSTOM FIRMWARE NAME: U5303ADPULX2AVE.bit

    // note that CAL=0 is not necessary with MD2 as the card is initialized without calibration.
				
    VARIANT_BOOL idQuery = VARIANT_TRUE;
    VARIANT_BOOL reset   = VARIANT_TRUE;

    simulated_ = false;
    bool success = false;
    try {
        BSTR strInitOptions = _bstr_t( L"Simulate=false, DriverSetup= Model=U5303A" ); // <-- this file does not exist
        success = spDriver_->Initialize( strResourceDesc, idQuery, reset, strInitOptions ) == S_OK;
    } catch ( _com_error & e ) {
        ERR(e,"Initialize");
    }
    
    if ( !success ) {
        try {
            BSTR strInitOptions = _bstr_t( L"Simulate=true, DriverSetup= Model=U5303A, Trace=false" );
            success = spDriver_->Initialize( strResourceDesc, idQuery, reset, strInitOptions ) == S_OK;
            simulated_ = true;
            simulator_ = new u5303a::simulator();
            // threads for waveform generation
            threads_.push_back( adportable::asio::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
            threads_.push_back( adportable::asio::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
        } catch ( _com_error & e ) {
            ERR( e, "Initialize" );
        }
    }

    if ( success ) {
        
        ident_ = std::make_shared< identify >();

        try {
            ident_->Identifier = static_cast<const char *>( _bstr_t( spDriver_->Identity->Identifier.GetBSTR() ) );
            ident_->Revision = static_cast<const char *>( _bstr_t( spDriver_->Identity->Revision.GetBSTR() ) );
            ident_->Vendor = static_cast<const char *>( _bstr_t( spDriver_->Identity->Vendor.GetBSTR() ) );
            ident_->Description = static_cast<const char *>( _bstr_t( spDriver_->Identity->Description.GetBSTR() ) );
            ident_->InstrumentModel = static_cast<const char *>( _bstr_t( spDriver_->Identity->InstrumentModel.GetBSTR() ) );
            ident_->FirmwareRevision = static_cast<const char *>( _bstr_t( spDriver_->Identity->InstrumentFirmwareRevision.GetBSTR() ) );
            if ( auto iinfo = spDriver_->InstrumentInfo ) {
                ident_->SerialNumber = iinfo->SerialNumberString;
                ident_->IOVersion = iinfo->IOVersion;
                ident_->Options = iinfo->Options;
                ident_->NbrADCBits = iinfo->NbrADCBits;
            }
        } catch ( _com_error& ex ) {
            ERR( ex, "Identification failed." );
        }

        for ( auto& reply: reply_handlers_ ) reply( "Identifier", ident_->Identifier );
        for ( auto& reply: reply_handlers_ ) reply( "Revision", ident_->Revision );
        for ( auto& reply: reply_handlers_ ) reply( "Description", ident_->Description );
        for ( auto& reply: reply_handlers_ ) reply( "InstrumentModel", ident_->InstrumentModel );
        for ( auto& reply: reply_handlers_ ) reply( "InstrumentFirmwareRevision", ident_->FirmwareRevision );
        for ( auto& reply: reply_handlers_ ) reply( "SerialNumber", ident_->SerialNumber );
        for ( auto& reply: reply_handlers_ ) reply( "IOVersion", ident_->IOVersion );
        for ( auto& reply: reply_handlers_ ) reply( "Options", ident_->Options );

        if ( simulated_ )
            device<Simulate>::initial_setup( *this, method_ );
        else
            device<UserFDK>::initial_setup( *this, method_ );
    }
    for ( auto& reply: reply_handlers_ )
        reply( "InitialSetup", ( success ? "success" : "failed" ) );
	return success;
}

bool
task::handle_terminating()
{
	return false;
}

bool
task::handle_prepare_for_run( const u5303a::method m )
{
    if ( simulated_ )
        device<Simulate>::initial_setup( *this, m );
    else
        device<UserFDK>::initial_setup( *this, m );
    method_ = m;
    return true;
}

bool
task::handle_protocol( const u5303a::method m )
{
#if defined _DEBUG
    ADTRACE() << "u5303a::task::handle_protocol"
              << "\tnbr_of_samples: " << m.nbr_of_s_to_acquire
              << "\tnbr_of_average: " << m.nbr_of_averages
              << "\tdelay_to_first_s: " << adcontrols::metric::scale_to_micro( m.delay_to_first_sample );
#endif
    if ( simulated_ )
        device<Simulate>::setup( *this, m );
    else
        device<UserFDK>::setup( *this, m );
    method_ = m;
    return true;
}


bool
task::handle_acquire()
{
    static int counter_;

    ++acquire_post_count_;
    io_service_.post( strand_.wrap( [&] { handle_acquire(); } ) );    // scedule for next acquire

    --acquire_post_count_;
    if ( acquire() ) {
        if ( waitForEndOfAcquisition( 3000 ) ) {
            auto avgr = std::make_shared< waveform >( ident_ );
            if ( readData( *avgr ) ) {
                // if ( software_events_ ) {
                //     avgr->wellKnownEvents |= software_events_; // marge with hardware events
                //     software_events_ = 0;

                //     // set time for injection to zero (a.k.a. retention time)
                //     if ( avgr->wellKnownEvents & SignalObserver::wkEvent_INJECT ) {
                //         averager_inject_usec_ = avgr->uptime;
                //         avgr->timeSinceInject = 0;
                //     }
                // }
                // assert( avgr->nbrSamples );
                u5303a::method m;
                for ( auto& reply: waveform_handlers_ ) {
                    if ( reply( avgr.get(), m ) )
                        handle_protocol( m );
                }
            }
        } else {
            ADTRACE() << "===== handle_acquire waitForEndOfAcquisitioon == not handled.";
        }
        return true;
    }
    return false;
}

bool
task::acquire()
{
    if ( simulated_ )
        return device<Simulate>::acquire( *this );
    else
        return device<UserFDK>::acquire( *this );
}

bool
task::waitForEndOfAcquisition( int timeout )
{
    if ( simulated_ )
        return device<Simulate>::waitForEndOfAcquisition( *this, timeout );
    else
        return device<UserFDK>::waitForEndOfAcquisition( *this, timeout );
}

bool
task::readData( waveform& data )
{
    data.serialnumber = serialnumber_++;
    if ( simulated_ )
        return device<Simulate>::readData( *this, data );
    else
        return device<UserFDK>::readData( *this, data );
}

void
task::connect( digitizer::command_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    reply_handlers_.push_back( f );
}

void
task::disconnect( digitizer::command_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );    
//	auto it = std::remove_if( reply_handlers_.begin(), reply_handlers_.end(), [=]( const digitizer::command_reply_type& t ){
//            return t == f;
//        });
//    reply_handlers_.erase( it, reply_handlers_.end() );
}

void
task::connect( digitizer::waveform_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    waveform_handlers_.push_back( f );
}

void
task::disconnect( digitizer::waveform_reply_type f )
{
    std::lock_guard< std::mutex > lock( mutex_ );    
//	auto it = std::remove_if( waveform_handlers_.begin(), waveform_handlers_.end(), [=]( const digitizer::waveform_reply_type& t ){
//            return t == f;
//        });
//    waveform_handlers_.erase( it, waveform_handlers_.end() );
}

void
task::error_reply( const _com_error& e, const std::string& method )
{
    _bstr_t msg( _bstr_t(L"Error: ") + e.Description() + L": " + e.ErrorMessage() );
    for ( auto& reply: reply_handlers_ )
        reply( method, static_cast< const char *>( msg ) );
}

void
task::setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw >& ptr )
{
    scanlaw_ = ptr;
}

///
identify::identify()
{
}

identify::identify( const identify& t ) : Identifier( t.Identifier )
                                        , Revision( t.Revision )
                                        , Vendor( t.Vendor )
                                        , Description( t.Description )
                                        , InstrumentModel( t.InstrumentModel )
                                        , FirmwareRevision( t.FirmwareRevision )
                                        , SerialNumber( t.SerialNumber )
                                        , Options( t.Options )
                                        , IOVersion( t.IOVersion )
                                        , NbrADCBits( t.NbrADCBits )
{
}

template<> bool
device<UserFDK>::initial_setup( task& task, const method& m )
{
    //IAgMD2LogicDevicePtr spDpuA = task.spDriver()->LogicDevices->Item[L"DpuA"];	
    //IAgMD2LogicDeviceMemoryBankPtr spDDR3A = spDpuA->MemoryBanks->Item[L"DDR3A"];
    //IAgMD2LogicDeviceMemoryBankPtr spDDR3B = spDpuA->MemoryBanks->Item[L"DDR3B"];
	
    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spDriver()->Channels->Item[L"Channel1"];
    
    // Set Interleave ON
#if 0
    try {
        spCh1->TimeInterleavedChannelList = "Channel2";
    } catch ( _com_error& e ) {
        TERR(e, "TimeInterleavedChannelList");
    }
#endif
    try {
        spCh1->PutRange(m.front_end_range);
    } catch ( _com_error& e ) {
        TERR(e, "Range");
    }
    try {
        spCh1->PutOffset(m.front_end_offset);
    } catch ( _com_error& e ) {
        TERR(e, "Offset");
    }
	try {
		task.spDriver()->Channels2->Item2[L"Channel1"]->DataInversionEnabled = m.invert_signal ? VARIANT_TRUE : VARIANT_FALSE;
	} catch (_com_error&) {

	}

    // Setup triggering
    try {
        task.spDriver()->Trigger->ActiveSource = "External1";
    } catch ( _com_error& e ) {
        TERR(e, "Trigger::ActiveSource");
    }
    try {
        task.spDriver()->Trigger->Delay = m.delay_to_first_sample;
    } catch ( _com_error& e ) {
        TERR(e,"mode");        
    }
    IAgMD2TriggerSourcePtr spTrigSrc = task.spDriver()->Trigger->Sources->Item[ L"External1" ];
    try {
		spTrigSrc->Level = m.ext_trigger_level;
	} catch ( _com_error& e ) {
		TERR(e, "TriggerSource::Level");
	}
    try {
		spTrigSrc->Edge->Slope = AgMD2TriggerSlopePositive;
	} catch ( _com_error& e ) {
		TERR(e, "TriggerSource::Level");
	}

        
    // Calibrate
    ADTRACE() << "Calibrating...";
    try {
        task.spDriver()->Calibration->SelfCalibrate();
    } catch ( _com_error& e ) {
        TERR(e, "Calibration::SelfCalibrate");
    }

    ADTRACE() << "Set the Mode FDK...";
    //try { task.spDriver()->Acquisition->Mode = AgMD2AcquisitionModeUserFDK; } catch (_com_error& e) { TERR(e,"Acquisition::Mode"); }
    try { task.spDriver()->Acquisition2->Mode = AgMD2AcquisitionModeAverager; } catch (_com_error& e) { TERR(e,"Acquisition::Mode"); }

    // Set the sample rate and nbr of samples to acquire
    bool success = false;
    const double sample_rate = 1.0e9; //m.samp_rate; // 3.2E9;
    try {
        task.spDriver()->Acquisition2->SampleRate = sample_rate;
        success = true;
    }  catch (_com_error& e) {
        TERR(e,"SampleRate");
    }
    if ( !success ) {
        try {
            task.spDriver()->Acquisition2->SampleRate = 1.0E9;
            success = true;
        } catch ( _com_error& e ) {
            TERR( e, "SampleRate" );
        }
    }

    // Start on trigger
    // Full bandwidth
#if 0
    try {
        task.spDriver()->Channels2->GetItem2("Channel1")->Filter->Bypass = 1;
    } catch ( _com_error& e ) {
        TERR( e, "Bandwidth" );
    }
#endif
    //--->
    try {
        task.spDriver()->Acquisition->RecordSize = m.nbr_of_s_to_acquire;
    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_s_to_acquire");        
    }
    try {
        task.spDriver()->Acquisition2->NumberOfAverages = m.nbr_of_averages;
    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_averages");        
    }
    try {
        task.spDriver()->Acquisition2->Mode = AgMD2AcquisitionModeAverager;
    } catch ( _com_error& e ) {
        TERR(e,"mode");        
    }
    //<---
    try {
        task.spDriver()->Calibration->SelfCalibrate();
    } catch ( _com_error& e ) {
        TERR(e, "Calibration::SelfCalibrate");
    }

	return true;
}

template<> bool
device<UserFDK>::setup( task& task, const method& m )
{
    IAgMD2ChannelPtr spCh1 = task.spDriver()->Channels->Item[L"Channel1"];
#if 0
    try {
        uint64_t nDelay = uint64_t( m.delay_to_first_sample / 1.0e-9 + 0.5 ); // ns
        task.spDriver()->Trigger->Delay = double( ( nDelay / 16 ) * 16 ) * 1.0e-9;
    } catch ( _com_error& e ) {
        TERR(e,"mode");        
    }
    // task.spDriver()->Calibration->SelfCalibrate();
    try {
        task.spDriver()->Acquisition->RecordSize = m.nbr_of_s_to_acquire;
    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_s_to_acquire");        
    }
    try {
        task.spDriver()->Acquisition2->NumberOfAverages = m.nbr_of_averages;
    } catch ( _com_error& e ) {
        TERR(e,"mbr_of_averages");        
    }
#endif
    return true; //device<UserFDK>::initial_setup( task, m );
}

template<> bool
device<UserFDK>::acquire( task& task )
{
    // Perform the acquisition.
    try {
        task.spDriver()->Acquisition->Initiate();
    } catch ( _com_error & e ) {
        TERR(e,"Initialte");
    } 
    //Start the acquisition
    // try { task.spDriver()->Acquisition->UserControl->StartSegmentation(); } catch ( _com_error& e ) { TERR(e, "StartSegmentation"); }
    // try { task.spDriver()->Acquisition->UserControl->StartProcessing(AgMD2UserControlProcessingType1); } catch ( _com_error& e ) {
    //     TERR( e, "StartProcessing" ); }
    return true;
}

template<> bool
device<UserFDK>::waitForEndOfAcquisition( task& task, int timeout )
{
	(void)timeout;
    //Wait for the end of the acquisition

    long const timeoutInMs = 3000;

    // long wait_for_end = 0x80000000;
    // IAgMD2LogicDevicePtr spDpuA = task.spDriver()->LogicDevices->Item[L"DpuA"];	

    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
    try {
        task.spDriver()->Acquisition->WaitForAcquisitionComplete(timeoutInMs);
    } catch ( _com_error& e ) {
        TERR(e, "WaitForAcquisitionComplete");
    }
    return true;
}

template<> bool
device<UserFDK>::readData( task& task, waveform& data )
{
    IAgMD2Channel2Ptr spCh1 = task.spDriver()->Channels2->Item2[ L"Channel1" ];
    __int64 firstRecord = 0;
    __int64 numRecords = 1;
    __int64 offsetWithinRecord = 0;
    SAFEARRAY* dataArray = 0;
    long actualAverages = 0;
    // __int64 actualRecords = 0;
    SAFEARRAY* actualPoints = 0;
    SAFEARRAY* firstValidPoint = 0;
    SAFEARRAY* initialXTimeSeconds = 0;
    SAFEARRAY* initialXTimeFraction = 0;
    SAFEARRAY* flags = 0;
    const int64_t numPointsPerRecord = task.method().nbr_of_s_to_acquire;
    
    try {
        spCh1->Measurement2->FetchAccumulatedWaveformInt32( firstRecord
                                                            , numRecords
                                                            , offsetWithinRecord
                                                            , numPointsPerRecord
                                                            , &dataArray
                                                            , &actualAverages
                                                            , &data.meta.actualRecords
                                                            , &actualPoints
                                                            , &firstValidPoint
                                                            , &data.meta.initialXOffset
                                                            , &initialXTimeSeconds
                                                            , &initialXTimeFraction
                                                            , &data.meta.xIncrement
                                                            , &data.meta.scaleFactor
                                                            , &data.meta.scaleOffset
                                                            , &flags );

        data.method_ = task.method();

        data.meta.actualAverages = actualAverages;

		safearray_t<__int64> saFirstValidPoint( firstValidPoint );
        data.meta.firstValidPoint = saFirstValidPoint.data()[ 0 ];

        safearray_t< double > saInitialXTimeSeconds( initialXTimeSeconds );
        safearray_t< double > saInitialXTimeFraction( initialXTimeFraction );
        data.meta.initialXTimeSeconds = saInitialXTimeSeconds.data()[ 0 ] + saInitialXTimeFraction.data()[ 0 ];

		safearray_t<int32_t> sa( dataArray );
        data.d_.resize( numPointsPerRecord );
        std::copy( sa.data() + data.meta.firstValidPoint, sa.data() + numPointsPerRecord, data.d_.begin() );
        data.meta.firstValidPoint = 0;

        // Release memory.
        SafeArrayDestroy(flags);
        SafeArrayDestroy(initialXTimeFraction);
        SafeArrayDestroy(initialXTimeSeconds);
        SafeArrayDestroy(firstValidPoint);
        SafeArrayDestroy(actualPoints);
        SafeArrayDestroy(dataArray);

    } catch ( _com_error& e ) {
        TERR(e,"readData::ReadIndirectInt32");
        return false;
    }
    return true;
}


template<> bool
device<Simulate>::initial_setup( task& task, const method& m )
{
    if ( simulator * simulator = task.simulator() )
        simulator->setup( m );

    // Create smart pointers to Channels and TriggerSource interfaces
    IAgMD2ChannelPtr spCh1 = task.spDriver()->Channels->Item[L"Channel1"];
    IAgMD2TriggerSourcePtr spTrigSrc = task.spDriver()->Trigger->Sources->Item[L"External1"];

    try { spCh1->TimeInterleavedChannelList = "Channel2";       } catch ( _com_error& e ) { TERR(e, "TimeInterleavedChannelList");  }
    try { spCh1->PutRange(m.front_end_range);                   } catch ( _com_error& e ) { TERR(e, "Range");  }
    try { spCh1->PutOffset(m.front_end_offset);                 } catch ( _com_error& e ) { TERR(e, "Offset");  }
    // Setup triggering
    try { task.spDriver()->Trigger->ActiveSource = "External1"; } catch ( _com_error& e ) { TERR(e, "Trigger::ActiveSource");  }
    try { spTrigSrc->PutLevel(m.ext_trigger_level);             } catch ( _com_error& e ) { TERR(e, "TriggerSource::Level");  }

    // Calibrate
    ADTRACE() << "Calibrating...";
    try { task.spDriver()->Calibration->SelfCalibrate(); } catch ( _com_error& e ) { TERR(e, "Calibration::SelfCalibrate"); }

    // Set the sample rate and nbr of samples to acquire
    double sample_rate = 1.0e9; //m.samp_rate; // 3.2E9;
    try { task.spDriver()->Acquisition->PutSampleRate(sample_rate); }  catch (_com_error& e) { TERR(e,"SampleRate"); }

    try { spCh1->Filter->Bypass = 1; } catch ( _com_error& e ) { TERR( e, "Bandwidth" ); } // invalid value

    ADTRACE() << "Apply setup...";
    try { task.spDriver()->Acquisition->ApplySetup();  } catch ( _com_error& e ) { TERR( e, "ApplySetup" ); }

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

	return true;
}

template<> bool
device<Simulate>::setup( task& task, const method& m )
{
    if ( simulator * simulator = task.simulator() )
        simulator->setup( m );
    
    // Set the sample rate and nbr of samples to acquire
    // double sample_rate = m.samp_rate; // 3.2E9;
    //try { task.spDriver()->Acquisition->PutSampleRate( sample_rate ); } catch ( _com_error& e ) { TERR( e, "SampleRate" ); }
    //try { spCh1->Filter->Bypass = 1; } catch ( _com_error& e ) { TERR( e, "Bandwidth" ); } // invalid value

    ADTRACE() << "Apply setup...";
    try { task.spDriver()->Acquisition->ApplySetup(); } catch ( _com_error& e ) { TERR( e, "ApplySetup" ); }

    //std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
    
    return true; // device<Simulate>::initial_setup( task, m );
}

template<> bool
device<Simulate>::acquire( task& task )
{
    if ( simulator * simulator = task.simulator() )
        return simulator->acquire( task.io_service() );
    return false;
}

template<> bool
device<Simulate>::waitForEndOfAcquisition( task& task, int /* timeout */)
{
    if ( simulator * simulator = task.simulator() )
        return simulator->waitForEndOfAcquisition();
    return false;
}

template<> bool
device<Simulate>::readData( task& task, waveform& data )
{
    data.method_ = task.method();
    if ( simulator * simulator = task.simulator() ) {
        simulator->readData( data );
        return true;
    }
    return false;
}
