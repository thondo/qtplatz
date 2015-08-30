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

#include "sampleprocessor.hpp"
#include "task.hpp"
#include "logging.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adinterface/signalobserverC.h>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/split_filename.hpp>
#include <adportable/utf.hpp>
#include <adutils/mscalibio.hpp>
#include <adutils/acquiredconf.hpp>
#include <adutils/acquireddata.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <regex>
#include <string>

using namespace adcontroller;

static size_t __nid__;

SampleProcessor::~SampleProcessor()
{
    fs_->close();
    boost::filesystem::path progress_name( storage_name_ );
    storage_name_.replace_extension( ".adfs" );

    boost::system::error_code ec;
    boost::filesystem::rename( progress_name, storage_name_, ec );
    if ( ec )
        ADERROR() << boost::format( "Sample %1% close failed: " ) % storage_name_.stem().string() % ec.message();
    else
        ADTRACE() << boost::format( "Sample %1% closed." ) % storage_name_.stem().string();
}

SampleProcessor::SampleProcessor( boost::asio::io_service& io_service
                                  , std::shared_ptr< adcontrols::SampleRun > run
                                  , std::shared_ptr< adcontrols::ControlMethod::Method > cmth )
    : fs_( new adfs::filesystem )
    , inProgress_( false )
    , myId_( __nid__++ )
    , strand_( io_service )
    , objId_front_( 0 )
    , pos_front_( 0 )
    , stop_triggered_( false )
    , sampleRun_( run )
    , ctrl_method_( cmth )
    , ts_inject_trigger_( 0 )
{
}

void
SampleProcessor::prepare_storage( SignalObserver::Observer * masterObserver )
{
    boost::filesystem::path path( sampleRun_->dataDirectory() );

	if ( ! boost::filesystem::exists( path ) )
		boost::filesystem::create_directories( path );

    boost::filesystem::path prefix = adportable::split_filename::prefix<wchar_t>( sampleRun_->filePrefix() );

    int runno = 0;
	if ( boost::filesystem::exists( path ) && boost::filesystem::is_directory( path ) ) {
        using boost::filesystem::directory_iterator;
		for ( directory_iterator it( path ); it != directory_iterator(); ++it ) {
            boost::filesystem::path fname = (*it);
			if ( fname.extension().string() == ".adfs" ) {
                runno = std::max( runno, adportable::split_filename::trailer_number_int( fname.stem().wstring() ) );
            }
        }
    }
    std::wostringstream o;
    o << prefix.wstring() << std::setw( 4 ) << std::setfill( L'0' ) << (runno + 1);
    
	boost::filesystem::path filename = path / o.str();
	filename.replace_extension( ".adfs~" );

	storage_name_ = filename.normalize();

    sampleRun_->filePrefix( filename.stem().wstring().c_str() );
	
	///////////// creating filesystem ///////////////////
	if ( ! fs_->create( storage_name_.wstring().c_str() ) )
		return;

    adutils::AcquiredConf::create_table( fs_->db() );
    adutils::AcquiredData::create_table( fs_->db() );
    adutils::mscalibio::create_table( fs_->db() );
	
	populate_descriptions( masterObserver );
    populate_calibration( masterObserver );

	Logging( L"Sample processor '%1%' is ready to run.", EventLog::pri_INFO ) % storage_name_.wstring();
}

void
SampleProcessor::stop_triggered()
{
    stop_triggered_ = true;
}

void
SampleProcessor::pos_front( unsigned int pos, unsigned long objId )
{
    if ( pos > pos_front_ ) {
        // keep largest pos and it's objId
        pos_front_ = pos;
        objId_front_ = objId;
    }
}

void
SampleProcessor::handle_data( unsigned long objId, long pos
                              , const SignalObserver::DataReadBuffer& rdBuf )
{
    if ( rdBuf.events & SignalObserver::wkEvent_INJECT ) {
        ts_inject_trigger_ = rdBuf.uptime;
        tp_inject_trigger_ = std::chrono::steady_clock::now(); // CAUTION: this has some unknown delay from exact trigger.
        if ( ! inProgress_ )
            Logging( L"Sample '%1%' got an INJECTION", EventLog::pri_INFO ) % storage_name_.wstring();
		inProgress_ = true;
        iTask::instance()->notify_inject( this, objId, pos, rdBuf.uptime );
    }

	if ( ! inProgress_ ) 
		return;

	adfs::stmt sql( fs_->db() );
	sql.prepare( "INSERT INTO AcquiredData VALUES( :oid, :time, :npos, :fcn, :events, :data, :meta )" );
	sql.begin();
	sql.bind( 1 ) = objId;
	sql.bind( 2 ) = rdBuf.uptime;
	sql.bind( 3 ) = pos;
	sql.bind( 4 ) = rdBuf.fcn;
	sql.bind( 5 ) = rdBuf.events;
	sql.bind( 6 ) = adfs::blob( rdBuf.xdata.length(), reinterpret_cast<const int8_t *>( rdBuf.xdata.get_buffer() ) );
	sql.bind( 7 ) = adfs::blob( rdBuf.xmeta.length(), reinterpret_cast<const int8_t *>( rdBuf.xmeta.get_buffer() ) ); // method
	if ( sql.step() == adfs::sqlite_done )
		sql.commit();
	else
		sql.reset();

    if ( objId == objId_front_ && pos_front_ > unsigned( pos + 10 ) ) {
        size_t nBehind = pos_front_ - pos;
        if ( stop_triggered_ && ( nBehind % 5 == 0 ) )
            Logging( L"'%1%' is being closed.  Saving %2% more data in background.", EventLog::pri_INFO ) % storage_name_.stem() % nBehind;
        else if ( nBehind % 25 == 0 )
            Logging( L"Sample '%1%' %2% data behind.", EventLog::pri_INFO ) % storage_name_.stem() % nBehind;
    }

    auto elapsed_count = rdBuf.uptime - ts_inject_trigger_;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - tp_inject_trigger_);
    if ( objId == 1 )
        Logging( L"Elapsed time: %1%", EventLog::pri_INFO ) % double( elapsed_time.count() / 60.0 );

    if ( elapsed_time.count() >= sampleRun_->methodTime() || double( elapsed_count ) * 1.0e-6 >= sampleRun_->methodTime() ) {
        inProgress_ = false;
        iTask::instance()->post_stop_run();
    }

}

void
SampleProcessor::populate_calibration( SignalObserver::Observer * parent )
{
    SignalObserver::Observers_var vec = parent->getSiblings();

    if ( ( vec.ptr() != 0 ) && ( vec->length() > 0 ) ) {
        
        for ( CORBA::ULong i = 0; i < vec->length(); ++i ) {
            
            SignalObserver::Observer_ptr observer = vec[ i ];
            unsigned long objId = observer->objId();
            CORBA::String_var dataClass;
            SignalObserver::octet_array_var data;
            CORBA::ULong idx = 0;
            while ( observer->readCalibration( idx++, data, dataClass ) ) {
                adfs::stmt sql( fs_->db() );
                sql.prepare( "INSERT INTO Calibration VALUES(:objid,:dataClass,:data,0)" );
                sql.bind( 1 ) = objId;
                sql.bind( 2 ) = std::wstring( adportable::utf::to_wstring( dataClass.in() ) );
                sql.bind( 3 ) = adfs::blob( data->length(), reinterpret_cast< const int8_t *>( data->get_buffer() ) );
                if ( sql.step() == adfs::sqlite_done )
                    sql.commit();
                else
                    sql.reset();
            }
        }
        for ( CORBA::ULong i = 0; i < vec->length(); ++i )
            populate_calibration( vec[ i ] );
    }
    
}

void
SampleProcessor::populate_descriptions( SignalObserver::Observer * parent )
{
    SignalObserver::Observers_var vec = parent->getSiblings();

    unsigned long pobjId = parent->objId();

    if ( ( vec.ptr() != 0 ) && ( vec->length() > 0 ) ) {
        
        for ( CORBA::ULong i = 0; i < vec->length(); ++i ) {
            
            SignalObserver::Observer_ptr observer = vec[ i ];
            
            unsigned long objId = observer->objId();
            
			CORBA::String_var clsid = observer->dataInterpreterClsid();
			SignalObserver::Description_var desc = observer->getDescription();
			CORBA::String_var trace_id = desc->trace_id.in();
			CORBA::String_var trace_display_name = desc->trace_display_name.in();
			CORBA::String_var axis_x_label = desc->axis_x_label.in();
			CORBA::String_var axis_y_label = desc->axis_y_label.in();

            adutils::AcquiredConf::insert( fs_->db()
                                           , objId
                                           , pobjId
                                           , adportable::utf::to_wstring( clsid.in() )
                                           , uint64_t( desc->trace_method )
                                           , uint64_t( desc->spectrometer )
                                           , adportable::utf::to_wstring( trace_id.in() )
                                           , adportable::utf::to_wstring( trace_display_name.in() )
                                           , adportable::utf::to_wstring( axis_x_label.in() )
                                           , adportable::utf::to_wstring( axis_y_label.in() )
                                           , uint64_t( desc->axis_x_decimals )
                                           , uint64_t( desc->axis_y_decimals ) );
        }

        for ( CORBA::ULong i = 0; i < vec->length(); ++i )
            populate_descriptions( vec[ i ] );

    }
}

std::shared_ptr< const adcontrols::SampleRun >
SampleProcessor::sampleRun() const
{
    return sampleRun_;
}
