/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "targeting.hpp"
#include "targetingmethod.hpp"
#include "chemicalformula.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <adportable/combination.hpp>
#include <adportable/for_each_combination.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <set>
#include <map>

namespace adcontrols {

    typedef std::map< std::string, uint32_t > adduct_complex_t;

    struct make_formula {
        template< char sign >
        bool to_formula( int charge
                         , const adduct_complex_t& complex
                         , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts
                         , std::ostream& o ) {

            bool once_flag(true);

            for ( const auto& a: complex ) {
                assert( a.second );
                auto it = adducts.find( a.first );
                if ( it != adducts.end() ) {
                    if ( it->second.second == sign ) {
                        if ( once_flag ) {
                            o << sign;
                            if ( charge )
                                o << "[";
                            once_flag = false;
                        }
                        size_t natoms = adcontrols::ChemicalFormula::number_of_atoms( it->second.first );
                        if ( a.second > 1 && natoms > 1 )
                            o << "(";
                        o << it->second.first;
                        if ( a.second > 1 && natoms > 1 )
                            o << ")";
                        if ( a.second > 1 )
                            o << a.second;
                    }
                }
            }
            if ( charge && !once_flag ) {
                o << "]";
                if ( std::abs( charge ) > 1 )
                    o << std::abs( charge );
                o << (charge > 0 ? '+' : '-');
            }
            return !once_flag;
        }

        std::string
        operator()( int charge
                    , const adduct_complex_t& complex
                    , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts ) {
            std::ostringstream o;
            to_formula<'-'>( 0, complex, adducts, o );
            if ( ! to_formula<'+'>( charge, complex, adducts, o ) ) {
                std::ostringstream n;
                to_formula<'-'>( charge, complex, adducts, n );
                return n.str();
            }
            return o.str();
        }

    };

    struct make_combination {

        typedef std::map< std::string, uint32_t > adduct_complex_t;
        std::vector< adduct_complex_t > complex_;

        static void print( int charge
                           , const adduct_complex_t& complex
                           , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts
                           , std::ostream& o ) {
            o << make_formula()( charge, complex, adducts );
        }

        std::vector< std::string >
        operator()( uint32_t charge
                         , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts ) {

            const size_t r = charge;
            const size_t n = adducts.size();

            std::vector< std::string > v_keys( n );
            std::transform( adducts.begin(), adducts.end(), v_keys.begin(), [](const auto& a){ return a.first; });
            std::vector< std::vector< std::string >::const_iterator > v_iter( r, v_keys.begin() );
            do {
                std::ostringstream o;

                adduct_complex_t an_adduct;
                std::for_each( v_iter.begin(), v_iter.end(), [&]( const auto& it ){ an_adduct[ *it ]++; });
                // print( charge, an_adduct, adducts, o );
                // ADDEBUG() << "\t" << o.str();
                complex_.emplace_back( std::move( an_adduct ) );

            } while ( boost::next_mapping( v_iter.begin(), v_iter.end(), v_keys.begin(), v_keys.end() ) );

            std::vector< std::string > results;
            for ( const auto& complex: complex_ ) {
                results.emplace_back( make_formula()( charge, complex, adducts ) );
            }
            return results;
        }

    };
}

using namespace adcontrols;

Targeting::Targeting() : method_( std::make_shared< TargetingMethod >() )
{
}

Targeting::Targeting( const Targeting& t ) : method_( t.method_ )
                                           , candidates_( t.candidates_ )
                                           , active_formula_( t.active_formula_ )
                                           , pos_adducts_( t.pos_adducts_ )
                                           , neg_adducts_( t.neg_adducts_ )
{
}

Targeting::Targeting( const TargetingMethod& m ) : method_( std::make_shared<TargetingMethod>( m ) )
{
    setup( m );
}

Targeting::Candidate::Candidate() : idx(0)
                                  , fcn(0)
                                  , charge(1)
                                  , mass_error(0)
{
}

Targeting::Candidate::Candidate( const Candidate& t ) : idx( t.idx )
                                                      , fcn( t.fcn )
                                                      , charge( t.charge )
                                                      , mass_error( t.mass_error )
                                                      , formula( t.formula )
{
}


Targeting::Candidate::Candidate( uint32_t _idx, uint32_t _fcn, uint32_t _charge, double _error, const std::string& _formula )
    : idx( _idx )
    , fcn( _fcn )
    , charge( _charge )
    , mass_error( _error )
    , formula( _formula )
{
}

bool
Targeting::find_candidate( const MassSpectrum& ms, int fcn, bool polarity_positive, const std::vector< charge_adduct_type >& list )
{
    if ( ms.size() == 0 )
        return false;

    adcontrols::MSFinder finder( method_->tolerance( method_->toleranceMethod() ), method_->findAlgorithm(), method_->toleranceMethod() );

    for ( auto& formula : active_formula_ ) {
        double target_mass = formula.second; // search 'M'

        size_t pos = finder( ms, target_mass );
        if ( pos != MassSpectrum::npos ) {
            double error = ms.getMass( pos ) - target_mass;
            candidates_.push_back( Candidate( uint32_t( pos /*idx*/ ), fcn, 1/*charge*/, error, formula.first /*formula*/ ) );
        }
    }

    (void)polarity_positive; // this will be necessary for account an electron mass, todo
    for ( auto& adduct : list ) {
        for ( auto& formula : active_formula_ ) {
            double target_mass = ( formula.second /* M */ + std::get<0>( adduct /*mass*/ ) ) / std::get<2>( adduct /*charge*/ );

            size_t pos = finder( ms, target_mass );
            if ( pos != MassSpectrum::npos ) {
                double error = ms.getMass( pos ) - target_mass;
                candidates_.push_back( Candidate( uint32_t( pos ), fcn, std::get<2>( adduct ), error, formula.first + "+" + std::get<1>( adduct ) ) );
            }
        }
    }
    return true;
}

bool
Targeting::operator()( const MassSpectrum& ms )
{
    candidates_.clear();

    if ( !ms.isCentroid() )
        return false;

    bool polarity_positive = (ms.polarity() == PolarityPositive) || (ms.polarity() == PolarityIndeterminate);

    segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    int fcn = 0;
    for ( auto& fms : segs ) {
        if ( polarity_positive ) {
            find_candidate( fms, fcn++, true, poslist_ );
        }
        else {
            find_candidate( fms, fcn++, false, neglist_ );
        }
    }
    return true;
}

bool
Targeting::force_find( const MassSpectrum& ms, const std::string& formula, int32_t fcn )
{
    candidates_.clear();

    if ( !ms.isCentroid() )
        return false;
    if ( ms.size() == 0 )
        return false;

    double mass = ChemicalFormula().getMonoIsotopicMass( formula );

    int proto( 0 );
    for ( auto& fms: segment_wrapper< const adcontrols::MassSpectrum >( ms ) ) {
        if ( proto == fcn ) {
            double width = method_->tolerance( idToleranceDaltons );
            do {
                adcontrols::MSFinder finder( width, idFindClosest );
                size_t pos = finder( fms, mass );
                if ( pos != MassSpectrum::npos ) {
                    candidates_.emplace_back( pos, proto, /*charge*/ 1, fms.getMass( pos ) - mass, formula );
                    return true;
                }
                width += width;
            } while ( width < 1.0 );
        }
        ++proto;
    }
    return false;
}

void
Targeting::setup( const TargetingMethod& m )
{
    ChemicalFormula formula_parser;

    active_formula_.clear();

    std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t > adducts_global;

    bool positive = true;

    auto charge_range = m.chargeState();

    for ( auto& a: m.adducts( positive ) ) {
        if ( a.first ) {
            for ( const auto& adduct: ChemicalFormula::split( a.second ) ) {
                auto sign = (adduct.second == '-' ? "-" : "+");
                adducts_global[ sign + ChemicalFormula::standardFormula( adduct.first, true ) ] = { adduct.first, sign[0] };
            }
        }
    }

    for ( auto& x : m.molecules().data() ) {
        if ( x.enable() ) {

            std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t > adducts_local( adducts_global );

            if ( !std::string( x.adducts() ).empty() ) {
                for ( const auto& a: ChemicalFormula::split( x.adducts() ) ) {
                    auto sign = (a.second == '-' ? "-" : "+");
                    auto pair = ChemicalFormula::neutralize( a.first ); // neutral formula, charge
                    adducts_local[ sign + ChemicalFormula::standardFormula( a.first, true ) ] = { pair.first, sign[0] };
                }
                auto formula = std::string( x.formula() ) + x.adducts();
            }

            for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
                if ( charge == 1 ) {
                    for ( const auto& a: adducts_local ) {
                        std::ostringstream t;
                        t << x.formula() << a.second.second << "[" << a.second.first << "]+";  // "+|-" + ['adduct']+
                        active_formula_.emplace_back( t.str(), formula_parser.getMonoIsotopicMass( ChemicalFormula::split( t.str() ) ) );
                    }
                } else if ( charge >= 2 ) {
                    for ( const auto& a: make_combination()( charge, adducts_local ) ) {
                        std::ostringstream t;
                        t << x.formula() << a;
                        active_formula_.emplace_back( t.str(), formula_parser.getMonoIsotopicMass( ChemicalFormula::split( t.str() ) ) );
                    }
                }
            }
#ifndef NDEBUG
            for ( const auto& a: active_formula_ )
                ADDEBUG() << a << ", neutral: " << ChemicalFormula::neutralize( a.first );
#endif
        }
    }

}

void
Targeting::setup_adducts( const TargetingMethod& m, bool positive, std::vector< adduct_type >& adducts )
{
    ChemicalFormula formula_parser;

    for ( auto& a: m.adducts( positive ) ) {

        if ( a.first ) { // if (enable)
            std::string addformula;
            std::string loseformula;
            std::pair< double, double > addlose( 0, 0 );
            if ( !addformula.empty() ) {
                addlose.first = formula_parser.getMonoIsotopicMass( addformula );
            }
            if ( !loseformula.empty() ) {
                addlose.second = -formula_parser.getMonoIsotopicMass( loseformula );
            }
            adducts.push_back( std::make_pair( addlose.first + addlose.second, a.second ) );
        }
    }
}



// void
// Targeting::make_combination( uint32_t charge
//                              , const std::vector< adduct_type >& adducts
//                              , std::vector< charge_adduct_type >& list )
// {
//     if ( adducts.empty() || charge == 0 )
//         return;

//     typedef size_t formula_number;
//     typedef size_t formula_count;
//     std::set< std::map< formula_number, formula_count > > combination_with_repetition;
//     {
//         // adportable::scoped_debug<> scope( __FILE__, __LINE__ ); scope << "making combination(2):";
//         std::vector< uint32_t > selector( adducts.size() * charge );
//         auto it = selector.begin();
//         for( uint32_t n = 0; n < adducts.size(); ++n ) {
//             std::fill( it, it + charge, n );
//             std::advance( it, charge );
//         }
//         do {
//             std::map< formula_number, formula_count > formulae;

//             for ( auto it = selector.begin(); it != selector.begin() + charge; ++it )
//                 formulae[ *it ]++;

//             combination_with_repetition.insert( formulae );

//         } while ( boost::next_combination( selector.begin(), selector.begin() + charge, selector.end() ) );
//     }

//     // translate index combination into a vector of atoms|formula with exact-mass

//     for ( auto& fmap : combination_with_repetition ) {
//         std::ostringstream formula;
//         double mass = 0;
//         std::for_each( fmap.begin(), fmap.end(), [&] ( const std::pair< size_t, size_t >& a ){
//             formula << a.second << "(" << adducts[ a.first ].second << ")";
//             mass += adducts[ a.first ].first * a.second;
//         } );
//         list.push_back( std::make_tuple( mass, formula.str(), charge ) );
//         // ADDEBUG() << "charge: " << charge << "\tmass=" << mass << "\t" << formula.str();
//     }
// }

bool
Targeting::archive( std::ostream& os, const Targeting& v )
{
    portable_binary_oarchive ar( os );
    ar << v;
    return true;
}

bool
Targeting::restore( std::istream& is, Targeting& v )
{
    portable_binary_iarchive ar( is );
    ar >> v;
    return true;
}
