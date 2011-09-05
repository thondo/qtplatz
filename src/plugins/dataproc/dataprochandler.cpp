// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dataprochandler.hpp"
#include <adcontrols/centroidprocess.hpp>

#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/isotopemethod.hpp>

#include <adcontrols/tableofelements.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/polfit.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>

#ifdef _DEBUG
#include <iomanip>
#include <fstream>
#include <adportable/string.hpp>
#endif

using namespace dataproc;

DataprocHandler::DataprocHandler()
{
}

bool
DataprocHandler::doCentroid( adcontrols::MassSpectrum& res, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    if ( peak_detector( m, profile ) )
        return peak_detector.getCentroidSpectrum( res );
    return false;
}

bool
DataprocHandler::doIsotope( adcontrols::MassSpectrum& res, const adcontrols::IsotopeMethod& m )
{
    adcontrols::ChemicalFormula chemicalFormula; 

    adcontrols::IsotopeCluster cluster;
    cluster.clearFormulae();

    double ra = res.getMaxIntensity();
    if ( ra <= 1.0 )
        ra = 100;

    if ( m.size() ) {

        for ( adcontrols::IsotopeMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it ) {
            std::wstring stdFormula = chemicalFormula.standardFormula( it->formula );
            cluster.addFormula( stdFormula, it->adduct, it->chargeState, it->relativeAmounts );
        }

        res.resize(0);  // clear peaks
        size_t nPeaks(0);
        if ( cluster.computeFormulae( m.threshold(), true, m.resolution(), res, nPeaks, m.useElectronMass(), ra / 100 ) ) {
            std::pair<double, double> mrange = res.getAcquisitionMassRange();
            if ( nPeaks && mrange.second <= 1.0 ) {
                mrange.second = res.getMassArray()[ res.size() - 1 ];
                mrange.first = res.getMassArray()[ 0 ];
                double d = mrange.second - mrange.first;
                mrange.first -= d / 10;
                mrange.second += d / 10;
                res.setAcquisitionMassRange( mrange.first, mrange.second );
            }
            return true;
        }

    }
    return false;
}

bool
DataprocHandler::doMSCalibration( adcontrols::MSCalibrateResult& res
                                 , const adcontrols::MassSpectrum& centroid
                                 , const adcontrols::MSCalibrateMethod& m )
{
    using adcontrols::MSProperty;

    res.calibration( centroid.calibration() );
    res.references( m.references() );
    double tolerance = m.massToleranceDa();
    double hMAThreshold = centroid.getMaxIntensity() * m.minimumRAPercent() / 100;

    adportable::array_wrapper<const double> masses( centroid.getMassArray(), centroid.size() );
    adportable::array_wrapper<const double> intens( centroid.getIntensityArray(), centroid.size() );
    boost::scoped_array< double > times( new double [ centroid.size() ] );
    centroid.compute_profile_time_array( times.get(), centroid.size() );

    //const double * times = centroid.getTimeArray();

    std::vector< unsigned char > colors( centroid.size() );
    memset( &colors[0], 0, colors.size() * sizeof( unsigned char ) );

    std::vector< std::pair< unsigned int, adcontrols::MSReference > > calibPoints;
    size_t idReference(0);
    for ( adcontrols::MSReferences::vector_type::const_iterator it = res.references().begin(); it != res.references().end(); ++it ) {
        const adcontrols::MSReference& ref = *it;
        if ( ! ref.enable() )
            continue;

        double exactMass = ref.exactMass();
        adportable::array_wrapper<const double>::const_iterator lBound = std::lower_bound( masses.begin(), masses.end(), exactMass - tolerance );
        adportable::array_wrapper<const double>::const_iterator uBound = std::lower_bound( masses.begin(), masses.end(), exactMass + tolerance );

        if ( lBound != masses.end() ) {

            size_t lIdx = std::distance( masses.begin(), lBound );
            size_t uIdx = std::distance( masses.begin(), uBound );

            // find closest
            size_t cIdx = lIdx;
            for ( size_t i = lIdx + 1; i < uIdx; ++i ) {
                double d0 = std::abs( masses[ cIdx ] - exactMass );
                double d1 = std::abs( masses[ i ] - exactMass );
                if ( d1 < d0 )
                    cIdx = i;
            }

            // find highest
            adportable::array_wrapper<const double>::const_iterator hIt = std::max_element( intens.begin() + lIdx, intens.begin() + uIdx );
            if ( *hIt < hMAThreshold )
                continue;

            size_t idx = std::distance( intens.begin(), hIt );
            /*
            if ( idx != cIdx )
                idx = cIdx; // take closest, this is a workaround for Xe spectrum
                */
            
            colors[ idx ] = ref.enable() ? 1 : 0;
            adcontrols::MSAssignedMass assigned( idReference, idx, it->formula(), it->exactMass(), times[ idx ], masses[ idx ] ); 
            res.assignedMasses() << assigned;
            if ( ref.enable() )
                calibPoints.push_back( std::make_pair( idx, *it ) );
        }
        ++idReference;
    }

    do {
        std::vector<double> tmvec, msvec, coeffs;
        for ( size_t i = 0; i < calibPoints.size(); ++i ) {
            msvec.push_back( sqrt( calibPoints[i].second.exactMass() ) );
            tmvec.push_back( times[ calibPoints[i].first ] );
        }
        size_t nterm = m.polynomialDegree() + 1;
        if ( adportable::polfit::fit( &tmvec[0], &msvec[0], tmvec.size(), nterm, coeffs ) ) {
            adcontrols::MSCalibration calib;
            calib.coeffs( coeffs );
            res.calibration( calib );

            for ( adcontrols::MSAssignedMasses::vector_type::iterator it = res.assignedMasses().begin(); it != res.assignedMasses().end(); ++it ) {
                double t = it->time();
                double mq = adcontrols::MSCalibration::compute( coeffs, t );
                it->mass( mq * mq );
            }
        }

        if ( calibPoints.size() == 2 ) {
            double m1 = calibPoints[0].second.exactMass();
            double m2 = calibPoints[1].second.exactMass();
            double t1 = times[ calibPoints[0].first ];
            double t2 = times[ calibPoints[1].first ];
            // theoretical calibration  [ sqrt(m) = a + b*t ]
            double b = ( std::sqrt( m2 ) - std::sqrt( m1 ) ) / ( t2 - t1 );
            double a = std::sqrt( m1 ) - b * t1;
            std::vector< double > coeffs;
            coeffs.push_back( a );
            coeffs.push_back( b );
            adcontrols::MSCalibration calib;
            calib.coeffs( coeffs );
            res.calibration( calib );
        }
        // ------------
    } while( 0 );

    const_cast< adcontrols::MassSpectrum& >( centroid ).setColorArray( &colors[0] );

    //////////////////////
#ifdef _DEBUG
    do {
        const adcontrols::MSReferences& ref = res.references();
        const adcontrols::MSAssignedMasses& assigned = res.assignedMasses();

        std::ofstream of( "massassign.txt" );
        //of << "#\tm/z(observed)\ttime(us)\tintensity\tformula,\tm/z(exact)\tm/z(calibrated)\terror(mDa)" << std::endl;
        of << "#,m/z(observed),time(us),intensity,formula,m/z(exact),error(mDa)" << std::endl;
        adcontrols::MSReferences::vector_type::const_iterator refIt = ref.begin();
        for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it, ++refIt ) {
            const adcontrols::MSAssignedMass& a = *it;
            of << std::setprecision(8)
                << a.idMassSpectrum() << "\t" // id
                << std::fixed      << centroid.getMass( a.idMassSpectrum() ) << "\t"      // m/z(observed)
                << std::scientific << centroid.getTime( a.idMassSpectrum() ) << "\t"      // tof
                << std::scientific << centroid.getIntensity( a.idMassSpectrum() ) << "\t" // intensity
                << std::setw(10)   << adportable::string::convert( a.formula() ) << "\t"  // formula
                << std::fixed      << it->exactMass() << "\t"                          // mass(exact)
                << std::fixed      << a.mass() << "\t"                                    // m/z(calibrated)
                << std::setprecision(1) << ( a.mass() - it->exactMass() ) * 1000       // error(mDa)
                << std::endl;
        }
        of << "#--------------------------- centroid peak list (#,mass,intensity)--------------------------" << std::endl;
        for ( size_t i = 0; i < centroid.size(); ++i ) {
            if ( centroid.getIntensity( i ) > hMAThreshold ) {
                
                double mq = adcontrols::MSCalibration::compute( res.calibration().coeffs(), centroid.getTime( i ) );
                double mass = mq * mq;

                of << i << "\t"
                    << std::setprecision(8) << centroid.getMass( i ) << "\t"
                    << std::setprecision(8) << mass << "\t"
                    << std::setprecision(1) << centroid.getIntensityArray()[i] << std::endl;
            }
        }
    } while(0);
#endif
    return true;
}

