/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "waveform.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include <adportable/array_wrapper.hpp>
#include <adportable/fft.hpp>
#include <adportable/float.hpp>
#include <vector>
#include <complex>
#include <algorithm>

using namespace adcontrols;

extern "C" {
    // Takuya OOURA's package, built on adportable static library
    void cdft( int, int isgn, double * );
    void rdft( int, int isgn, double * );    
}

waveform::waveform()
{
}

bool
waveform::fft::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    if ( ms.isCentroid() || ms.size() < 32 )
        return false;

	size_t N = 32;
    while ( N < ms.size() )
		N *= 2;

	const size_t NN = ms.size();

	double sampInterval = ms.getMSProperty().getSamplingInfo().fSampInterval(); // seconds
    if ( sampInterval == 0 )
        sampInterval = ( ms.getTime( ms.size() - 1 ) - ms.getTime( 0 ) ) / ms.size();

    const double T = N * sampInterval;  // time full scale in seconds.  Freq = n/T (Hz)

    // power spectrum has N/2 points and is n/T Hz horizontal axis  := data[N/2] = (N/2)/T Hz
    size_t cutoff = size_t( T * freq );

	adportable::array_wrapper<const double> pIntens( ms.getIntensityArray(), N );

	std::vector< std::complex<double> > spc( N );
	std::vector< std::complex<double> > fft( N );
	size_t n;
	for ( n = 0; n < N && n < NN; ++n )
		spc[ n ] = std::complex<double>( pIntens[ n ] );

	while ( n < N )
		spc[ n++ ] = pIntens[ NN - 1 ];

	adportable::fft::fourier_transform( fft, spc, false );

    // appodization
    for ( size_t i = cutoff; i < N - cutoff; ++i )
        fft[ i ] = 0;

	adportable::fft::fourier_transform( spc, fft, true );

	std::vector<double> data( N );
	for ( size_t i = 0; i < NN; ++i )
		data[ i ] = spc[i].real();

	ms.setIntensityArray( &data[0] );

	return true;
}

bool
waveform::fft::lowpass_filter( std::vector<double>& intens, double sampInterval, double freq )
{
    if ( intens.size() < 32 )
        return false;

	size_t N = 32;
    while ( N < intens.size() )
		N *= 2;

    const double T = N * sampInterval;  // time full scale in seconds.  Freq = n/T (Hz)

    const size_t cutoff = size_t( T * freq );

	std::vector< std::complex<double> > spc( N );

	std::vector< std::complex<double> > fft( N );
    
    std::copy( intens.begin(), intens.begin() + intens.size(), spc.begin() );

    std::fill( spc.begin() + intens.size(), spc.end(), intens.back() );

	adportable::fft::fourier_transform( fft, spc, false );

    // appodization
    std::fill( fft.begin() + cutoff, fft.begin() + ( N - cutoff ), 0 );

	adportable::fft::fourier_transform( spc, fft, true );

    auto src = spc.begin();
    
    for ( auto it = intens.begin(); it != intens.end(); ++it ) {
		*it = src->real();
        ++src;
    }

	return true;
}

bool
waveform::fft4g::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    if ( ms.isCentroid() || ms.size() < 32 )
        return false;

    int N = 32;
    while ( N < ms.size() )
		N *= 2;

    double sampInterval = ms.getMSProperty().samplingInfo().fSampInterval(); // seconds
    if ( adportable::compare<double>::approximatelyEqual( sampInterval, 0 ) )
        sampInterval = ( ms.getTime( ms.size() - 1 ) - ms.getTime( 0 ) ) / ms.size();

    const size_t cutoff = size_t( ( N * sampInterval ) * freq );

    std::vector< double > a( N );
    
    std::copy( ms.getIntensityArray(), ms.getIntensityArray() + ms.size(), a.begin() );
    std::fill( a.begin() + ms.size(), a.end(), ms.getIntensity( ms.size() - 1 ) );
    
    rdft( N, 1, a.data() );
    
    // appodization
    std::fill( a.begin() + cutoff, a.begin() + ( N - cutoff ), 0 );
    
    rdft( N, -1, a.data() );

    std::transform( a.begin(), a.begin() + ms.size(), a.begin(), [N] ( double a ) { return a * 2.0 / N; } );
    ms.setIntensityArray( a.data() );

    return true;
}

bool
waveform::fft4g::lowpass_filter( size_t size, double * data, double sampInterval, double freq )
{
    if ( size < 32 )
        return false;

    int N = 32;
    while ( N < size )
		N *= 2;

    const size_t cutoff = size_t( ( N * sampInterval ) * freq ) * 2.0;

    std::vector< double > a( N );
    
    std::copy( data, data + size, a.begin() );
    std::fill( a.begin() + size, a.end(), data[ size - 1 ] );
    
    rdft( N, 1, a.data() );
    
    // appodization
    std::fill( a.begin() + cutoff, a.begin() + ( N - cutoff ), 0 );
    
    rdft( N, -1, a.data() );

    auto src = a.begin();
    for ( double * it = data; it != data + size; ++it, ++src )
        *it = *src * 2.0 / N;

	return true;
}

//////////////////////
bool
waveform::fft4c::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    if ( ms.isCentroid() || ms.size() < 32 )
        return false;

    int N = 32;
    while ( N < ms.size() )
		N *= 2;

    double sampInterval = ms.getMSProperty().samplingInfo().fSampInterval(); // seconds
    if ( adportable::compare<double>::approximatelyEqual( sampInterval, 0 ) )
        sampInterval = ( ms.getTime( ms.size() - 1 ) - ms.getTime( 0 ) ) / ms.size();

    const size_t cutoff = size_t( ( N * sampInterval ) * freq );

	std::vector< std::complex<double> > a( N );
    
    std::copy( ms.getIntensityArray(), ms.getIntensityArray() + ms.size(), a.begin() );
    std::fill( a.begin() + ms.size(), a.end(), ms.getIntensity( ms.size() - 1 ) );
    
    cdft( N * 2, 1, reinterpret_cast<double *>( a.data() ) );

    // appodization
    std::fill( a.begin() + cutoff, a.begin() + ( N - cutoff ), 0 );
    
    cdft( N * 2, -1, reinterpret_cast<double *>( a.data() ) );

    for ( size_t i = 0; i < ms.size(); ++i )
        ms.setIntensity( i, a[ i ].real() * 2.0 / ( N * 2 ) );

    return true;
}

bool
waveform::fft4c::lowpass_filter( size_t size, double * data, double sampInterval, double freq )
{
    if ( size < 32 )
        return false;

    int N = 32;
    while ( N < size )
		N *= 2;

    const size_t cutoff = size_t( ( N * sampInterval ) * freq );
	std::vector< std::complex<double> > a( N );
    
    std::copy( data, data + size, a.begin() );
    std::fill( a.begin() + size, a.end(), data[ size - 1 ] );

    cdft( N * 2, 1, reinterpret_cast<double *>( a.data() ) );    
    
    // appodization
    std::fill( a.begin() + cutoff, a.begin() + ( N - cutoff ), 0 );

    cdft( N * 2, -1, reinterpret_cast<double *>( a.data() ) );        

    auto src = a.begin();
    for ( double * it = data; it != data + size; ++it, ++src )
        *it = src->real() * 2.0 / ( N * 2 );

	return true;
}
