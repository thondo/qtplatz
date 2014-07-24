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
#include "msfinder.hpp"
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

		std::pair< unsigned int, unsigned int > chargeState() const;
		void chargeState( unsigned int, unsigned int );
        typedef std::pair< std::string, std::pair< bool, std::wstring > > formula_type;
        typedef std::pair< bool, std::pair< std::string, std::string > > peptide_type;

		std::vector< formula_type >& formulae();
		const std::vector< formula_type >& formulae() const;
        
		std::vector< peptide_type  >& peptides();
		const std::vector< peptide_type >& peptides() const;

        idToleranceMethod toleranceMethod() const;
        void setToleranceMethod( idToleranceMethod );

        double tolerance( idToleranceMethod ) const;
        void setTolerance( idToleranceMethod, double );

        std::pair< bool, bool > isMassLimitsEnabled() const;
        void isLowMassLimitEnabled( bool );
        void isHighMassLimitEnabled( bool );
        
        double lowMassLimit() const;
        void lowMassLimit( double );

        double highMassLimit() const;
        void highMassLimit( double );

        //double tolerance() const;
        //void tolerance( double );

        struct ADCONTROLSSHARED_EXPORT formula_data {
            TargetingMethod::formula_type f_;
            formula_data( bool enable, const std::string& formula, const std::wstring& desc ) {
                f_ = std::make_pair( formula, std::make_pair( enable, desc ) );
            }
            operator const TargetingMethod::formula_type& () { return f_; }
            static bool enable( const TargetingMethod::formula_type& f ) { return f.second.first; }
            static const std::string& formula( const TargetingMethod::formula_type& f ) { return f.first; }
            static const std::wstring& description( const TargetingMethod::formula_type& f ) { return f.second.second; }
        };

    private:
        idTarget idTarget_;
        idToleranceMethod toleranceMethod_;
        double tolerancePpm_;
        double toleranceDaltons_;
        uint32_t chargeStateMin_;
        uint32_t chargeStateMax_;
        bool isLowMassLimitEnabled_;
        bool isHighMassLimitEnabled_;
        double lowMassLimit_;
        double highMassLimit_;
        double tolerance_;

        std::vector< std::pair< bool, std::string > > pos_adducts_; // if start with '-' means lose instead of add
        std::vector< std::pair< bool, std::string > > neg_adducts_;

        std::vector< formula_type > formulae_;
        std::vector< peptide_type > peptides_;
        
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            bool is_use_resolving_power; // workaround

            if ( version <= 1 ) {
                std::vector< std::pair< std::wstring, bool > > adducts;
                std::vector< std::pair< std::wstring, bool > > formulae;
                bool isPositive;

                ar & BOOST_SERIALIZATION_NVP( isPositive );
                ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power );
                ar & BOOST_SERIALIZATION_NVP( tolerancePpm_ );
                ar & BOOST_SERIALIZATION_NVP( toleranceDaltons_ );
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
                    ;
                if ( version < 4 )
                    ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power );
                else if ( version >= 4 )
                    ar & BOOST_SERIALIZATION_NVP( toleranceMethod_ );
                ar & BOOST_SERIALIZATION_NVP( tolerancePpm_ )
                    & BOOST_SERIALIZATION_NVP( toleranceDaltons_ )
                    & BOOST_SERIALIZATION_NVP( chargeStateMin_ )
                    & BOOST_SERIALIZATION_NVP( chargeStateMax_ )
                    & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ )
                    & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ )
                    & BOOST_SERIALIZATION_NVP( lowMassLimit_ )
                    & BOOST_SERIALIZATION_NVP( highMassLimit_ )
                    & BOOST_SERIALIZATION_NVP( tolerance_ )
                    & BOOST_SERIALIZATION_NVP( pos_adducts_ )
                    & BOOST_SERIALIZATION_NVP( neg_adducts_ )
                    ;
                if ( version < 3 ) {
                    std::vector< std::pair< bool, std::string > > formulae;
                    std::vector< std::pair< bool, std::pair< std::string, std::string > > > peptides;
                    ar & BOOST_SERIALIZATION_NVP( formulae )
                        & BOOST_SERIALIZATION_NVP( peptides )
                        ;
                } else {
                    ar & BOOST_SERIALIZATION_NVP( formulae_ )
                        & BOOST_SERIALIZATION_NVP( peptides_ )
                        ;
                }
            }
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::TargetingMethod, 3 )

#endif // TARGETINGMETHOD_H
