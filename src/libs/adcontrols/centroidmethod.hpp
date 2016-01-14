// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

	class ADCONTROLSSHARED_EXPORT CentroidMethod {
	public:

        enum eNoiseFilterMethod {
            eNoFilter
            , eDFTLowPassFilter
        };

		enum ePeakWidthMethod {
			ePeakWidthTOF
			, ePeakWidthProportional
			, ePeakWidthConstant
		};

        enum eAreaMethod {
            eAreaDa         // I * Da
            , eAreaTime       // I * ns
            , eWidthNormalized  // A / width
            , eAreaPoint      // assume data interval := 1
        };

		~CentroidMethod(void);
		CentroidMethod(void);
		CentroidMethod(const CentroidMethod &);
		CentroidMethod & operator = ( const CentroidMethod & rhs );

		bool operator == ( const CentroidMethod & rhs ) const;
		bool operator != ( const CentroidMethod & rhs ) const;

		double baselineWidth() const;
		double rsConstInDa() const;
		double rsPropoInPpm() const;
		double rsTofInDa() const;
		double rsTofAtMz() const;
		// double attenuation() const; not in use
		double peakCentroidFraction() const;
		ePeakWidthMethod peakWidthMethod() const;

		bool centroidAreaIntensity() const;
		void baselineWidth(double);
		void rsConstInDa(double);
		void rsPropoInPpm(double);
		void rsTofInDa(double);
		void rsTofAtMz(double);
		void attenuation(double);
		void peakWidthMethod(ePeakWidthMethod);
		void centroidAreaIntensity(bool);
		void peakCentroidFraction(double);

        eNoiseFilterMethod noiseFilterMethod() const;
        void noiseFilterMethod( eNoiseFilterMethod );
        double cutoffFreqHz() const;
        void cutoffFreqHz( double );

        eAreaMethod areaMethod() const;
        void areaMethod( eAreaMethod );

        bool processOnTimeAxis() const;
        void setProcessOnTimeAxis( bool );
        double rsInSeconds() const;
        void setRsInSeconds( double );

	private:
		double baselineWidth_; // depreicated
        double rsConstInDa_;
        double rsPropoInPpm_;
        double rsTofInDa_;
		double rsTofAtMz_;
        double attenuation_; // not in use
		bool bCentroidAreaIntensity_;
		double peakCentroidFraction_;
        ePeakWidthMethod peakWidthMethod_;

        eNoiseFilterMethod noiseFilterMethod_; // since v2.2.3
        double cutoffFreqHz_; // Hz

        // CLASS VERSION 3
        eAreaMethod areaMethod_; // since v2.7.5

        // CLASS VERSION 4
        bool processOnTimeAxis_;
        double rsInSeconds_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP(baselineWidth_) // deprecated
                & BOOST_SERIALIZATION_NVP(rsConstInDa_)
                & BOOST_SERIALIZATION_NVP(rsPropoInPpm_)
                & BOOST_SERIALIZATION_NVP(rsTofInDa_)
                & BOOST_SERIALIZATION_NVP(rsTofAtMz_)
                & BOOST_SERIALIZATION_NVP(attenuation_)
                & BOOST_SERIALIZATION_NVP(bCentroidAreaIntensity_)
                & BOOST_SERIALIZATION_NVP(peakCentroidFraction_);
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP(noiseFilterMethod_)
                    & BOOST_SERIALIZATION_NVP(cutoffFreqHz_)
                    ;
            }
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( areaMethod_ );
            }
            if ( version >= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( processOnTimeAxis_ );
                ar & BOOST_SERIALIZATION_NVP( rsInSeconds_ );
            }
        }

	};

}

BOOST_CLASS_VERSION( adcontrols::CentroidMethod, 4 )
