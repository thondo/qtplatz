/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
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

#include "histogram.hpp"
#include "massspectrum.hpp"
#include "massspectrometerbroker.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include "scanlaw.hpp"
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <cmath>

using namespace adcontrols;

std::shared_ptr< MassSpectrum >
histogram::make_profile( const MassSpectrum& ms, const MassSpectrometer& sp )
{
    auto profile = std::make_shared< MassSpectrum >( ms );

    for ( auto& tgt: segment_wrapper<>( *profile ) )
        histogram_to_profile( tgt, sp );

    return profile;
}

void
histogram::histogram_to_profile( MassSpectrum& ms, const MassSpectrometer& spectrometer )
{
    auto& prop = ms.getMSProperty();
    auto& info = prop.samplingInfo();

    std::vector< double > counts, masses, times;  //( info.nSamples() ); // counts intensity array
    counts.reserve( info.nSamples() );
    masses.reserve( info.nSamples() );
    times.reserve( info.nSamples() );

    auto scanlaw = spectrometer.scanLaw();
    if ( !scanlaw )
        return;

    size_t x(0);
    std::generate( masses.begin(), masses.end(), [&]{ return scanlaw->getMass( prop.toSeconds( x++, info ), int( info.mode() ) ); } );

    for ( size_t i = 0; i < ms.size(); ++i ) {
        counts.emplace_back( ms.getIntensity( i ) );
        times.emplace_back( ms.getTime( i ) );
        masses.emplace_back( ms.getMass( i ) );
    }

    ms.setCentroid( CentroidNone );
    ms.setMassArray( std::move( masses ) );
    ms.setTimeArray( std::move( times ) );
    ms.setIntensityArray( std::move( counts ) );
}



