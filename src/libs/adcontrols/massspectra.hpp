// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include "massspectrum.hpp"
#include <string>
#include <memory>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/serialization_shared_ptr.hpp>

#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class MassSpectrum;
    class Chromatogram;

    class ADCONTROLSSHARED_EXPORT MassSpectra {
    public:
        ~MassSpectra();
        MassSpectra();
        MassSpectra( const MassSpectra& );

        typedef std::shared_ptr< MassSpectrum > value_type;
        typedef std::vector< value_type >::iterator iterator;
        typedef std::vector< value_type >::const_iterator const_iterator;

        MassSpectra& operator << ( value_type ); // shared object
        MassSpectra& operator << ( const MassSpectrum& ); // create new copy

        size_t size() const;
        MassSpectrum& operator [] ( size_t fcn );
        const MassSpectrum& operator [] ( size_t fcn ) const;
        
        inline iterator begin() { return vec_.begin(); }
        inline iterator end() { return vec_.end(); }
        inline const_iterator begin() const { return vec_.begin(); }
        inline const_iterator end() const { return vec_.end(); }

        void setChromatogram( const Chromatogram& );
        const std::vector< double >& x() const;

        void lower_mass( double );
        void upper_mass( double );
        void z_max( double );

        double lower_mass() const;
        double upper_mass() const;
        double x_left() const;
        double x_right() const;
        double z_max() const;

		static const wchar_t * dataClass() { return L"adcontrols::MassSpectra"; }
        static bool archive( std::ostream&, const MassSpectra& );
        static bool restore( std::istream&, MassSpectra& );
    private:
        std::vector< value_type > vec_;
        std::vector< double > x_;
        double lower_mass_;
        double upper_mass_;
        double z_max_;

	    friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int) {
            ar & lower_mass_ & upper_mass_ & z_max_ & x_ & vec_;
        }
    };    
    typedef std::shared_ptr<MassSpectra> MassSpectraPtr;
}


