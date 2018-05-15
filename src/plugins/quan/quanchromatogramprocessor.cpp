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
#include <adwidgets/progresswnd.hpp>
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
        static double find( const adcontrols::Targeting& t, const std::string& formula, int32_t proto
                            , const adcontrols::MassSpectrum& centroid, boost::property_tree::ptree& ptree ) {

            assert( centroid.isCentroid() );

            auto it = std::find_if( t.candidates().begin(), t.candidates().end(), [&](const auto& c){ return c.fcn == proto && c.formula == formula; });
            if ( it != t.candidates().end() ) {
                try {
                    auto& tms = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid )[ proto ];
                    ptree.put( "targeting.matchedMass", tms.getMass( it->idx ) );
                    ptree.put( "targeting.mass_error", it->mass_error );
                    ptree.put( "targeting.idx", it->idx );
                    return tms.getMass( it->idx );
                } catch ( std::exception& ex ) {
                    ADDEBUG() << ex.what();
                }
            }
            ptree.put( "targeting.idx", -1 );
            return 0;
        }
    };

    // new interface as of 2018-MAY
    struct save_chromatograms {
        static std::wstring make_title( const wchar_t * dataSource, const std::string& formula, int fcn, double width, const wchar_t * trailer )  {
            boost::filesystem::path path( dataSource );
            std::wstring title = ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}" ) % adportable::utf::to_wstring( formula ) % fcn % (width*1000) % path.stem().wstring() ).str();
            title += trailer;
            return title;
        }
        
        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer
              , const wchar_t * dataSource
              , const std::vector< std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult > > >& vec
              , std::shared_ptr< const adcontrols::ProcessMethod > procm
              , const wchar_t * title_trailer )   {
            
            boost::uuids::uuid fGuid = { 0 };
            
            for ( auto & pair : vec ) {
                std::wstring title;
                if ( auto child = pair.first->ptree().get_child_optional( "generator.extract_by_mols" ) ) {
                    auto uuid = child.get().get_optional< boost::uuids::uuid >( "molid" );
                    
                    if ( auto mol = child.get().get_child_optional( "moltable" ) ) {
                        auto formula = mol.get().get_optional< std::string >( "formula" );
                        auto width = mol.get().get_optional< double >( "width" );
                        auto protocol = mol.get().get_optional< int32_t >( "protocol" );
                        title = make_title( dataSource, formula ? formula.get() : "", protocol ? protocol.get() : -1, width ? width.get() : 0, title_trailer );
                    } else {
                        title = make_title( dataSource, "", pair.first->protocol(), 0, title_trailer );                
                    }
                } else {
                    title = make_title( dataSource, "", pair.first->protocol(), 0, title_trailer );
                }
                
                if ( adfs::file file = writer->write( *pair.first, title ) ) {
                    fGuid = boost::uuids::string_generator()( file.name() );
                    auto afile = writer->attach< adcontrols::PeakResult >( file, *pair.second, pair.second->dataClass() );
                    writer->attach< adcontrols::ProcessMethod >( afile, *procm, L"ProcessMethod" );
                }
            }
            return fGuid;
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
                if ( !ms->isCentroid() ) {
                    if ( auto cm = procm->find< adcontrols::CentroidMethod >() ) {
                        adcontrols::MSPeakInfo pkinfo;
                        adcontrols::MassSpectrum centroid;
                        if ( adprocessor::dataprocessor::doCentroid( pkinfo, centroid, *ms, *cm ) ) {
                            centroid.addDescription( adcontrols::description( L"process", L"Centroid" ) );
                            if ( auto att = writer->attach< adcontrols::MassSpectrum >( file, centroid, adcontrols::constants::F_CENTROID_SPECTRUM ) ) {
                                if ( auto tm = procm->find< adcontrols::TargetingMethod >() ) {
                                    adcontrols::Targeting targeting( *tm );                            
                                    if ( targeting( centroid ) ) {
                                        target_result_finder::find( targeting, formula, proto, centroid, ptree );
                                        writer->attach< adcontrols::Targeting >( att, targeting, adcontrols::constants::F_TARGETING );
                                    }
                                }
                            }
                        }
                    }
                }
                return boost::uuids::string_generator()( file.name() );
            }
            return { 0 };
        }
    };

    /////////////////////////
    /////////////////////////    

    struct response_builder {

        static void build( QuanSampleProcessor& processor
                           , adcontrols::QuanSample& sample
                           , const std::vector< std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult> > >& rlist
                           , const adcontrols::QuanCompounds& compounds
                           , const boost::uuids::uuid& folderGuid ) {
            
            for ( auto& pair: rlist ) {
                auto matchedMass = pair.first->ptree().get_optional< double >( "targeting.matchedMass" );
                
                if ( auto child = pair.first->ptree().get_child_optional( "generator.extract_by_mols" ) ) {

                    if ( auto cmpdGuid = child.get().get_optional< boost::uuids::uuid >( "molid" ) ) { // "generator.extract_by_mols.molid"

                        if ( auto mol = child.get().get_child_optional( "moltable" ) ) {               // "generator.extract_by_mols.moltable"
                        
                            // lookup c-peak
                            if ( auto formula = mol.get().get_optional< std::string >( "formula" ) ) {
                                adcontrols::QuanResponse resp;
                                resp.uuid_cmpd( cmpdGuid.get() );  // compound id (uuid) identify each molecule(formula) and protocol
                                resp.uuid_cmpd_table( compounds.uuid() );
                                resp.dataGuid_ = boost::lexical_cast< std::wstring >( folderGuid );   // corresponding chromatogram data on output adfs file
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
                                auto& pt = pair.first->ptree();
                                pt.put( "resp.uuid_cmpd", resp.uuid_cmpd() );
                                pt.put( "resp.uuid_cmpd_table", resp.uuid_cmpd_table() );
                                pt.put( "resp.dataGuid", folderGuid );
                                pt.put( "resp.mass", resp.mass_ );
                                pt.put( "resp.idx", resp.idx_ );
                                pt.put( "resp.fcn", resp.fcn_ );
                                pt.put( "resp.intensity", resp.intensity_ );
                                pt.put( "resp.tR", resp.tR_ );
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
            for ( auto& cm: cXmethods_ ) {
                cm->setLockmass( lkm->enabled() );
                cm->setLockmass( lkm->enabled() );
            }
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


//////// main entry point ////////
// new interface as of 2018-MAY //
//////////////////////////////////
bool
QuanChromatogramProcessor::operator()( QuanSampleProcessor& processor
                                       , adcontrols::QuanSample& sample
                                       , std::shared_ptr< QuanDataWriter > writer
                                       , std::shared_ptr< adwidgets::Progress > progress )
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
                do {
                    adcontrols::ProcessMethod pm( *procm_ );
                    pm *= (*cXmethods_[ idx ]);
                    std::vector< std::shared_ptr< adcontrols::Chromatogram > > clist;
                    extractor->extract_by_mols( clist, pm, reader, [progress]( size_t, size_t )->bool{ (*progress)(); } );
                    std::transform( clist.begin(), clist.end(), std::back_inserter( rlist ), []( auto p ){ return std::make_pair( std::move( p ), std::make_shared< adcontrols::PeakResult >() ); });
                } while (0);

                if ( auto peakm = procm_->find< adcontrols::PeakMethod >() ) {
                    for ( auto& pair: rlist ) {
                        if ( findPeaks( *pair.second, *pair.first, *peakm ) ) {
                            identify( *pair.second, *pCompounds, *pair.first );
                        }
                        pair.first->setBaselines( pair.second->baselines() );
                        pair.first->setPeaks( pair.second->peaks() );
                    }
                }
                
                for ( auto& pair: rlist ) {
                    for ( auto& pk: pair.second->peaks() ) {
                        if ( !pk.name().empty() ) {
                            auto& chr = pair.first;
                            if ( auto ms = extractor->getMassSpectrum( pk.peakTime() ) ) {
                                auto title = save_spectrum::make_title( sample.dataSource(), pk.formula(), pk.peakTime(), (idx == 0 ? L" (profile)" : L" (histogram)" ) );
                                auto uuid = save_spectrum::save( writer, sample.dataSource(), ms, title, procm_, pk.formula(), chr->protocol(), chr->ptree() );
                            }
                        }
                    }
                }
                auto fGuid = save_chromatograms::save( writer, sample.dataSource(), rlist, procm_, (idx == 0 ? L" (profile)" : L" (histogram)" ) );
                response_builder::build( processor, sample, rlist, *pCompounds, fGuid );
                ++idx;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void
QuanChromatogramProcessor::save_candidate_chromatograms( std::shared_ptr< QuanDataWriter > writer
                                                         , const wchar_t * dataSource
                                                         , std::shared_ptr< const QuanChromatograms > chromatograms
                                                         , const wchar_t * title_trailer )
{
    for ( auto & chro : *chromatograms ) {
                
        std::string formula = chro->formula();
        auto range = chro->msrange();
        std::wstring title = make_title( dataSource, formula, chro->fcn(), range.second - range.first, title_trailer );
                
        if ( chro ) {

            if ( adfs::file file = writer->write( *chro->chromatogram(), title ) ) {

                chro->setDataGuid( file.name() ); // dataGuid for Chromatogarm on adfs
                
                if ( auto pkinfo = chro->peakResult() ) {
                    
                    auto afile = writer->attach< adcontrols::PeakResult >( file, *pkinfo, pkinfo->dataClass() );
                    writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );

                }
            }
        }

    }
}

std::wstring
QuanChromatogramProcessor::save_spectrum( std::shared_ptr< QuanDataWriter > writer
                                          , const QuanCandidate& candidate
                                          , const std::wstring& title )
{
    std::wstring dataGuid;
        
    if ( auto profile = candidate.profile() ) {

        writer->remove( title, L"/Processed/Spectra" );

        if ( adfs::file file = writer->write( *profile, title ) ) {
            if ( auto filtered = candidate.filtered() ) {
                writer->attach< adcontrols::MassSpectrum >( file, *filtered, dataproc::Constants::F_DFT_FILTERD );
            }
            if ( auto centroid = candidate.centroid() ) {
                auto afile = writer->attach< adcontrols::MassSpectrum >( file, *centroid, dataproc::Constants::F_CENTROID_SPECTRUM );
                writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
            }
            if ( auto pkinfo = candidate.mspkinfo() ) {
                writer->attach< adcontrols::MSPeakInfo >( file, *pkinfo, dataproc::Constants::F_MSPEAK_INFO );
            }
            dataGuid = file.name();
        }
    }
    return dataGuid;
}
        
//static
bool
QuanChromatogramProcessor::doCentroid( const adcontrols::MassSpectrum& profile
                        , const adcontrols::ProcessMethod& pm
                        , std::shared_ptr< adcontrols::MSPeakInfo >& pkInfo
                        , std::shared_ptr< adcontrols::MassSpectrum >& centroid
                        , std::shared_ptr< adcontrols::MassSpectrum >& filtered )
{
    pkInfo = std::make_shared< adcontrols::MSPeakInfo >();
    centroid = std::make_shared< adcontrols::MassSpectrum >();
    filtered.reset();

    if ( auto pCentroidMethod = pm.find< adcontrols::CentroidMethod >() ) {

        if ( pCentroidMethod->noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter ) {

            auto filtered = std::make_shared< adcontrols::MassSpectrum >();
            filtered->clone( profile, true );

            for ( auto& ms : adcontrols::segment_wrapper<>( *filtered ) ) {
                adcontrols::waveform_filter::fft4c::lowpass_filter( ms, pCentroidMethod->cutoffFreqHz() );
            }

            filtered->addDescription( adcontrols::description( L"process", dataproc::Constants::F_DFT_FILTERD ) );
            return QuanSampleProcessor::doCentroid( *pkInfo, *centroid, *filtered, *pCentroidMethod );

        } else {
            return QuanSampleProcessor::doCentroid( *pkInfo, *centroid, profile, *pCentroidMethod );
        }
    }
    return false;
}


void
QuanChromatogramProcessor::doit( QuanSampleProcessor& processor
                                 , adcontrols::QuanSample& sample
                                 , std::shared_ptr< QuanDataWriter > writer
                                 , const std::string& reader_objtext
                                 , std::shared_ptr< adwidgets::Progress > progress )
{
    bool save_on_datasource = false;

    if ( auto qm = procm_->find< adcontrols::QuanMethod >() ) {
        debug_level_ = qm->debug_level();
        save_on_datasource_ = qm->save_on_datasource();
    }

    if ( reader_objtext.find( "histogram" ) != std::string::npos ) {
        // counting data reader specified
        doCountingChromatogram( processor, sample, writer, reader_objtext, progress );
        
    } else {
        // profile data reader specified
        doProfileChromatogram( processor, sample, writer, reader_objtext, progress );
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

                pk->setFormula( formula.get().c_str() );
                pk->setName( adcontrols::ChemicalFormula::formatFormula( pk->formula() ) );
                
                // next candidate
                std::advance( cmpd, 1 );
                cmpd = std::find_if( cmpd, compounds.end(), [&]( auto& a ) { return a.uuid() == uuid.get(); } );
            }
        }
    }
}

void
QuanChromatogramProcessor::doCountingChromatogram( QuanSampleProcessor& processor
                                                   , adcontrols::QuanSample& sample
                                                   , std::shared_ptr< QuanDataWriter > writer
                                                   , const std::string& reader_objtext
                                                   , std::shared_ptr< adwidgets::Progress > progress )
{
    std::vector< std::shared_ptr< QuanTarget > > targets;
    std::vector< QuanCandidate > candidates;
    
    if ( auto compounds = procm_->find< adcontrols::QuanCompounds >() ) {
        for ( auto& comp : *compounds ) {
            std::string formula( comp.formula() );
            if ( !formula.empty() && comp.isCounting() )
                targets.emplace_back( std::make_shared< QuanTarget >( formula ) );
        }
    }
    
    do { // first phase
        double mass_width = 0.010; // 10mDa default
        if ( auto targeting_method = procm_->find< adcontrols::TargetingMethod >() ) {
            mass_width = targeting_method->tolerance( targeting_method->toleranceMethod() ) * 2.5; // experimental 
        }

        double tolerance = 0; // tolerance = 0 ==> single chromatogram per target (no parallel)
        std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v1; // array of enum of chromatogram
        find_parallel_chromatograms( qcrms_v1, targets, reader_objtext, mass_width, tolerance );  

        for ( auto& qcrms : qcrms_v1 ) {
            save_candidate_chromatograms( writer, sample.dataSource(), qcrms, L"(counting)" );
        }

    } while ( 0 );
}


void
QuanChromatogramProcessor::doProfileChromatogram( QuanSampleProcessor& processor
                                                  , adcontrols::QuanSample& sample
                                                  , std::shared_ptr< QuanDataWriter > writer
                                                  , const std::string& reader_objtext
                                                  , std::shared_ptr< adwidgets::Progress > progress )
{
#if 0
    std::vector< std::shared_ptr< QuanTarget > > targets;
    std::vector< QuanCandidate > candidates;
    
    if ( auto compounds = procm_->find< adcontrols::QuanCompounds >() ) {
        for ( auto& comp : *compounds ) {
            std::string formula( comp.formula() );
            if ( !formula.empty() && !comp.isCounting() )
                targets.emplace_back( std::make_shared< QuanTarget >( formula ) );
        }
    }

    double mass_width = 0.005; // 5mDa default
    do { // first phase
        if ( auto targeting_method = procm_->find< adcontrols::TargetingMethod >() ) {
            mass_width = targeting_method->tolerance( targeting_method->toleranceMethod() );
        }

        // extract candidate chromatograms for a target
        // find all peaks, and retention-time identification
        double tolerance = 0; // tolerance = 0 ==> single chromatogram per target (no parallel)
        std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v1; // array of enum of chromatogram
        find_parallel_chromatograms( qcrms_v1, targets, reader_objtext, mass_width, tolerance );  

        if ( debug_level_ == 4 || debug_level_ >= 6 ) { // 4 || 6
            std::wstring trailer = L"(1st phase),waveform";
            for ( auto& qcrms : qcrms_v1 )
                save_candidate_chromatograms( writer, sample.dataSource(), qcrms, trailer.c_str() );
        }

        for ( auto& qcrms: qcrms_v1 ) {
            qcrms->refine_chromatograms( reader_objtext, candidates, [=]( int64_t rowid ){
                    auto it = spectra_.find( rowid );
                    return ( it != spectra_.end() ) ? it->second : QuanChromatograms::spectra_type();
                });
        }

        if ( debug_level_ == 2 || debug_level_ >= 6 ) {
            for ( auto& qcrms : qcrms_v1 ) {
                std::wstring trailer = L"(2nd phase),waveform";
                save_candidate_chromatograms( writer, sample.dataSource(), qcrms, trailer.c_str() );
            }
        }

    } while ( 0 );

    // ---------- run quantitative analysis process ----------------
    // process based on found mass -- QuanChromatograms now point single chromatogram
    std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v;
    for ( auto& candidate : candidates ) {
        qcrms_v.emplace_back( std::make_shared< QuanChromatograms >( candidate.formula(), candidate, mass_width ) );
    }

    // re-generate chromatograms
    for ( auto& sp : spectra_ ) {
        for ( auto& candidate : qcrms_v )
            candidate->append_to_chromatogram( sp.first, sp.second.profile, reader_objtext );
    }

    std::pair< double, double > time_range =
        std::make_pair( spectra_.begin()->second.profile->getMSProperty().timeSinceInjection()
                      , spectra_.rbegin()->second.profile->getMSProperty().timeSinceInjection() );

    for ( auto& qchro : qcrms_v ) {
        std::for_each( qchro->begin(), qchro->end(), [=] ( std::shared_ptr<QuanChromatogram> c ) {
                c->chromatogram()->minimumTime( time_range.first );
                c->chromatogram()->maximumTime( time_range.second );
            });
    }

    auto pCompounds = procm_->find< adcontrols::QuanCompounds >();
    for ( auto& qchro : qcrms_v ) {

        qchro->process_chromatograms( procm_ );
        if ( pCompounds )
            qchro->identify( pCompounds, procm_ ); // retention time id

        qchro->finalize( [=] ( uint32_t pos ) {    // m/z id (if not identified yet)
                auto it = spectra_.find( pos );
                if ( it != spectra_.end() ) return it->second; else return QuanChromatograms::spectra_type(); } );
    }

    // save reference spectra
    for ( auto& qchro : qcrms_v ) {
        if ( auto candidate = qchro->quanCandidate() ) {

            std::wstring title = make_title( sample.dataSource(), *candidate );
            std::wstring dataGuid = save_spectrum( writer, *candidate, title );
            if ( ! dataGuid.empty() ) {
                std::for_each( qchro->begin(), qchro->end(), [=] ( std::shared_ptr<QuanChromatogram> c ) {
                        c->setReferenceDataGuid( dataGuid, candidate->idx(), candidate->fcn() );
                    } );
            }
        }
    }

    // identify all
	if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {
        for ( auto& qchro: qcrms_v ) {
            std::wstring trailer = L"(final),waveform";
            save_candidate_chromatograms( writer, sample.dataSource(), qchro, trailer.c_str() );

            std::for_each( qchro->begin(), qchro->end(), [&] ( std::shared_ptr<QuanChromatogram> c ) {

                    std::string formula = c->formula();
                    auto itCmpd = std::find_if( pCompounds->begin(), pCompounds->end()
                                                , [c] ( const adcontrols::QuanCompound& cmpd ) { return cmpd.formula() == c->formula(); } );
                    if ( itCmpd != pCompounds->end() ) {

                        auto& peaks = c->peakResult()->peaks();
                        auto pk = std::find_if( peaks.begin(), peaks.end()
                                                , [c] ( const adcontrols::Peak& p ) {
                                                    return std::string( p.formula() ) == c->formula(); } );

                        if ( pk != peaks.end() ) {

                            adcontrols::QuanResponse resp;

                            resp.dataGuid_ = c->dataGuid();
                            resp.mass_ = c->matchedMass();
                            resp.formula( c->formula().c_str() );
                            resp.uuid_cmpd( itCmpd->uuid() );
                            resp.uuid_cmpd_table( pCompounds->uuid() );
                            resp.idx_ = pk->peakId();
                            resp.fcn_ = c->fcn();
                            resp.intensity_ = pk->peakArea();
                            resp.amounts_ = 0;
                            resp.tR_ = pk->peakTime(); // double( adcontrols::timeutil::toMinutes( pk->peakTime() ) );

                            sample << resp;
                        }
                    }
                });
        }
    }

    for ( auto& qchro : qcrms_v ) {
        for ( auto & c : *qchro )
            writer->insert_table( c->dataGuid(), c->referenceDataGuids() );
    }
    
    if ( save_on_datasource_ ) {
        if ( auto source = std::make_shared< QuanDataWriter >( sample.dataSource() ) ) {
            if ( source->open() ) {

                // save reference spectra
                for ( auto& qchro : qcrms_v ) {

                    if ( auto candidate = qchro->quanCandidate() )
                        save_spectrum( source, *candidate, make_title( sample.dataSource(), *candidate ) );

                    for ( auto& chro: *qchro ) {
                        
                        std::wstring title = make_title( sample.dataSource(), chro->formula(), chro->fcn(), chro->matchedMass() - chro->exactMass(), L"(final)" );
                        writer->remove( title, L"/Processed/Chromatograms" );
                        
                        if ( adfs::file file = writer->write( *chro->chromatogram(), title ) ) {
                            if ( auto pkinfo = chro->peakResult() ) {
                                auto afile = writer->attach< adcontrols::PeakResult >( file, *pkinfo, pkinfo->dataClass() );
                                writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}

void
QuanChromatogramProcessor::correct_baseline( adcontrols::MassSpectrum& profile )
{
    if ( profile.isCentroid() )
        return;
    adcontrols::segment_wrapper<> segments( profile );

    for ( auto& ms: segments ) {
        double dbase(0), rms(0), tic(0);
        const double * data = ms.getIntensityArray();
        tic = adportable::spectrum_processor::tic( static_cast< unsigned int >( ms.size() ), data, dbase, rms );

        for ( size_t idx = 0; idx < ms.size(); ++idx )
            ms.setIntensity( idx, data[ idx ] - dbase );
    }
}

bool
QuanChromatogramProcessor::doMSLock( adcontrols::MassSpectrum& profile )
{
    //--------- centroid --------
    auto centroid = std::make_shared< adcontrols::MassSpectrum >();

    if ( auto m = procm_->find< adcontrols::CentroidMethod >() ) {

        adcontrols::CentroidProcess peak_detector;

        adcontrols::segment_wrapper<> segments( profile );
        
        centroid->clone( profile, false );

        if ( peak_detector( *m, profile ) ) {
            peak_detector.getCentroidSpectrum( *centroid );

            for ( auto fcn = 0; fcn < profile.numSegments(); ++fcn ) {
                auto cseg = std::make_shared< adcontrols::MassSpectrum >();
                peak_detector( profile.getSegment( fcn ) );
                peak_detector.getCentroidSpectrum( *cseg );
                *centroid << std::move( cseg ); 
            }

        }
    }
    
    //--------- lockmass --------
    // this does not find reference from segments attached to 'centroid'
    if ( centroid ) {
        
        adcontrols::MSFinder find( mslockm_->tolerance( mslockm_->toleranceMethod() ), mslockm_->algorithm(), mslockm_->toleranceMethod() );

        adcontrols::segment_wrapper<> segments( *centroid );
        
        if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {
            for ( auto& compound: *pCompounds ) {

                if ( compound.isLKMSRef() ) {
                    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( compound.formula() );                    

                    for ( auto& fms: segments ) {
                        if ( fms.getMass( 0 ) < exactMass && exactMass < fms.getMass( fms.size() - 1 ) ) {
                            auto idx = find( fms, exactMass );
                            if ( idx != adcontrols::MSFinder::npos ) {
                                *mslock_ << adcontrols::lockmass::reference( compound.formula(), exactMass, fms.getMass( idx ), fms.getTime( idx ) );
                            }
                        }
                    }
                }
            }
        }
    }
    if ( mslock_ )
        mslock_->fit();
    ( *mslock_ )( profile, true );
    return true;
}

void
QuanChromatogramProcessor::find_parallel_chromatograms( std::vector< std::shared_ptr< QuanChromatograms > >& vec
                                                        , const std::vector< std::shared_ptr< QuanTarget > >& targets
                                                        , const std::string& reader_objtext
                                                        , double mspeak_width  // 0.002
                                                        , double tolerance )   // 0.010
                                                        
{
    for ( auto& t: targets ) {
        std::vector < QuanTarget::target_value > developped_target_values;
        t->compute_candidate_masses( mspeak_width, tolerance, developped_target_values );
        vec.emplace_back( std::make_shared< QuanChromatograms >( t->formula(), developped_target_values, reader_objtext ) );
    }

    for ( auto& sp : spectra_ ) {
        for ( auto& candidate: vec )
            candidate->append_to_chromatogram( sp.first, sp.second.profile, reader_objtext );
    }

    auto pCompounds = procm_->find< adcontrols::QuanCompounds >();

    for ( auto& qchro: vec ) {
        qchro->process_chromatograms( procm_ );
        if ( pCompounds )
            qchro->identify( pCompounds, procm_ );
    }
}

