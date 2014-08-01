/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quanprocessor.hpp"
#include "quandatawriter.hpp"
#include <adcontrols/quancalibrations.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adlog/logger.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include <adwidgets/progresswnd.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <algorithm>
#include <numeric>
#include <set>
#include <vector>

namespace quan {

    struct QuanUnknown {
        uint64_t idResponse_; // QuanResponse primary id
        uint64_t idCompound_; // QuanCompound primary id (or NULL)
        uint64_t uniqId_;
        std::string uniqGuid_;
        double intensity_;
        QuanUnknown() : idResponse_( 0 ), idCompound_(0), uniqId_(0), intensity_( 0 ) {}
        QuanUnknown( const QuanUnknown& t ) : idResponse_( t.idResponse_ ), idCompound_( t.idCompound_ ), uniqId_( t.uniqId_ ), intensity_( t.intensity_ ) {
        }
    };

}

using namespace quan;

QuanProcessor::~QuanProcessor()
{
}

QuanProcessor::QuanProcessor()
{
}

QuanProcessor::QuanProcessor( const QuanProcessor& t ) : sequence_( t.sequence_ )
                                                       , que_( t.que_ )
                                                       , progress_( t.progress_ )
                                                       , progress_total_( t.progress_total_ )
                                                       , progress_count_( t.progress_count_ )
{
}

QuanProcessor::QuanProcessor( std::shared_ptr< adcontrols::QuanSequence >& s
                              , std::shared_ptr< adcontrols::ProcessMethod >& pm ) : sequence_( s )
                                                                                   , procmethod_( pm )
                                                                                   , progress_( adwidgets::ProgressWnd::instance()->addbar() )
                                                                                   , progress_total_(0)
                                                                                   , progress_count_(0)
{
    // combine per dataSource
    for ( auto it = sequence_->begin(); it != sequence_->end(); ++it )
        que_[ it->dataSource() ].push_back( *it );
    progress_total_ = std::accumulate( que_.begin(), que_.end(), 0, [] ( int n, const decltype(*que_.begin())& q ){ return n + int( q.second.size() ); } );
    (*progress_)(0, progress_total_);
}

adcontrols::QuanSequence *
QuanProcessor::sequence()
{
    return sequence_.get();
}

const adcontrols::QuanSequence *
QuanProcessor::sequence() const
{
    return sequence_.get();
}

const std::shared_ptr< adcontrols::ProcessMethod >&
QuanProcessor::procmethod() const
{
    return procmethod_;
}

QuanProcessor::iterator
QuanProcessor::begin()
{
    return que_.begin();
}

QuanProcessor::iterator
QuanProcessor::end()
{
    return que_.end();
}

QuanProcessor::const_iterator
QuanProcessor::begin() const
{
    return que_.begin();
}

QuanProcessor::const_iterator
QuanProcessor::end() const
{
    return que_.end();
}

void
QuanProcessor::complete( const adcontrols::QuanSample * )
{
    (*progress_)(++progress_count_, progress_total_);
}

void
QuanProcessor::doCalibration()
{
    boost::filesystem::path database( sequence_->outfile() );
    if ( !boost::filesystem::exists( database ) )
        return;

    adfs::filesystem fs;
    if ( !fs.mount( database.wstring().c_str() ) )
        return;
    
    auto qM = procmethod_->find< adcontrols::QuanMethod >();
    if ( !qM )
        return;

    adcontrols::QuanCalibrations results;

    adfs::stmt sql( fs.db() );

    if ( !QuanDataWriter::insert_table( sql, results.ident(), "Create QuanCalibration" ) )
        return;

    int nLevels = qM->levels(); // must be 1 or larger
    int nReplicates = qM->replicates(); // must be 1 or larger
    adcontrols::QuanMethod::CalibEq eq = qM->equation();
    int order = qM->polynomialOrder(); // if CalibEq >= isCalibLinear, otherwise taking an average

    std::map< uint64_t, adcontrols::QuanCalibration > calibrants;
    std::map< uint64_t, std::set< int > > levels;

    if ( sql.prepare( "\
SELECT QuanCompound.id \
, QuanCompound.uniqId \
, QuanCompound.uuid \
, QuanResponse.formula \
, QuanResponse.intensity \
, QuanAmount.amount \
, QuanSample.level \
, QuanSample.sampleType \
FROM QuanSample,QuanResponse,QuanAmount,QuanCompound \
WHERE QuanSample.id = idSample \
AND QuanCompound.id = QuanAmount.idCompound \
AND sampleType = 1 \
AND QuanAmount.idCompound = (SELECT id from QuanCompound WHERE uniqId = QuanResponse.uniqId) \
AND QuanAmount.level = QuanSample.level \
ORDER BY QuanCompound.id") ) {

        while ( sql.step() == adfs::sqlite_row ) {
            int row = 0;
            uint64_t idCompound = sql.get_column_value< uint64_t >( row++ );     // QuanCompound.id
            uint64_t uniqId = sql.get_column_value< uint64_t >( row++ );         // QuanCompound.uniqId
            std::string uniqGuid = sql.get_column_value< std::string >( row++ ); // QuanCompound.uniqGuid
            std::string formula = sql.get_column_value< std::string >( row++ );  // QuanResponse.formula
            double intensity = sql.get_column_value< double >( row++ );          // QuanResponse.intensity
            double amount = sql.get_column_value< double >( row++ );             // QuanAmount.amount (standard amount added)
            uint64_t level = sql.get_column_value< uint64_t >( row++ );          // QuanSample.level
            uint64_t sampType = sql.get_column_value< uint64_t >( row++ );       // STD (1)
            
            auto it = calibrants.find( idCompound );
            if ( it == calibrants.end() ) {
                adcontrols::QuanCalibration& d = calibrants[ idCompound ];
                d.uniqId( uint32_t( uniqId ) );
                d.uniqGuid( boost::uuids::string_generator()(uniqGuid) );
                d.formula( formula.c_str() );
            }
            adcontrols::QuanCalibration& d = calibrants[ idCompound ];
            if ( d.uniqId() != uniqId || d.uniqGuid() != boost::uuids::string_generator()(uniqGuid) )
                ADERROR() << "wrong calibrant data selected";
            
            d << std::make_pair( amount, intensity );
        }
    }

    sql.begin();
    for ( auto& map : calibrants ) {
        map.second.fit( nLevels );
        results << map.second;

        if ( sql.prepare( "\
INSERT INTO QuanCalib (idAudit, uuid, idCompound, uniqGuid, uniqId, uuidMethod, n, min_x, max_x, chisqr, a, b, c, d, e, f) \
SELECT idAudit.id, :uuid, ?, ?, ?, ?, :n, :min_x, :max_x, :chisqr, :a, :b, :c, :d, :e, :f \
FROM idAudit WHERE uuid = :uuid") ) {
            int row = 1;
            sql.bind( row++ ) = boost::lexical_cast<std::string>(results.ident().uuid()); // :uuid
            sql.bind( row++ ) = map.first;                                                // ?idCompound
            sql.bind( row++ ) = boost::lexical_cast<std::string>(map.second.uniqGuid());  // ?uniqGuid
            sql.bind( row++ ) = map.second.uniqId();                                      // ?uniqId
            sql.bind( row++ ) = boost::lexical_cast<std::string>(qM->ident().uuid());     // ?uuidMethod
            sql.bind( row++ ) = uint64_t( map.second.size() );                            // :n
            sql.bind( row++ ) = double( map.second.min_x() );                             // :min_x
            sql.bind( row++ ) = double( map.second.max_x() );                             // :max_x
            sql.bind( row++ ) = double( map.second.chisqr() );                            // chisqr
            size_t i = 0;
            while ( i < map.second.nTerms() )
                sql.bind( row++ ) = map.second.coefficients()[ i++ ];                     // a..f
            while ( i++ < 5 )
                sql.bind( row++ ) = adfs::null();                                         // a..f

            if ( sql.step() != adfs::sqlite_done ) {
                ADERROR() << "sql error";
            }
        }
    }
    sql.commit();

#if 0
        if ( sql.prepare( "\
SELECT QuanResponse.id, QuanResponse.idCompound, QuanResponse.uniqId, QuanResponse.uniqGuid, QuanResponse.intensity \
FROM QuanSample, QuanResponse \
WHERE sampleType = 0 AND QuanResponse.idSample = QuanSample.id" ) ) {
            
            while ( sql.step() == adfs::sqlite_row ) {
                int row = 0;
                QuanUnknown unk;
                unk.idResponse_ = sql.get_column_value< uint64_t >( row++ );
                unk.idCompound_ = sql.get_column_value< uint64_t >( row++ );
                unk.uniqId_     = sql.get_column_value< uint64_t >( row++ );
                unk.uniqGuid_   = sql.get_column_value< uint64_t >( row++ );
                unk.intensity_  = sql.get_column_value< double >( row++ );
                unknowns.push_back( unk );
            }
        }

        for ( auto& unk : unknowns ) {
            auto it = calibrants.find( unk.idCompound_ );
            if ( it != calibrants.end() && it->second.coeffs_.empty() ) {
                double est_a = adportable::polfit::estimate_y( it->second.coeffs_, unk.idResponse_ );

                if ( sql.prepare( "\
UPDATE QuanResponse SET calibId = (SELECT id from QuanCalib WHERE idCompound = ? ORDER BY id DESC)\
, amount = ? WHERE QuanResponse.id = ?" ) ) {
                    sql.bind( 1 ) = est_a;
                    sql.bind( 2 ) = unk.idCompound_;
                    sql.bind( 3 ) = unk.idResponse_;

                    if ( sql.step() != adfs::sqlite_row )
                        ADTRACE() << "sel error.";
                }
            }
        }

    }    
#endif
}

void
QuanProcessor::doQuantification()
{
}
