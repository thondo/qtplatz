// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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

#include "dataprocessor.hpp"
#include "processmediator.hpp"
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adfs/adfs.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adutils/acquiredconf_v3.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#if __cplusplus >= 201402L
# include <codecvt>
#else
# include <adportable/utf.hpp>
#endif
#include <locale>
#include <numeric>
#include <string>

using namespace adprocessor;

dataprocessor::~dataprocessor()
{
}

dataprocessor::dataprocessor() : rawdata_( 0 )
                               , modified_( false )
                               , portfolio_( std::make_unique< portfolio::Portfolio >() )
                               , mode_( 0 )
{
}

void
dataprocessor::setMode( int id )
{
    mode_ = id;
}

int
dataprocessor::mode() const
{
    return mode_;
}

void
dataprocessor::setModified( bool modified )
{
    modified_ = modified;
}

bool
dataprocessor::open( const std::wstring& filename, std::wstring& error_message )
{
    if ( auto file = std::unique_ptr< adcontrols::datafile >( adcontrols::datafile::open( filename, false ) ) ) {

        boost::filesystem::path path( filename );

        auto fs = std::make_unique< adfs::filesystem >();
        if ( fs->mount( path ) ) {
            fs_ = std::move( fs );
        } else if ( fs->create( ":memory:" ) ) {
            fs_ = std::move( fs );
            adutils::v3::AcquiredConf::create_table_v3( *db() );
        } else
            return false;
        file_ = std::move( file );
        try {
            file_->accept( *this );  // may access 'db' if file was imported from csv.
        } catch ( std::exception& ex ) {
#if __cplusplus >= 201402L
            std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
            error_message = converter.from_bytes( ex.what() );
#else
            error_message = adportable::utf::to_wstring( ex.what() );
#endif
            return false;
        }
        if ( auto sp = massSpectrometer() )
            ProcessMediator::instance()->onCreate( sp->massSpectrometerClsid(), this->shared_from_this() );

        return true;
    }
    return false;
}

const std::wstring&
dataprocessor::filename() const
{
    return file_->filename();
}

void
dataprocessor::setFile( std::unique_ptr< adcontrols::datafile >&& file )
{
    file_ = std::move( file );

    if ( file_ ) {
        file_->accept( *this );
    }

}

adcontrols::datafile *
dataprocessor::file()
{
    return file_.get();
}

const adcontrols::datafile *
dataprocessor::file() const
{
    return file_.get();
}

const adcontrols::LCMSDataset *
dataprocessor::rawdata()
{
    return rawdata_;
}

std::shared_ptr< adfs::sqlite >
dataprocessor::db() const
{
    if ( fs_ )
        return fs_->_ptr();
    else
        return nullptr;
}

adfs::filesystem *
dataprocessor::fs()
{
    return fs_ ? fs_.get() : nullptr;
}

const adfs::filesystem *
dataprocessor::fs() const
{
    return fs_ ? fs_.get() : nullptr;
}

const portfolio::Portfolio&
dataprocessor::portfolio() const
{
    return *portfolio_;
}

portfolio::Portfolio&
dataprocessor::portfolio()
{
    return *portfolio_;
}

///////////////////////////
bool
dataprocessor::subscribe( const adcontrols::LCMSDataset& data )
{
    rawdata_ = &data;
	return true;
}

bool
dataprocessor::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::string xml = processed.xml();
    portfolio_ = std::make_unique< portfolio::Portfolio >( xml );

    return true;
}

void
dataprocessor::notify( adcontrols::dataSubscriber::idError, const std::string& json )
{
    ADDEBUG() << json;
}

std::shared_ptr< adcontrols::MassSpectrometer >
dataprocessor::massSpectrometer()
{
    if ( ! spectrometer_ && this->db() ) {

        adfs::stmt sql( *this->db() );

        boost::uuids::uuid clsidSpectrometer{{ 0 }};
        sql.prepare( "SELECT id FROM Spectrometer LIMIT 1" );
        if ( sql.step() == adfs::sqlite_row )
            clsidSpectrometer = sql.get_column_value< boost::uuids::uuid >( 0 );

        if ( auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( clsidSpectrometer ) ) {
            spectrometer->initialSetup( *this->db(), {{ 0 }} ); // load relevant to 'master observer'
            spectrometer_ = spectrometer;
        } else {
            if ( auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( adcontrols::iids::adspectrometer_uuid ) ) {
                spectrometer->initialSetup( *this->db(), {{ 0 }} ); // load relevant to 'master observer'
                spectrometer_ = spectrometer;
            }
        }
    }
    return spectrometer_;
}

std::shared_ptr< adcontrols::MassSpectrum >
dataprocessor::readSpectrumFromTimeCount()
{
    std::shared_ptr< adcontrols::MassSpectrum > ms;

    adfs::stmt sql( *this->db() );
    sql.prepare( "SELECT objuuid from AcquiredConf WHERE objtext like 'histogram.timecount.1.%' LIMIT 1" );
    if ( sql.step() == adfs::sqlite_row ) {

        auto objuuid = sql.get_column_value< boost::uuids::uuid >( 0 );

        if ( auto raw = this->rawdata() ) {
            if ( auto reader = raw->dataReader( objuuid ) ) {
                auto it = reader->begin();
                ms = reader->readSpectrum( it );
            }
        }
    }

    boost::uuids::uuid clsidSpectrometer{{ 0 }};
    sql.prepare( "SELECT id FROM Spectrometer LIMIT 1" );
    if ( sql.step() == adfs::sqlite_row )
        clsidSpectrometer = sql.get_column_value< boost::uuids::uuid >( 0 );

    auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( clsidSpectrometer );
    if ( ! spectrometer ) {
        ADDEBUG() << "MassSpectrometer " << clsidSpectrometer << " Not installed.";
        return nullptr;
    }

    spectrometer->initialSetup( *db(), {{ 0 }} );
    auto scanlaw = spectrometer->scanLaw();

    {
        // lead control method
        auto idstr = boost::lexical_cast< std::string >( adcontrols::ControlMethod::Method::clsid() );
        sql.prepare( "SELECT data FROM MetaData WHERE clsid = ?" );
        sql.bind( 1 ) = idstr;
        while( sql.step() == adfs::sqlite_row ) {
            auto blob = sql.get_column_value< adfs::blob >( 0 );
            boost::iostreams::basic_array_source< char > device( reinterpret_cast< const char * >( blob.data() ), size_t( blob.size() ) );
            boost::iostreams::stream< boost::iostreams::basic_array_source< char > > strm( device );
            adcontrols::ControlMethod::Method method;
            if ( adcontrols::ControlMethod::Method::restore( strm, method ) )
                spectrometer->setMethod( method );
        }
    }

    std::vector< uint64_t > countTriggers; // per protocol
    {
        sql.prepare( "SELECT COUNT(*) FROM trigger GROUP BY protocol ORDER BY protocol" );
        while ( sql.step() == adfs::sqlite_row )
            countTriggers.emplace_back( sql.get_column_value< uint64_t >( 0 ) );
    }
    size_t nbrProto = countTriggers.size();

    std::shared_ptr< adcontrols::MassSpectrum > hist = std::make_shared< adcontrols::MassSpectrum >();

    size_t proto(0);
    for ( const auto& trigCounts: countTriggers ) {

        auto temp = proto == 0 ? hist : std::make_shared< adcontrols::MassSpectrum >();
        temp->clone( *ms );
        temp->setCentroid( adcontrols::CentroidNative );

        std::vector< double > t, y, m;
        double ptime(0);
        sql.prepare( "SELECT ROUND(peak_time, 9) AS time,COUNT(*) FROM peak,trigger"
                     " WHERE id=idTrigger AND protocol=? GROUP BY time ORDER BY time" );
        sql.bind(1) = proto;
        while ( sql.step() == adfs::sqlite_row ) {

            double time = sql.get_column_value< double >( 0 ); // time
            uint64_t count = sql.get_column_value< uint64_t >( 1 ); // count

            if ( (time - ptime) > 1.2e-9 ) {
                if ( ptime > 1.0e-9 ) {
                    // add count(0) to the end of last cluster
                    t.emplace_back( ptime + 1.0e-9 );
                    y.emplace_back( 0 ); // count
                    m.emplace_back( scanlaw->getMass( ( ptime + 1.0e-9 ), spectrometer->mode( proto ) ) );
                }
                // add count(0) to the begining of next cluster
                t.emplace_back( time - 1.0e-9 );
                y.emplace_back( 0 ); // count
                m.emplace_back( scanlaw->getMass( ( time - 1.0e-9 ), spectrometer->mode( proto ) ) );
            }
            t.emplace_back( time );
            y.emplace_back( count ); // count
            m.emplace_back( scanlaw->getMass( time, spectrometer->mode( proto ) ) );
            ptime = time;
        }
        temp->setMassArray( std::move( m ) );
        temp->setTimeArray( std::move( t ) );
        temp->setIntensityArray( std::move( y ) );

        if ( auto method = spectrometer->method() )
            spectrometer->setMSProperty( *temp, *method, proto );

        temp->getMSProperty().setNumAverage( trigCounts );
        temp->setProtocol( proto, nbrProto );

        if ( proto++ )
            (*hist) << std::move( temp );
    }
    return hist;
}

#if 0
std::shared_ptr< adcontrols::MassSpectrum >
dataprocessor::readSpectrum( bool histogram, uint32_t pos, int proto )
{
    const std::string traceid = histogram ? "histogram.timecount.1.%" : "tdcdoc.waveform.1.u5303a.ms-cheminfo.com";

    adfs::stmt sql( *this->db() );

    sql.prepare( "SELECT objuuid from AcquiredConf WHERE objtext like ? LIMIT 1" );
    sql.bind( 1 ) = traceid;

    if ( sql.step() == adfs::sqlite_row ) {

        auto objuuid = sql.get_column_value< boost::uuids::uuid >( 0 );

        if ( auto raw = this->rawdata() ) {
            if ( auto reader = raw->dataReader( objuuid ) ) {
                auto it = reader->begin();
                auto ms = reader->readSpectrum( it );
                return ms;
            }
        }
    }
    return nullptr;
}
#endif

std::shared_ptr< adcontrols::MassSpectrum >
dataprocessor::readCoAddedSpectrum( bool histogram, int proto )
{
    const std::string traceid = histogram ? "histogram.timecount.1.%" : "tdcdoc.waveform.1.u5303a.ms-cheminfo.com";

    adfs::stmt sql( *this->db() );

    sql.prepare( "SELECT objuuid from AcquiredConf WHERE objtext like ? LIMIT 1" );
    sql.bind( 1 ) = traceid;

    if ( sql.step() == adfs::sqlite_row ) {

        auto objuuid = sql.get_column_value< boost::uuids::uuid >( 0 );

        if ( auto raw = this->rawdata() ) {
            if ( auto reader = raw->dataReader( objuuid ) ) {
                auto ms = reader->coaddSpectrum( reader->begin( proto ), reader->end() );
                return ms;
            }
        }
    }
    return nullptr;
}

bool
dataprocessor::doCentroid( adcontrols::MSPeakInfo& pkInfo
                           , adcontrols::MassSpectrum& centroid
                           , const adcontrols::MassSpectrum& profile
                           , const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    bool result = false;

    int fcn(0);
    for ( auto& seg: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( profile ) ) {

        if ( fcn == 0 ) {
            result |= peak_detector( m, seg );

            peak_detector.getCentroidSpectrum( centroid );
            pkInfo = peak_detector.getPeakInfo();
            pkInfo.setProtocol( 0, profile.numSegments() + 1 );
            centroid.setProtocol( 0, profile.numSegments() + 1 );

        } else {
            result |= peak_detector( seg );

            auto info = peak_detector.getPeakInfo();
            info.setProtocol( fcn, profile.numSegments() + 1 );
            pkInfo.addSegment( info );

            auto temp = std::make_shared< adcontrols::MassSpectrum >();
            peak_detector.getCentroidSpectrum( *temp );
            temp->setProtocol( fcn, profile.numSegments() + 1 );
            centroid << std::move( temp );
        }
        ++fcn;
    }

    return result;
}

boost::optional< std::pair< adcontrols::MSPeakInfo, adcontrols::MassSpectrum > >
dataprocessor::doCentroid( const adcontrols::MassSpectrum& profile
                           , const adcontrols::ProcessMethod& procm )
{
    if ( auto cm = procm.find< adcontrols::CentroidMethod >() ) {
        adcontrols::MSPeakInfo pkinfo;
        adcontrols::MassSpectrum centroid;

        if ( doCentroid( pkinfo, centroid, profile, *cm ) )
            return {{ pkinfo, centroid }};
    }
    return boost::none;
}

uint64_t
dataprocessor::countTimeCounts( const adcontrols::MassSpectrum& hist, double lMass, double uMass )
{
    const double * masses = hist.getMassArray();
    const double * counts = hist.getIntensityArray();
    auto beg = std::lower_bound( masses, masses + hist.size(), lMass );
    auto end = std::lower_bound( masses, masses + hist.size(), uMass );

    if ( beg != masses + hist.size() ) {
        size_t ids = std::distance( masses, beg );
        size_t ide = std::distance( masses, end );
        uint64_t count = uint64_t( std::accumulate( counts + ids, counts + ide, double(0) ) + 0.5 );
        return count;
    }
    return 0;
}

void
dataprocessor::addContextMenu( ContextID context
                               , QMenu& menu
                               , std::shared_ptr< const adcontrols::MassSpectrum > ms
                               , const std::pair<double, double> & range , bool isTime )
{
    if ( auto sp = massSpectrometer() )
        ProcessMediator::instance()->addContextMenu( sp->massSpectrometerClsid()
                                                     , this->shared_from_this()
                                                     , context
                                                     , menu
                                                     , ms
                                                     , range
                                                     , isTime );
}

void
dataprocessor::addContextMenu( ContextID context
                               , QMenu& menu
                               , const portfolio::Folium& folium )
{
    if ( auto sp = massSpectrometer() )
        ProcessMediator::instance()->addContextMenu( sp->massSpectrometerClsid()
                                                     , this->shared_from_this()
                                                     , context
                                                     , menu
                                                     , folium );
}

bool
dataprocessor::estimateScanLaw( std::shared_ptr< const adcontrols::MassSpectrum > ms
                                , const std::vector<std::pair<int, int> > & refs )
{
    if ( auto sp = massSpectrometer() )
        return ProcessMediator::instance()->estimateScanLaw( sp->massSpectrometerClsid()
                                                             , this->shared_from_this()
                                                             , ms
                                                             , refs );

    return false;
}

bool
dataprocessor::export_text( const portfolio::Folium& folium, std::ostream& outf ) const
{
    auto any = folium.data();

    return true;
}

std::shared_ptr< adcontrols::MassSpectra >
dataprocessor::createSpectrogram( std::shared_ptr< const adcontrols::ProcessMethod > pm
                                  , const adcontrols::DataReader * reader
                                  , int proto
                                  , std::function< bool(size_t, size_t) > progress ) const
{
    const adcontrols::CentroidMethod * centroidMethod = pm->find< adcontrols::CentroidMethod >();

    auto spectra = std::make_shared< adcontrols::MassSpectra >();

    // todo: handle fcn
    if ( auto tic = reader->TIC( proto ) ) {

        spectra->setChromatogram( *tic );

        const size_t total = tic->size();
        progress( 0, total );

        int pos( 0 );

        for ( auto it = reader->begin( proto ); it != reader->end(); ++it ) {

            auto ms = reader->getSpectrum( it->rowid() );
            progress( ++pos, total );

            if ( !ms->isCentroid() || ms->isHistogram() ) {
                adcontrols::MSPeakInfo result;
                auto ptr = std::make_shared< adcontrols::MassSpectrum >();
                doCentroid( result, *ptr, *ms, *centroidMethod );
                ( *spectra ) << std::move( ptr );
            } else {
                ( *spectra ) << std::move( ms );
            }

        }
        using adportable::utf;
        spectra->addDescription( adcontrols::description( L"Create"
                                                          , ( boost::wformat( L"%s,fcn(%d)" )
                                                              % utf::to_wstring( reader->display_name() )
                                                              % proto ).str() )  );
        return spectra;
    }
    return nullptr;
}
