/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC
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
#include <boost/numeric/ublas/fwd.hpp>
#include <cstdint>

namespace boost { namespace serialization { class access; } }
namespace adcontrols { class idAudit; }

namespace adcontrols {

    class MappedSpectrum;
    
    /** \class MappedSpectra
     * 
     * \brief Imaging MALDI TOF data class
     *
     * This class holds accumulated MALDI TOF image spectral data.
     * Spectral data are stored into <em>N by M</em> matrix.  Each spectrum
     * is consisted from simple array of time-of-flight, count pair.
     */

    class ADCONTROLSSHARED_EXPORT MappedSpectra {
    public:
        ~MappedSpectra( void );
        MappedSpectra( void );
        MappedSpectra( size_t i, size_t j );
        MappedSpectra( const MappedSpectra & );
        MappedSpectra & operator = ( const MappedSpectra & rhs );
        const adcontrols::idAudit& ident() const;

        size_t size1() const;  // horizontal pixel count
        size_t size2() const;  // vertical pixel count

        MappedSpectrum& operator ()( size_t i, size_t j );
        const MappedSpectrum& operator ()( size_t i, size_t j ) const;

        MappedSpectra& operator += ( const boost::numeric::ublas::matrix< uint16_t >& frame );
            
    private:
        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
}

