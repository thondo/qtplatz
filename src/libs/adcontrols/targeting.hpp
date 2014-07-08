/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef TARGETING_HPP
#define TARGETING_HPP

#include "adcontrols_global.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <tuple>

#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class TargetingMethod;
    class MassSpectrum;

    class ADCONTROLSSHARED_EXPORT Targeting {
    public:
        struct Candidate;

        Targeting();
        Targeting( const Targeting& );
        Targeting( const TargetingMethod& );

        bool operator ()( const MassSpectrum& );

        const Candidate& results() const;
        const Candidate& combination() const;

        struct Candidate {
            uint32_t idx; // peak index on mass-spectrum 
            uint32_t fcn; // protocol (aka segment) id
            uint32_t charge;
            double mass_error;
            std::string formula; // this is the exact formula matched with the peak (contains adducts)
            Candidate();
            Candidate( const Candidate& );
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( idx )
                    & BOOST_SERIALIZATION_NVP( fcn )
                    & BOOST_SERIALIZATION_NVP( charge )
                    & BOOST_SERIALIZATION_NVP( mass_error )
                    & BOOST_SERIALIZATION_NVP( formula )
                    ;
            }
        };

    private:
        std::vector< Candidate > candidates_;
        typedef std::pair< double, std::string > adduct_type;
        typedef std::tuple< double, std::string, uint32_t > charge_adduct_type;
        std::vector< std::pair< std::string, double > > active_formula_; // M
        std::vector< adduct_type > pos_adducts_;
        std::vector< adduct_type > neg_adducts_;
        std::vector< charge_adduct_type > poslist_;
        std::vector< charge_adduct_type > neglist_;

        void setup( const TargetingMethod& );
        static void setup_adducts( const TargetingMethod&, bool, std::vector< adduct_type >& );
        static void make_combination( uint32_t charge, std::vector< adduct_type >&, std::vector< charge_adduct_type >& );

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( candidates_ )
                ;
        }
    };

}

#endif // TARGETING_HPP
