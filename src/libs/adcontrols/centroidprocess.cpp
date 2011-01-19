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

#include "centroidprocess.h"
#include "centroidmethod.h"
#include "import_sacontrols.h"
#include "samassspectrum.h"
#include "massspectrum.h"
#include "msproperty.h"
#include "mspeakinfoitem.h"
#include "description.h"
#include <adportable/array_wrapper.hpp>
#include <vector>
#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <sstream>
#include <cmath>
#include <adportable/moment.hpp>

/*
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
*/
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>


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

			// result
            std::vector< MSPeakInfoItem > info_;
		private:
			MassSpectrum clone_;
			CentroidMethod method_;
			Description desc_;
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

	CComPtr<SACONTROLSLib::ISAMSPeakDetect2> pi;
	if ( pi.CoCreateInstance( SACONTROLSLib::CLSID_SAMSPeakDetect ) != S_OK )
		return false;
	setup( pi, pImpl_->method() );

    CComPtr<SACONTROLSLib::ISAMassSpectrum5> pims;
    if ( pims.CoCreateInstance( SACONTROLSLib::CLSID_SAMassSpectrum ) != S_OK )
        return false;

    internal::SAMassSpectrum::copy( pims, profile );
    do {
        try {
            CComQIPtr< SACONTROLSLib::ISAMassSpectrum > piMS = pims;
            if ( pi->Detect( piMS ) != S_OK )
                return false;
        } catch ( _com_error & ) {
            return false;
        }
    } while(0);

	CComPtr< SACONTROLSLib::ISAMSPeakInformation2 > piInfo = pi->PeakInformation2;
    size_t nSize = piInfo->Count;

    if ( nSize ) {
		CComPtr< SACONTROLSLib::ISAMSPeakInformationItem2 > piItem;

        adportable::array_wrapper<const double> masses( profile.getMassArray(), profile.size() );
        adportable::array_wrapper<const double>::const_iterator it = masses.begin();
        const double sampInterval = profile.getMSProperty().instSamplingInterval() * 1.0e-12;  // ps -> s
        const unsigned long startDelay = profile.getMSProperty().instSamplingStartDelay();
        
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
#ifdef _DEBUG
            double cx;
            do { // centroid by time
                adportable::timeFunctor tof( startDelay, sampInterval );
                adportable::Moment< adportable::timeFunctor > moment( tof, profile.getIntensityArray() );
                // todo: hight recalculate if base below zero level
                cx = moment.centerX( height * pImpl_->method().peakCentroidFraction(), spos, tpos, epos );
            } while(0);

            do {
                const MSCalibration& calib = profile.calibration();
                double mz1 = std::pow( MSCalibration::compute( calib.coeffs(), tt ), 2 );
                // double mz2 = std::pow( MSCalibration::compute( calib.coeffs(), cx ), 2 );
                double dm1 = std::abs( mz1 - mass ) * 1000;
                // double dm2 = std::abs( mz2 - mass ) * 1000;
                assert( dm1 < 0.1 ); // 0.1mDa
            } while(0);
#endif
            pImpl_->info_.push_back( MSPeakInfoItem( mass, area, height, hh, tt ) );
        }
    }
    return true;
}

bool
CentroidProcess::getCentroidSpectrum( MassSpectrum& ms )
{
	pImpl_->copy( ms );

	size_t nSize;
	if ( pImpl_ && ( nSize = pImpl_->info_.size() ) ) {

		ms.resize( nSize );
        ms.setCentroid( adcontrols::CentroidPeakAreaWaitedMass );
		bool is_area = pImpl_->method().centroidAreaIntensity();

        for ( size_t i = 0; i < nSize; ++i ) {
			ms.setIntensity( i, is_area ? pImpl_->info_[i].area() : pImpl_->info_[i].height() );
            ms.setMass( i, pImpl_->info_[i].mass() );

            // tof recalculation

        }
        return true;
    }
    return false;
}

void
CentroidProcess::setup( SACONTROLSLib::ISAMSPeakDetect* pi, const CentroidMethod& method )
{
	if ( method.centroidAreaIntensity() )
		pi->SetCentroidAreaIntensity();
	else
		pi->SetCentroidHeightIntensity();

	switch(method.peakWidthMethod()) {
	case CentroidMethod::ePeakWidthTOF:
		pi->SetTOFPeakWidth( method.rsTofInDa(), method.rsTofAtMz() );
		break;
	case CentroidMethod::ePeakWidthProportional:
		pi->SetppmPeakWidth( method.rsPropoInPpm() );
		break;
	case CentroidMethod::ePeakWidthConstant:
		pi->SetDaPeakWidth( method.rsConstInDa() );
		break;
	}
	pi->SetBaselineWidth( method.baselineWidth() );
	pi->SetAttenuation( method.attenuation() );
	pi->SetPeakCentroidFraction( method.peakCentroidFraction() );
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