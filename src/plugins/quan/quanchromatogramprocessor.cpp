/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanchromatogramprocessor.hpp"
#include "quancandidate.hpp"
#include "quanchromatogram.hpp"
#include "quanchromatograms.hpp"
#include "quandatawriter.hpp"
#include "quandocument.hpp"
#include "quanprocessor.hpp"
#include "quansampleprocessor.hpp"
#include "quantarget.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adutils/cpio.hpp>
//#include <adwidgets/progresswnd.hpp>
#include <adwidgets/progressinterface.hpp>
#include <chromatogr/chromatography.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

namespace quan {

    struct target_result_finder {
        static bool find( const adcontrols::Targeting& t, const std::string& formula, int32_t proto
                          , const adcontrols::MassSpectrum& centroid, boost::property_tree::ptree& ptree ) {

            assert( centroid.isCentroid() );

            auto it = std::find_if( t.candidates().begin(), t.candidates().end(), [&](const auto& c){ return c.fcn == proto && c.formula == formula; });
            if ( it != t.candidates().end() ) {
                try {
                    auto& tms = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid )[ proto ];
                    ptree.put( "targeting.matchedMass", tms.getMass( it->idx ) );
                    ptree.put( "targeting.mass_error", it->mass_error );
                    ptree.put( "targeting.idx", it->idx );
                    return true;
                } catch ( std::exception& ex ) {
                    ADDEBUG() << ex.what();
                }
            }
            return false;
        }
    };

    struct annotation {
        static void add( adcontrols::MassSpectrum& centroid, int idx, int proto, const std::string& formula ) {
            if ( auto tms = centroid.findProtocol( proto ) ) {
                // todo: erase if peak already has the annotation
                tms->get_annotations() << adcontrols::annotation( formula, tms->getMass( idx ), tms->getIntensity( idx ), idx, 0, adcontrols::annotation::dataFormula );
            }
        }
    };

    // new interface as of 2018-MAY
    struct save_chromatogram {
        
        static std::wstring make_title( const wchar_t * dataSource, const boost::property_tree::ptree& pt )  {
            boost::filesystem::path path( dataSource );

            auto wform = pt.get_optional< std::string >( "generator.extract_by_mols.wform_type" );
            
            if ( auto mol = pt.get_child_optional( "generator.extract_by_mols.moltable" ) ) {
                auto formula = mol.get().get_optional< std::string >( "formula" );
                auto width = mol.get().get_optional< double >( "width" );
                auto proto = mol.get().get_optional< int32_t >( "protocol" );
                
                return ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}-%s" )
                         % ( formula ? adportable::utf::to_wstring( formula.get() ) : L"" )
                         % ( proto ? proto.get() : (-1) )
                         % ( width ? width.get() *1000 : 0.0 )
                         % path.stem().wstring()
                         % ( wform ? adportable::utf::to_wstring( wform.get() ) : L"n/a" ) ).str();
            } else {
                return ( boost::wformat( L"{%s}" ) % path.stem().wstring() ).str();
            }
        }
        
        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer, const wchar_t * dataSource
              , std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult > >& pair
              , const adcontrols::ProcessMethod& procm, size_t idx )   {

            auto title = make_title( dataSource, pair.first->ptree() );
            if ( adfs::file file = writer->write( *pair.first, title ) ) {
                auto fGuid = boost::uuids::string_generator()( file.name() );
                pair.first->ptree().put( "folder.dataGuid", fGuid );
                auto afile = writer->attach< adcontrols::PeakResult >( file, *pair.second, pair.second->dataClass() );
                writer->attach< adcontrols::ProcessMethod >( file, procm, L"Process Method" );
                return fGuid;
            }
            return { 0 };
        }
    };

    // new interface as of 2018-MAY    
    struct save_spectrum {
        static std::wstring make_title( const wchar_t * dataSource, const std::string& formula, double tR, const wchar_t * trailer = L"" ) {
            boost::filesystem::path path( dataSource );
            return ( boost::wformat( L"%s tR(%.3fs) {%s} %s" ) % adportable::utf::to_wstring( formula ) % tR %  path.stem().wstring() % trailer ).str();
        }
        
        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer
              , const wchar_t * dataSource
              , std::shared_ptr< const adcontrols::MassSpectrum > ms
              , const std::wstring& title
              , std::shared_ptr< const adcontrols::ProcessMethod > procm
              , const std::string& formula
              , int32_t proto
              , boost::property_tree::ptree& ptree )  {
            
            if ( auto file = writer->write( *ms, title ) ) {
                if ( auto cm = procm->find< adcontrols::CentroidMethod >() ) {
                    adcontrols::MSPeakInfo pkinfo;
                    adcontrols::MassSpectrum centroid;
                    boost::optional< adcontrols::Targeting > targeting;

                    if ( adprocessor::dataprocessor::doCentroid( pkinfo, centroid, *ms, *cm ) ) {

                        centroid.addDescription( adcontrols::description( L"process", L"Centroid" ) );
                        
                        if ( auto tm = procm->find< adcontrols::TargetingMethod >() ) {
                            targeting = adcontrols::Targeting( *tm );                            
                            if ( targeting.get()( centroid ) || targeting->force_find( centroid, formula, proto ) ) {
                                if ( target_result_finder::find( targeting.get(), formula, proto, centroid, ptree ) )
                                    annotation::add( centroid, ptree.get_optional< int >("targeting.idx").get(), proto, formula );
                            }
                        }

                        if ( auto att = writer->attach< adcontrols::MSPeakInfo >( file, pkinfo, dataproc::Constants::F_MSPEAK_INFO ) ) {
                        }
                        if ( auto att = writer->attach< adcontrols::ProcessMethod >( file, *procm, L"Process Method" ) ) {
                        }
                        if ( auto att = writer->attach< adcontrols::MassSpectrum >( file, centroid, adcontrols::constants::F_CENTROID_SPECTRUM ) ) {
                            if ( targeting )
                                writer->attach< adcontrols::Targeting >( att, targeting.get(), adcontrols::constants::F_TARGETING );
                        }
                    }
                }
                
                if ( ms->isHistogram() ) {
                    if ( auto hist = adcontrols::histogram::make_profile( *ms ) )
                        writer->attach< adcontrols::MassSpectrum >( file, *hist, adcontrols::constants::F_PROFILED_HISTOGRAM );
                }
                
                return boost::uuids::string_generator()( file.name() );
            }
            return { 0 };
        }
    };

    /////////////////////////
    /////////////////////////    

    struct response_builder {

        static void add( QuanSampleProcessor& processor
                         , adcontrols::QuanSample& sample
                         , const std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult> >& pair
                         , const adcontrols::QuanCompounds& compounds ) {

            auto& ptree = pair.first->ptree();
            ADDEBUG() << "************ " << ptree;
                        
            auto matchedMass = ptree.get_optional< double >( "targeting.matchedMass" );
            auto dataGuid = ptree.get_optional< boost::uuids::uuid >( "folder.dataGuid" );
            
            if ( auto child = pair.first->ptree().get_child_optional( "generator.extract_by_mols" ) ) {
                
                if ( auto cmpdGuid = child.get().get_optional< boost::uuids::uuid >( "molid" ) ) { // "generator.extract_by_mols.molid"
                    
                    if ( auto mol = child.get().get_child_optional( "moltable" ) ) {               // "generator.extract_by_mols.moltable"
                        
                        // lookup c-peak
                        if ( auto formula = mol.get().get_optional< std::string >( "formula" ) ) {
                            adcontrols::QuanResponse resp;
                            resp.uuid_cmpd( cmpdGuid.get() );  // compound id (uuid) identify each molecule(formula) and protocol
                            resp.uuid_cmpd_table( compounds.uuid() );
                            if ( dataGuid )
                                resp.setDataGuid( dataGuid.get() );   // corresponding chromatogram data on output adfs file
                            resp.mass_ = matchedMass ? matchedMass.get() : 0;
                            resp.idx_ = (-1);
                            auto it = std::find_if( pair.second->peaks().begin(), pair.second->peaks().end(), [&](auto& p){ return p.formula() == formula.get(); } );
                            if ( it != pair.second->peaks().end() ) {
                                resp.formula( formula.get().c_str() );
                                resp.idx_ = it->peakId();
                                resp.fcn_ = pair.first->protocol();
                                resp.intensity_ = it->peakArea();
                                resp.amounts_ = 0;
                                resp.tR_ = it->peakTime();
                            }
                            sample << resp;
                            
                            ptree.put( "resp.uuid_cmpd", resp.uuid_cmpd() );
                            ptree.put( "resp.uuid_cmpd_table", resp.uuid_cmpd_table() );
                            ptree.put( "resp.dataGuid", dataGuid.get() );
                            ptree.put( "resp.mass", resp.mass_ );
                            ptree.put( "resp.idx", resp.idx_ );
                            ptree.put( "resp.fcn", resp.fcn_ );
                            ptree.put( "resp.intensity", resp.intensity_ );
                            ptree.put( "resp.tR", resp.tR_ );
                        } else {
                            assert( 0 );
                        }
                    }
                } else {
                    ADERROR() << "No Compound ID found -- internal error";
                }
            }
#if ! defined NDEBUG
            ADDEBUG() << pair.first->ptree();
#endif
        }
    };

    /////////////////////////
    /////////////////////////    
}

using namespace quan;

// static
std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const QuanCandidate& candidate, const wchar_t * trailer )
{
    boost::filesystem::path path( dataSource );
    std::wstring title = ( boost::wformat( L"%s, %s" ) % path.stem().wstring() % adportable::utf::to_wstring( candidate.formula() ) ).str();
    title += trailer;  //L" (1st phase)";
    
    return title;
}

std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const std::string& formula, int fcn, double error, const wchar_t * trailer )
{
    return L"";
}


QuanChromatogramProcessor::QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm )
    : procm_( std::make_shared< adcontrols::ProcessMethod >( *pm ) )
    , cXmethods_{ std::make_unique< adcontrols::MSChromatogramMethod >(), std::make_unique< adcontrols::MSChromatogramMethod >() }
    , debug_level_( 0 )
    , save_on_datasource_( false )
{
    if ( auto qm = pm->find< adcontrols::QuanMethod >() ) {
        debug_level_ = qm->debug_level();
        save_on_datasource_ = qm->save_on_datasource();
    }
    
    if ( auto pCompounds = pm->find< adcontrols::QuanCompounds >() ) {

        // mass chromatograms extraction method 
        pCompounds->convert_if( cXmethods_[ 0 ]->molecules(), []( const adcontrols::QuanCompound& comp ){ return !comp.isCounting();} );
        pCompounds->convert_if( cXmethods_[ 1 ]->molecules(), []( const adcontrols::QuanCompound& comp ){ return comp.isCounting();} );

        if ( auto lkm = pm->find< adcontrols::MSLockMethod >() ) {
            for ( auto& cm: cXmethods_ )
                cm->setLockmass( lkm->enabled() );
        }

        if ( auto targeting_method = pm->find< adcontrols::TargetingMethod >() ) {
            for ( auto& cm: cXmethods_ )
                cm->width( targeting_method->tolerance( targeting_method->toleranceMethod() ), adcontrols::MSChromatogramMethod::widthInDa );

            auto tm = std::make_shared< adcontrols::TargetingMethod >( *targeting_method );
            pCompounds->convert( tm->molecules() );
            *procm_ *= *tm;
        }
    }
}

////////////////////////////////////
/// main entry point             ///
/// new interface as of 2018-MAY ///
///////////////////////////////////
bool
QuanChromatogramProcessor::operator()( QuanSampleProcessor& processor
                                       , adcontrols::QuanSample& sample
                                       , std::shared_ptr< QuanDataWriter > writer
                                       , std::shared_ptr< adwidgets::ProgressInterface > progress )
{
    
    if ( auto raw = processor.getLCMSDataset() ) {

        if ( raw->dataformat_version() < 3 )  // no support for old (before 2014) data
            return false;

        auto extractor = std::make_unique< adprocessor::v3::MSChromatogramExtractor >( raw );

        std::array< std::shared_ptr< const adcontrols::DataReader >, 2 > readers;

        for ( auto reader: raw->dataReaders() ) {
            if ( reader->objtext().find( "waveform" ) != std::string::npos )
                readers[ 0 ] = reader;
            if ( reader->objtext().find( "histogram" ) != std::string::npos )
                readers[ 1 ] = reader;
        }

        auto pCompounds = procm_->find< adcontrols::QuanCompounds >();
        if ( !pCompounds )
            return false;

        size_t idx = 0;
        for ( auto reader: readers ) {
            if ( reader ) {
                std::vector< std::pair< std::shared_ptr< adcontrols::Chromatogram >
                                        , std::shared_ptr< adcontrols::PeakResult > > > rlist;

                adcontrols::ProcessMethod pm( *procm_ );
                pm *= (*cXmethods_[ idx ]);
                do {
                    std::vector< std::shared_ptr< adcontrols::Chromatogram > > clist;
                    extractor->extract_by_mols( clist, pm, reader, [progress]( size_t, size_t )->bool{ return (*progress)(); } );
                    std::transform( clist.begin(), clist.end(), std::back_inserter( rlist )
                                    , []( auto p ){ return std::make_pair( std::move( p ), std::make_shared< adcontrols::PeakResult >() ); });
                } while (0);

                if ( auto peakm = procm_->find< adcontrols::PeakMethod >() ) {
                    for ( auto& pair: rlist ) {
                        if ( findPeaks( *pair.second, *pair.first, *peakm ) )
                            identify( *pair.second, *pCompounds, *pair.first );
                        pair.first->setBaselines( pair.second->baselines() );
                        pair.first->setPeaks( pair.second->peaks() );          // copy identified peak result (for annotation in chroamtogram widget)
                    }
                }
                
                for ( auto& pair: rlist ) {
                    boost::uuids::uuid msGuid{ 0 }, dataGuid{ 0 };
                    auto& chr = pair.first;
                    auto& ptree = chr->ptree();
                    for ( auto& pk: pair.second->peaks() ) {
                        if ( !pk.name().empty() ) {
                            if ( auto ms = extractor->getMassSpectrum( pk.peakTime() ) ) {
                                auto title = save_spectrum::make_title( sample.dataSource(), pk.formula(), pk.peakTime(), (idx == 0 ? L" (profile)" : L" (histogram)" ) );
                                msGuid = save_spectrum::save( writer, sample.dataSource(), ms, title, procm_, pk.formula(), chr->protocol(), chr->ptree() );
                            }
                        }
                    }
                    dataGuid = save_chromatogram::save( writer, sample.dataSource(), pair, pm, idx );
                    response_builder::add( processor, sample, pair, *pCompounds );
                    auto index = ptree.get_optional< int32_t >( "targeting.idx" );
                    writer->insert_reference( dataGuid, msGuid, index ? index.get() : (-1), chr->protocol() );
                }
                ++idx;
            }
        }
    }
}

// static
bool
QuanChromatogramProcessor::findPeaks( adcontrols::PeakResult& res, const adcontrols::Chromatogram& chr, const adcontrols::PeakMethod& pm )
{
    chromatogr::Chromatography peakfinder( pm );
    if ( peakfinder( chr ) ) {
        res.baselines() = peakfinder.getBaselines();
        res.peaks() = peakfinder.getPeaks();
        return true;
    }
    return false;
}

// static
bool
QuanChromatogramProcessor::identify( adcontrols::PeakResult& res, const adcontrols::QuanCompounds& compounds, const adcontrols::Chromatogram& chr )
{
    if ( auto child = chr.ptree().get_child_optional( "generator.extract_by_mols" ) ) {
        auto uuid = child.get().get_optional< boost::uuids::uuid >( "molid" );
        if ( auto mol = child.get().get_child_optional( "moltable" ) ) {
            auto formula = mol.get().get_optional< std::string >( "formula" );

            auto cmpd = std::find_if( compounds.begin(), compounds.end(), [&]( auto& a ) { return a.uuid() == uuid.get(); } );
            while ( cmpd != compounds.end() ) {

                auto pk = std::find_if( res.peaks().begin(), res.peaks().end(), [&]( const auto& p ){ return p.startTime() < cmpd->tR() && cmpd->tR() < p.endTime(); } );
                if ( pk != res.peaks().end() ) {
                    pk->setFormula( formula.get().c_str() );
                    pk->setName( adcontrols::ChemicalFormula::formatFormula( pk->formula() ) );
                }
                
                // next candidate
                std::advance( cmpd, 1 );
                cmpd = std::find_if( cmpd, compounds.end(), [&]( auto& a ) { return a.uuid() == uuid.get(); } );
            }
        }
    }
}

