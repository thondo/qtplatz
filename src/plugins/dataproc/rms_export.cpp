/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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
``** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "rms_export.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <fstream>
#include <iomanip>

using namespace dataproc;

void
rms_export::text_export( const boost::filesystem::path& path, const std::pair<double,double>& range, bool axisIsTime )
{
}

namespace {

    struct rms_writer {

        std::shared_ptr< adfs::sqlite > db_;
        const std::wstring filename_;
        int64_t fileid_;

        rms_writer( std::shared_ptr< adfs::sqlite > db, const std::wstring& filename ) : db_( db ), filename_( filename ), fileid_(-1) {
            adfs::stmt sql( *db_ );
            sql.begin();
            sql.prepare( "INSERT OR IGNORE INTO dataSource (filename) VALUES (?)" );
            sql.bind( 1 ) = filename_;
            if ( sql.step() == adfs::sqlite_done )
                fileid_ = db_->last_insert_rowid();
            sql.commit();
        }

        static void create_table( std::shared_ptr< adfs::sqlite > db ) {
            adfs::stmt sql( *db );
            sql.exec( "CREATE TABLE IF NOT EXISTS dataSource ("
                      "id INTEGER PRIMARY KEY"
                      ",filename TEXT"
                      ",UNIQUE( filename )"
                      ")" );

            sql.exec( "CREATE TABLE IF NOT EXISTS spectrum ("
                      "id INTEGER PRIMARY KEY"
                      ",fileid INTEGER"
                      ",spname TEXT"
                      ",sptype TEXT"
                      ",UNIQUE( spname )"                      
                      ",FOREIGN KEY ( fileid ) REFERENCES dataSource ( id )"
                      ")" );

            sql.exec( "DROP TABLE IF EXISTS RMS");
            sql.exec( "CREATE TABLE IF NOT EXISTS RMS ("
                      "spid     INTEGER"
                      ",proto   INTEGER"
                      ",mode    INTEGER"
                      ",t_left  REAL"
                      ",t_right REAL"                      
                      ",N       INTEGER"
                      ",rms     REAL"
                      ",t_min   REAL"
                      ",v_min   REAL"
                      ",t_max   REAL"
                      ",v_max   REAL"
                      ",nAvg    INTEGER"                                            
                      ",FOREIGN KEY ( spid ) REFERENCES spectrum ( id )"
                      ")" );
        }

        void write( adcontrols::MassSpectrum& profile, const portfolio::Folium& f, const std::pair<double,double>& range, bool isTime ) {
            int64_t spid(0);
            adfs::stmt sql( *db_ );
            sql.begin();
            sql.prepare( "INSERT OR IGNORE INTO spectrum ( fileid, spname, sptype ) SELECT id,?,? FROM dataSource WHERE filename = ?" );
            sql.bind(1) = f.name();
            sql.bind(2) = std::string("");
            sql.bind(3) = filename_;
            if ( sql.step() == adfs::sqlite_done )
                spid = db_->last_insert_rowid();
            else
                ADDEBUG() << sql.errmsg();
            sql.commit();

            sql.prepare(
                "INSERT OR REPLACE INTO RMS (spid,proto,mode,t_left,t_right,N,rms,t_min,v_min,t_max,v_max,nAvg )"
                " SELECT id,?,?,?,?,?,?,?,?,?,?,? FROM spectrum WHERE spname = ?" );
            // sql.prepare( "INSERT OR REPLACE INTO RMS(spid,proto,mode,t_left,t_right,N,rms,t_min,v_min,t_max,v_max,nAvg ) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)" );
            uint32_t proto(0);
            for ( const auto& ms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( profile ) ){
                if ( auto rms = rms_export::compute_rms( ms, range, isTime ) ) {
                    sql.reset();
                    uint32_t id(1);
                    sql.bind(id++) = proto;
                    sql.bind(id++) = ms.mode();
                    sql.bind(id++) = std::get<0>( rms.get() ).first;  // time start
                    sql.bind(id++) = std::get<0>( rms.get() ).second; // time end
                    sql.bind(id++) = std::get<1>( rms.get() );        // N
                    sql.bind(id++) = std::get<2>( rms.get() );        // rms
                    sql.bind(id++) = std::get<3>( rms.get() );        // min time
                    sql.bind(id++) = std::get<4>( rms.get() );        // min value
                    sql.bind(id++) = std::get<5>( rms.get() );        // max time
                    sql.bind(id++) = std::get<6>( rms.get() );        // max value
                    sql.bind(id++) = ms.getMSProperty().numAverage();
                    sql.bind(id++) = f.name();

                    if ( sql.step() != adfs::sqlite_done )
                        ADDEBUG() << sql.errmsg();
                }
                ++proto;
            }
        }

    };
}


void
rms_export::sqlite_export( const boost::filesystem::path& path, const std::pair<double,double>& range, bool axisIsTime )
{
    auto db = std::make_shared< adfs::sqlite >();
    if ( db->open( path.string().c_str(), adfs::flags::opencreate ) ) {
        adfs::stmt sql( *db );
        sql.exec( "PRAGMA synchronous = OFF" );
        sql.exec( "PRAGMA journal_mode = MEMORY" );
        sql.exec( "PRAGMA FOREIGN_KEYS = ON" );
        rms_writer::create_table( db );

        for ( auto& session : *SessionManager::instance() ) {
            if ( auto processor = session.processor() ) {

                rms_writer writer( db, processor->filename() );

                auto sfolio = processor->getPortfolio().findFolder( L"Spectra" );

                for ( auto& folium: sfolio.folio() ) {
                    if ( folium.attribute( L"isChecked" ) == L"true" ) {
                        if ( folium.empty() )
                            processor->fetch( folium );
                        if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                            ADDEBUG() << profile->getMSProperty().numAverage();
                            writer.write(*profile, folium, range, axisIsTime );
                        }
                    }
                }
            }
        }
    }
}

boost::optional<
    std::tuple< std::pair<double, double> // t0,t1
                , size_t // N
                , double // rms
                , double // min time
                , double // min value
                , double // max time
                , double // max value
                >
            >
rms_export::compute_rms( const adcontrols::MassSpectrum& ms, const std::pair< double, double >& xrange, bool isTime )
{
    std::pair<size_t, size_t> range;
    if ( isTime ) {
        range.first = ms.getIndexFromTime( xrange.first, false );
        range.second = ms.getIndexFromTime( xrange.second, true );
    } else {
        const double * masses = ms.getMassArray();
        range.first = std::distance( masses, std::lower_bound( masses, masses + ms.size(), xrange.first ) );
        range.second = std::distance( masses, std::lower_bound( masses, masses + ms.size(), xrange.second ) );
    }
    const size_t N = range.second - range.first + 1;

    ADDEBUG() << "compute_rms: " << xrange << ", range: " << range << ", N=" << N;

    if ( N >= 5 ) {
        double dbase, rms;

        adportable::array_wrapper<const double> data( ms.getIntensityArray() + range.first, N );

        std::tie( std::ignore, dbase, rms ) = adportable::spectrum_processor::tic( data.size(), data.data() );
        auto mm = std::minmax_element( data.begin(), data.end() );
        double t_min = ms.time( std::distance( data.begin(), mm.first ) + range.first );
        double t_max = ms.time( std::distance( data.begin(), mm.second ) + range.first );

        return std::make_tuple( std::make_pair( ms.time( range.first ), ms.time( range.second ) )
                                , N
                                , rms
                                , t_min
                                , *mm.first
                                , t_max
                                , *mm.second );
    }
    return boost::none;    
}
