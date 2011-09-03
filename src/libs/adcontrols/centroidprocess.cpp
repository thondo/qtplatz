// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "centroidprocess.hpp"
#include "centroidmethod.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "mspeakinfoitem.hpp"
#include "description.hpp"
#include <adportable/spectrum_processor.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/moment.hpp>
#include <adportable/differential.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>

#include <vector>
#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <sstream>
#include <cmath>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/foreach.hpp>

// #define DEBUG_CENTROID_PROCESS

using namespace adcontrols;

namespace adcontrols {

    class MSPeakInfoItem;

	namespace internal {

        class CentroidProcessImpl {
        public:
            CentroidProcessImpl() {}
			void clear();
            void setup( const CentroidMethod& );
            void setup( const MassSpectrum& );
			void copy( MassSpectrum& );
			const CentroidMethod& method() const { return method_; }
            void findpeaks( const MassSpectrum& profile );

			// result
            std::vector< MSPeakInfoItem > info_;
		private:
			MassSpectrum clone_;
			CentroidMethod method_;
			Description desc_;
        public:
            MassSpectrum debug_profile_;
        };

	}
}

CentroidProcess::~CentroidProcess(void)
{
    delete pImpl_;
}

CentroidProcess::CentroidProcess(void) : pImpl_( new internal::CentroidProcessImpl() )
{
}

CentroidProcess::CentroidProcess( const CentroidMethod& method)
  : pImpl_( new internal::CentroidProcessImpl() )
{
	pImpl_->setup( method );
}

bool
CentroidProcess::operator()( const CentroidMethod& method, const MassSpectrum& profile )
{
	pImpl_->setup( method );
	return (*this)( profile );
}

bool
CentroidProcess::operator()( const MassSpectrum& profile )
{
    pImpl_->clear();
	pImpl_->setup( profile );
    pImpl_->findpeaks( profile );

    //const double * masses = profile.getMassArray();
    //const double * intens = profile.getIntensityArray();
/*
    for ( size_t i = 0; i < nSize; ++i ) {
        piItem = piInfo->Item[ i + 1 ];
        double mass = piItem->GetPeakAreaWeightedMass();
        double area = piItem->GetPeakArea();
        double height = piItem->GetPeakHeight();
        double hh = piItem->GetPeakWidthHH();
        long spos = piItem->GetPeakStartIndex();
        long epos = piItem->GetPeakEndIndex();

        it = std::lower_bound( it, masses.end(), mass );
        size_t tpos = std::distance( masses.begin(), it );

        double t1 = double( startDelay + tpos - 1 ) * sampInterval;
        double tt = t1 + sampInterval * ( mass - *(it - 1) ) / ( *it - *(it - 1) );
            // validation
#if defined _DEBUG && 0
            double cx(0);
            do { // centroid by time
                double base = piItem->GetBaselineStartIntensity() + 
                    ( piItem->GetBaselineStartMass() - piItem->GetBaselineEndIntensity() ) / 2;
                adportable::timeFunctor tof( startDelay, sampInterval );
                adportable::Moment< adportable::timeFunctor > moment( tof );
                
                double threshold = base + height * pImpl_->method().peakCentroidFraction();
                cx = moment.centerX( profile.getIntensityArray(), threshold, spos, tpos, epos );
            } while(0);

            do {
                const MSCalibration& calib = profile.calibration();
                double mz1 = std::pow( MSCalibration::compute( calib.coeffs(), tt ), 2 );
                double mz2 = std::pow( MSCalibration::compute( calib.coeffs(), cx ), 2 );
                double dm1 = std::abs( mz1 - mass ) * 1000;
                double dm2 = std::abs( mz2 - mass ) * 1000;
                assert( dm1 < 0.1 ); // 0.1mDa
                (void)dm2;
                // assert( dm2 < 0.1 );
            } while(0);
#endif
            pImpl_->info_.push_back( MSPeakInfoItem( mass, area, height, hh, tt ) );
        }
    }
*/
    return true;
}

bool
CentroidProcess::getCentroidSpectrum( MassSpectrum& ms )
{
	pImpl_->copy( ms );

#if defined DEBUG_CENTROID_PROCESS
    ms = pImpl_->debug_profile_;
    return true;
#endif

	size_t nSize;
	if ( pImpl_ && ( nSize = pImpl_->info_.size() ) ) {

		ms.resize( nSize );
        ms.setCentroid( adcontrols::CentroidPeakAreaWaitedMass );
		bool is_area = pImpl_->method().centroidAreaIntensity();

        std::pair<double, double> mrange = ms.getAcquisitionMassRange();

        for ( size_t i = 0; i < nSize; ++i ) {
			ms.setIntensity( i, is_area ? pImpl_->info_[i].area() : pImpl_->info_[i].height() );
            ms.setMass( i, pImpl_->info_[i].mass() );
            ms.setTime( i, pImpl_->info_[i].time() );
        }
        return true;
    }
    return false;
}

/////////////////////////

using namespace adcontrols::internal;

void
CentroidProcessImpl::clear()
{
	info_.clear();
}

void
CentroidProcessImpl::setup( const CentroidMethod& method )
{
    method_ = method;
	desc_ = adcontrols::Description( L"CentroidProcess", L"Centroid" );

	std::wostringstream o;
	boost::archive::xml_woarchive ar( o );
	ar << boost::serialization::make_nvp("CentroidMethod", method);
	desc_.xml( o.str() );
}

void
CentroidProcessImpl::setup( const MassSpectrum& profile )
{
    clone_.clone( profile, false ); // keep meta data
}

void
CentroidProcessImpl::copy( MassSpectrum& ms )
{
	ms.clone( clone_, false );
	ms.addDescription( desc_ );
}

void
CentroidProcessImpl::findpeaks( const MassSpectrum& profile )
{
    using adportable::differential;
    using adportable::spectrum_processor;

    // double base = 0, sd = 0;
    // double tic = spectrum_processor::tic( profile.size(), profile.getIntensityArray(), base, sd );

    boost::scoped_array< double > pY( new double [ profile.size() ] );

    spectrum_processor::smoozing( profile.size(), pY.get(), profile.getIntensityArray(), 32 );

    // typedef std::pair<int, int> index_pair;
    // std::vector< index_pair > peakindex;

    adportable::spectrum_peakfinder finder;
    if ( method_.peakWidthMethod() == CentroidMethod::ePeakWidthConstant ) {
        finder.width_method_ = adportable::spectrum_peakfinder::Constant;
        finder.peakwidth_ = method_.rsConstInDa();
    } else if ( method_.peakWidthMethod() == CentroidMethod::ePeakWidthProportional ) {
        finder.width_method_ = adportable::spectrum_peakfinder::Proportional;
        finder.peakwidth_ = method_.rsConstInDa();
    } else if ( method_.peakWidthMethod() == CentroidMethod::ePeakWidthTOF ) {
        finder.width_method_ = adportable::spectrum_peakfinder::TOF;
        finder.peakwidth_ = method_.rsTofInDa();
        finder.atmz_ = method_.rsTofAtMz();
    }
    finder( profile.size(), profile.getMassArray(), pY.get() ); // profile.getIntensityArray() );

#if defined DEBUG_CENTROID_PROCESS
    debug_profile_ = profile;
    // debug_profile_.setIntensityArray( &finder.pdebug_[0] );
#endif

    adportable::array_wrapper<const double> intens( profile.getIntensityArray(), profile.size() );
    adportable::array_wrapper<const double> masses( profile.getMassArray(), profile.size() );

    BOOST_FOREACH( adportable::peakinfo& pk, finder.results_ ) {
        adportable::array_wrapper<const double>::iterator it = 
            std::max_element( intens.begin() + pk.first, intens.begin() + pk.second );
        double h = *it - pk.base;
        double a = adportable::spectrum_processor::area( intens.begin() + pk.first, intens.begin() + pk.second, pk.base );

        size_t idx = std::distance( intens.begin(), it );
        double t = ( profile.getMSProperty().instSamplingInterval() * ( profile.getMSProperty().instSamplingStartDelay() + idx ) ) * 1.0e12;

        double cx(0);
        do { // centroid by time
            double base = pk.base;
            adportable::massArrayFunctor mass_array( profile.getMassArray(), profile.size() );
            adportable::Moment< adportable::massArrayFunctor > moment( mass_array );

            double threshold = base + h * method_.peakCentroidFraction();
            cx = moment.centerX( profile.getIntensityArray(), threshold, pk.first, idx, pk.second );
        } while(0);

        // MSPeakInfoItem item( idx, masses[idx], a, h, 0 /* width */, t );
        MSPeakInfoItem item( idx, cx, a, h, 0 /* width */, t );

        if ( h > 20000 )
            adportable::debug() << "centroid result: " << cx << " error: " << double( cx - masses[idx] ) * 1000;

        item.peak_start_index( pk.first );
        item.peak_end_index( pk.second );
        item.base_height( pk.base );

        info_.push_back( item );

#if defined DEBUG_CENTROID_PROCESS
        debug_profile_.setIntensity( pair.first, 50000 );
        debug_profile_.setIntensity( pair.second, 25000 );
#endif
    }
}
