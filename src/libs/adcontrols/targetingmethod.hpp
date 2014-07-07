// This is a -*- C++ -*- header.
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

#ifndef TARGETINGMETHOD_H
#define TARGETINGMETHOD_H

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include <string>
#include <vector>
#include <compiler/disable_dll_interface.h>

namespace boost { namespace serialization {  class access;  } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT TargetingMethod {
    public:
        enum idTarget { idTargetFormula, idTargetPeptide };

        TargetingMethod( idTarget id = idTargetFormula );
        TargetingMethod( const TargetingMethod& );
        TargetingMethod& operator = ( const TargetingMethod& rhs );

        void targetId( idTarget );
        idTarget targetId() const;

        std::vector< std::pair< bool, std::string > >& adducts( bool positive = true );
        const std::vector< std::pair< bool, std::string > >& adducts( bool positive = true ) const;

        //std::vector< std::pair< bool, std::string > >& lose();
        //const std::vector< std::pair< bool, std::string > >& lose() const;

		std::pair< unsigned int, unsigned int > chargeState() const;
		void chargeState( unsigned int, unsigned int );

		std::vector< std::pair< bool, std::string > >& formulae();
		const std::vector< std::pair< bool, std::string > >& formulae() const;
        
		std::vector< std::pair< bool, std::pair< std::string, std::string > > >& peptides();
		const std::vector< std::pair< bool, std::pair< std::string, std::string > > >& peptides() const;

        bool is_use_resolving_power() const;
        void is_use_resolving_power( bool );

        double resolving_power() const;
        void resolving_power( double );

        double peak_width() const;
        void peak_width( double );

        std::pair< bool, bool > isMassLimitsEnabled() const;
        void isLowMassLimitEnabled( bool );
        void isHighMassLimitEnabled( bool );
        
        double lowMassLimit() const;
        void lowMassLimit( double );

        double highMassLimit() const;
        void highMassLimit( double );

        double tolerance() const;
        void tolerance( double );

    private:
        idTarget idTarget_;
        bool is_use_resolving_power_;
        double resolving_power_;
        double peak_width_;
        uint32_t chargeStateMin_;
        uint32_t chargeStateMax_;
        bool isLowMassLimitEnabled_;
        bool isHighMassLimitEnabled_;
        double lowMassLimit_;
        double highMassLimit_;
        double tolerance_;

        std::vector< std::pair< bool, std::string > > pos_adducts_; // if start with '-' means lose instead of add
        std::vector< std::pair< bool, std::string > > neg_adducts_;

        std::vector< std::pair< bool, std::string > > formulae_;
        std::vector< std::pair< bool, std::pair< std::string, std::string > > > peptides_;
        
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version <= 1 ) {
                std::vector< std::pair< std::wstring, bool > > adducts;
                std::vector< std::pair< std::wstring, bool > > formulae;
                bool isPositive;

                ar & BOOST_SERIALIZATION_NVP( isPositive );
                ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power_ );
                ar & BOOST_SERIALIZATION_NVP( resolving_power_ );
                ar & BOOST_SERIALIZATION_NVP( peak_width_ );
                ar & BOOST_SERIALIZATION_NVP( chargeStateMin_ );
                ar & BOOST_SERIALIZATION_NVP( chargeStateMax_ );
                ar & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ );
                ar & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ );
                ar & BOOST_SERIALIZATION_NVP( lowMassLimit_ );
                ar & BOOST_SERIALIZATION_NVP( highMassLimit_ );
                ar & BOOST_SERIALIZATION_NVP( tolerance_ );
                ar & BOOST_SERIALIZATION_NVP( formulae );
                ar & BOOST_SERIALIZATION_NVP( adducts ); // pos
                ar & BOOST_SERIALIZATION_NVP( adducts ); // neg
            } else {
                ar & BOOST_SERIALIZATION_NVP( idTarget_ )
                    & BOOST_SERIALIZATION_NVP( is_use_resolving_power_ )
                    & BOOST_SERIALIZATION_NVP( resolving_power_ )
                    & BOOST_SERIALIZATION_NVP( peak_width_ )
                    & BOOST_SERIALIZATION_NVP( chargeStateMin_ )
                    & BOOST_SERIALIZATION_NVP( chargeStateMax_ )
                    & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ )
                    & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ )
                    & BOOST_SERIALIZATION_NVP( lowMassLimit_ )
                    & BOOST_SERIALIZATION_NVP( highMassLimit_ )
                    & BOOST_SERIALIZATION_NVP( tolerance_ )
                    & BOOST_SERIALIZATION_NVP( pos_adducts_ )
                    & BOOST_SERIALIZATION_NVP( neg_adducts_ )
                    & BOOST_SERIALIZATION_NVP( formulae_ )
                    & BOOST_SERIALIZATION_NVP( peptides_ )
                    ;
            }
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::TargetingMethod, 3 )

#endif // TARGETINGMETHOD_H
