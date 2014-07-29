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

#include "quansampleprocessor.hpp"
#include "quanprocessor.hpp"
#include "quandatawriter.hpp"
#include "quandocument.hpp"
#include "../plugins/dataproc/constants.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>
#include <adlog/logger.hpp>
#include <adutils/cpio.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>

using namespace quan;

QuanSampleProcessor::~QuanSampleProcessor()
{
}

QuanSampleProcessor::QuanSampleProcessor( QuanProcessor * processor
                                        , std::vector< adcontrols::QuanSample >& samples )
                                        : raw_( 0 )
                                        , samples_( samples )
                                        , procmethod_( processor->procmethod() )
                                        , cformula_( std::make_shared< adcontrols::ChemicalFormula >() )
                                        , processor_( processor->shared_from_this() )
{
    if ( !samples.empty() )
        path_ = samples[ 0 ].dataSource();
}

QuanProcessor *
QuanSampleProcessor::processor()
{
    return processor_.get();
}

bool
QuanSampleProcessor::operator()( std::shared_ptr< QuanDataWriter > writer )
{
    open();

    for ( auto& sample : samples_ ) {

        switch ( sample.dataGeneration() ) {

        case adcontrols::QuanSample::GenerateChromatogram:
            break; // ignore for this version

        case adcontrols::QuanSample::GenerateSpectrum:
            if ( raw_ ) {
                adcontrols::MassSpectrum ms;
                if ( generate_spectrum( raw_, sample, ms ) ) {
                    processIt( sample, ms, writer.get() );
                }
            }
            break;
        case adcontrols::QuanSample::ASIS:
            do {
                if ( auto folder = portfolio_->findFolder( L"Spectra" ) ) {
                    if ( auto folium = folder.findFoliumByName( sample.name() ) ) {
                        if ( fetch( folium ) ) {
                            adcontrols::MassSpectrumPtr ms;
                            if ( portfolio::Folium::get< adcontrols::MassSpectrumPtr >( ms, folium ) ) {
                                sample.name( folium.name().c_str() );
                                processIt( sample, *ms, writer.get() );
                                // writer->write( *ms, folium.name() );
                            }
                        }
                    }
                }
            } while ( 0 );
            break;
        }
    }
    QuanDocument::instance()->sample_processed( this );
    return true;
}


void
QuanSampleProcessor::open()
{
    try {
        datafile_.reset( adcontrols::datafile::open( path_, true ) );
        if ( datafile_ )
            datafile_->accept( *this );
    }
    catch ( ... ) { ADERROR() << boost::current_exception_diagnostic_information(); }
}

bool
QuanSampleProcessor::subscribe( const adcontrols::LCMSDataset& d )
{
    raw_ = &d;
    return true;
}

bool
QuanSampleProcessor::subscribe( const adcontrols::ProcessedDataset& d )
{
    portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
    return true;
}

bool
QuanSampleProcessor::fetch( portfolio::Folium& folium )
{
    try {
        folium = datafile_->fetch( folium.id(), folium.dataClass() );
        portfolio::Folio attachs = folium.attachments();
        for ( auto att : attachs ) {
            if ( att.empty() )
                fetch( att ); // recursive call make sure for all blongings load up in memory.
        }
    }
    catch ( std::bad_cast& ) {}
    return true;
}

size_t 
QuanSampleProcessor::read_first_spectrum( const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms, uint32_t tidx )
{
    size_t pos = 0;
    if ( tidx == uint32_t(-1) ) { // select last spectrum
        pos = raw->find_scan( -1, -1 ); // find last "safe" spectrum scan#
    } else {
        pos = raw->find_scan( tidx, -1 );
    }
    
    int idx, fcn, rep;
    if ( raw->index( pos, idx, fcn, rep ) ) {
        assert( fcn == 0 );
        if ( ( rep == 0 ) && raw->index( pos + 1, idx, fcn, rep ) )
            ++pos; // increment for skip rep = 0 data;
    }
    if ( raw->getSpectrum( -1, pos, ms ) ) {
        while ( raw->index( pos + 1, idx, fcn, rep ) && fcn == 0 ) {
            adcontrols::MassSpectrum a;
            if ( raw->getSpectrum( -1, ++pos, a ) )
                ms += a;
        }
        return pos + 1; // return next pos
    }
    return 0;
}

size_t 
QuanSampleProcessor::read_next_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms )
{
    int idx, fcn, rep;
    while ( raw->index( pos, idx, fcn, rep ) && (fcn != 0 || rep != 0) ) // find next protocol=0 aligned data
        ++pos;
    if ( fcn == 0 && rep == 0 ) {
        if ( raw->index( pos + 1, idx, fcn, rep ) ) 
            ++pos;
        if ( raw->getSpectrum( -1, pos, ms ) ) {
            while ( raw->index( pos + 1, idx, fcn, rep ) && fcn == 0 ) {
                adcontrols::MassSpectrum a;
                if ( raw->getSpectrum( -1, ++pos, a ) )
                    ms += a;
            }
            return pos + 1; // return next pos
        }
    }
    return 0;
}


bool 
QuanSampleProcessor::generate_spectrum( const adcontrols::LCMSDataset * raw
                                        , const adcontrols::QuanSample& sample
                                        , adcontrols::MassSpectrum& ms )
{

    auto range = std::make_pair( sample.scan_range_first(), sample.scan_range_second() );

    if ( range.first == uint32_t( -1 ) )
        return read_first_spectrum( raw, ms, range.first ) != 0;

    size_t pos = read_first_spectrum( raw, ms, range.first++ );
    if ( pos == 0 )
        return false; // no spectrum have read

    adcontrols::MassSpectrum a;
    while ( pos && range.first++ < range.second ) {
        if ( (pos = read_next_spectrum( pos, raw, a )) )
            ms += a;
    }
    return true;
}

void
QuanSampleProcessor::processIt( adcontrols::QuanSample& sample, adcontrols::MassSpectrum& profile, QuanDataWriter * writer )
{
    // save profile spectrum
    adfs::file file = writer->write( profile, sample.name() );

    if ( auto pCentroidMethod = procmethod_->find< adcontrols::CentroidMethod >() ) {

        adcontrols::MassSpectrum centroid;
        adcontrols::MSPeakInfo pkInfo;

        bool result(false);

        if ( pCentroidMethod->noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter ) {

            adcontrols::MassSpectrum filtered;
            adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( filtered );
            for ( auto& ms : segments ) {
                adcontrols::waveform::fft::lowpass_filter( ms, pCentroidMethod->cutoffFreqHz() );
                double base( 0 ), rms( 0 );
                const double * intens = ms.getIntensityArray();
                adportable::spectrum_processor::tic( uint32_t( ms.size() ), intens, base, rms );
                for ( size_t i = 0; i < ms.size(); ++i )
                    ms.setIntensity( i, intens[ i ] - base );
            }
            filtered.addDescription( adcontrols::Description( L"process", dataproc::Constants::F_DFT_FILTERD ) );

            writer->attach<adcontrols::MassSpectrum>( file, filtered, dataproc::Constants::F_DFT_FILTERD );

            result = doCentroid( pkInfo, centroid, filtered, *pCentroidMethod );

        } else {
            result = doCentroid( pkInfo, centroid, profile, *pCentroidMethod );
        }

        if ( result ) {

            // doMSLock if required.
            if ( auto pCompounds = procmethod_->find< adcontrols::QuanCompounds >() ) {
                if ( auto lkMethod = procmethod_->find< adcontrols::MSLockMethod >() ) {
                    if ( lkMethod->enabled() )
                        doMSLock( pkInfo, centroid, *lkMethod, *pCompounds );
                }
            }
            
            // Look up compounds
            if ( auto pCompounds = procmethod_->find< adcontrols::QuanCompounds >() ) {
                if ( auto pTgtMethod = procmethod_->find< adcontrols::TargetingMethod >() ) {
                    doMSFind( pkInfo, centroid, sample, *pCompounds, *pTgtMethod );
                }
            }

            auto afile = writer->attach< adcontrols::MassSpectrum >( file, centroid, dataproc::Constants::F_CENTROID_SPECTRUM );
            writer->attach< adcontrols::ProcessMethod >( afile, *procmethod_, L"ProcessMethod" );
            writer->attach< adcontrols::MSPeakInfo >( file, pkInfo, dataproc::Constants::F_MSPEAK_INFO );
            writer->attach< adcontrols::QuanSample >( file, sample, dataproc::Constants::F_QUANSAMPLE );
            writer->insert_table( sample );

        }
    }
}

bool
QuanSampleProcessor::doCentroid( adcontrols::MSPeakInfo& pkInfo
                                 , adcontrols::MassSpectrum& res
                                 , const adcontrols::MassSpectrum& profile
                                 , const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    bool result = false;
    
    res.clone( profile, false );
    
    if ( peak_detector( m, profile ) ) {
        result = peak_detector.getCentroidSpectrum( res );
        pkInfo = peak_detector.getPeakInfo();
    }
    
    if ( profile.numSegments() > 0 ) {
        for ( size_t fcn = 0; fcn < profile.numSegments(); ++fcn ) {
            adcontrols::MassSpectrum centroid;
            result |= peak_detector( profile.getSegment( fcn ) );
            pkInfo.addSegment( peak_detector.getPeakInfo() );
            peak_detector.getCentroidSpectrum( centroid );
            res.addSegment( centroid );
        }
    }
    return result;
}

bool
QuanSampleProcessor::doMSLock( adcontrols::MSPeakInfo& pkInfo // will override
                               , adcontrols::MassSpectrum& centroid // will override
                               , const adcontrols::MSLockMethod& m
                               , const adcontrols::QuanCompounds& compounds )
{
    // find reference peak by mass window
    adcontrols::lockmass mslock;

    // TODO: consider how to handle segmented spectrum -- current impl is always process first 
    adcontrols::MSFinder find( m.tolerance( m.toleranceMethod() ), m.algorithm(), m.toleranceMethod() );

    for ( auto& compound : compounds ) {
        if ( compound.isLKMSRef() ) {
            double exactMass = cformula_->getMonoIsotopicMass( compound.formula() );
            size_t idx = find( centroid, exactMass );
            if ( idx != adcontrols::MSFinder::npos ) {
                // add found peaks into mslock
                mslock << adcontrols::lockmass::reference( compound.formula(), exactMass, centroid.getMass( idx ), centroid.getTime( idx ) );
            }
        }
    }
    if ( mslock.fit() ) {
        // this apply to all fragment spectra
        mslock( centroid );
        mslock( pkInfo );
        return true;
    }
    return false;
}

bool
QuanSampleProcessor::doMSFind( adcontrols::MSPeakInfo& pkInfo
                               , adcontrols::MassSpectrum& centroid
                               , adcontrols::QuanSample& sample
                               , const adcontrols::QuanCompounds& compounds
                               , const adcontrols::TargetingMethod& mtgt )
{
    double tolerance = mtgt.tolerance( adcontrols::idToleranceDaltons );

    adcontrols::segment_wrapper< adcontrols::MSPeakInfo > vPkInfo( pkInfo );
    adcontrols::segment_wrapper<> segs( centroid );

    int channel = sample.channel();
    int fcn = 0;
    for ( auto& fms : segs ) {

        if ( channel != 0 && fcn != (channel - 1) ) {
            ++fcn;
            continue;
        }
        
        auto& info = vPkInfo[ fcn ];
        adcontrols::MSFinder find( tolerance, adcontrols::idFindLargest, adcontrols::idToleranceDaltons );
        
        for ( auto& compound : compounds ) {
            adcontrols::QuanResponse resp;
            
            double exactMass = cformula_->getMonoIsotopicMass( compound.formula() );
            
            size_t idx = find( fms, exactMass );
            if ( idx != adcontrols::MSFinder::npos ) {
                
                resp.compoundId_ = compound.uniqId();
                resp.formula( compound.formula() );
                resp.idx_ = int32_t(idx);
                resp.fcn_ = fcn;
                resp.mass_ = fms.getMass( idx );
                resp.intensity_ = fms.getIntensity( idx );
                resp.amounts_ = 0;
                resp.tR_ = 0;

                adcontrols::annotation anno( resp.formula(), resp.mass_, resp.intensity_, resp.idx_ );
                fms.get_annotations() << anno;

                (info.begin() + idx)->formula( resp.formula() );

                sample << resp;
            }
        }
        ++fcn;
    }
    return false;
}

