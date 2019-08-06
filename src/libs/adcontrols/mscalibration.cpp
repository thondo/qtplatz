// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "massspectrometer.hpp"
#include "mscalibration.hpp"
#include <adportable/date_string.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

using namespace adcontrols;

MSCalibration::MSCalibration() : calibrationUuid_( boost::uuids::random_generator()() )
                               , massSpectrometerClsid_( {{0}} )
                               , mode_( 0 )
{
    process_date_ = adportable::date_string::logformat( std::chrono::system_clock::now(), true );
}

MSCalibration::MSCalibration( const boost::uuids::uuid& clsid ) : calibrationUuid_( boost::uuids::random_generator()() )
                                                                , massSpectrometerClsid_( clsid )
                                                                , mode_( 0 )
{
    process_date_ = adportable::date_string::logformat( std::chrono::system_clock::now(), true );
    assert( massSpectrometerClsid_ != boost::uuids::uuid{{0}} );
}


MSCalibration::MSCalibration( const MSCalibration& t ) : process_date_( t.process_date_ )
                                                       , mode_( t.mode_ )
                                                       , coeffs_( t.coeffs_ )
                                                       , calibrationUuid_( t.calibrationUuid_ )
                                                       , massSpectrometerClsid_( t.massSpectrometerClsid_ )
{
}

void
MSCalibration::setMassSpectrometerClsid( const boost::uuids::uuid& id )
{
    massSpectrometerClsid_ = id;
}

const boost::uuids::uuid&
MSCalibration::massSpectrometerClsid() const
{
    return massSpectrometerClsid_;
}

const boost::uuids::uuid&
MSCalibration::calibrationUuid() const
{
    return calibrationUuid_;
}

int32_t
MSCalibration::mode() const
{
    return mode_;
}

void
MSCalibration::setMode( int32_t value )
{
    mode_ = value;
}

const std::string&
MSCalibration::date() const
{
    return process_date_;
}


const std::vector< double >&
MSCalibration::coeffs() const
{
    return coeffs_;
}

void
MSCalibration::setCoeffs( const std::vector<double>& v )
{
    coeffs_ = v;
    assert( massSpectrometerClsid_ != boost::uuids::uuid{{0}} );
}

double
MSCalibration::compute_mass( double time ) const
{
    double sqrt = compute( coeffs_, time );
    if ( sqrt > 0 )
        return sqrt * sqrt;

    return 0;
}

// static
double
MSCalibration::compute( const std::vector<double>& v, double t )
{
	int n = 0;
	double sqmz = 0;
	for ( auto d: v )
		sqmz += d * std::pow( t, n++ );
	return sqmz;
}

std::string
MSCalibration::formulaText( bool ritchText ) const
{
    std::ostringstream o;

    if ( coeffs_.empty() )
        return "";

    if ( ritchText ) {
        o << "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span> = "
          << boost::format( "%.8le" ) % coeffs_[ 0 ];
        for ( size_t i = 1; i < coeffs_.size(); ++i )
            o << boost::format( " + %.8le &times; t<sup>%d</sup>" ) % coeffs_[ i ] % i;
    } else {
        o << "sqrt(m/z) = "
          << boost::format( "%.8le" ) % coeffs_[ 0 ];
        for ( size_t i = 1; i < coeffs_.size(); ++i )
            o << boost::format( " + %.8le * t^%d" ) % coeffs_[ i ] % i;
    }
    return o.str();
}
