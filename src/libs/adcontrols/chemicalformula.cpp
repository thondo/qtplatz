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

#include <compiler/diagnostic_push.h>
#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>
#include "chemicalformula.hpp"
#include "tableofelement.hpp"
#include "ctable.hpp"
#include "element.hpp"
#include <adportable/utf.hpp>
#include <adportable/formula_parser.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/noncopyable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/tokenizer.hpp>
#include <compiler/diagnostic_pop.h>

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>

#if defined _MSC_VER
# pragma warning( disable: 4503)
#endif
using namespace adcontrols;

namespace adcontrols {

    namespace internal {

        // for chemical formula formatter
        static const char * braces [] = { "(", ")" };
        using adportable::chem::atom_type;
        
        typedef std::vector< std::pair< atom_type, size_t > > format_type;
        struct formulaFormat {
            static void formula_add( format_type& m, const std::pair<const atom_type, std::size_t>& p ) {
                m.push_back( p );
            }
            static void formula_join( format_type& m, format_type& a ) {
                m.push_back( std::make_pair( atom_type( 0, braces[0] ), 0 ) );
                for ( auto t: a )
                    m.push_back( t );
            }
            static void formula_repeat( format_type& m, std::size_t n ) {
                m.push_back( std::make_pair( atom_type( 0, braces[1] ), n ) );
            }
        };

        class ChemicalFormulaImpl {
        public:
            ChemicalFormulaImpl();

            template< typename char_type > static double getMonoIsotopicMass( const std::basic_string< char_type >& formula ) {
                adportable::chem::comp_type comp;
                if ( parse( formula, comp ) ) {
                    adcontrols::TableOfElement *toe = adcontrols::TableOfElement::instance();
                    double mass =
                        std::accumulate( comp.begin(), comp.end(), 0.0, [=]( double m, const adportable::chem::comp_type::value_type& pair ){
                                if ( mol::element e = toe->findElement( pair.first.second ) )
                                    return m + mol::element::monoIsotopicMass( e, pair.first.first /* isotope */ ) * pair.second;
                                return m;
                            });
                    return mass;
                }
                return 0;
            }

            template< typename char_type > static double getChemicalMass( const std::basic_string< char_type >& formula ) {
                adportable::chem::comp_type comp;
                if ( parse( formula, comp ) ) {
                    adcontrols::TableOfElement *toe = adcontrols::TableOfElement::instance();
                    double mass =
                        std::accumulate( comp.begin(), comp.end(), 0.0, [=]( double m, const adportable::chem::comp_type::value_type& pair ){
                                if ( mol::element e = toe->findElement( pair.first.second ) )
                                    return m + mol::element::chemicalMass( e ) * pair.second;
                                return m;
                            });
                    return mass;
                }
                return 0;
            }

			template< typename char_type > static bool parse( const std::basic_string< char_type >& formula, adportable::chem::comp_type& comp ) {
                using adportable::chem::formulaComposition;
                using adportable::chem::comp_type;
                
                typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
                adportable::chem::chemical_formula_parser< iterator_type, formulaComposition, comp_type > cf;
                iterator_type it = formula.begin();
                iterator_type end = formula.end();

                return boost::spirit::qi::parse( it, end, cf, comp ) && it == end;
            }

			template< typename char_type > static bool format( const std::basic_string< char_type >& formula, format_type& fmt ) {
                using adportable::chem::comp_type;
                typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
                adportable::chem::chemical_formula_parser< iterator_type, formulaFormat, format_type > cf;
                iterator_type it = formula.begin();
                iterator_type end = formula.end();

                return boost::spirit::qi::parse( it, end, cf, fmt ) && it == end;
            }
            
            template< typename char_type > static std::basic_string<char_type> standardFormula( const std::basic_string<char_type>& formula ) {

				adportable::chem::comp_type map;
				using adportable::chem::atom_type;

                if ( parse( formula, map ) ) {
					std::vector< std::pair< atom_type, size_t > > orderd;
                    for ( auto atom: map )
						orderd.push_back( std::make_pair( atom.first, atom.second ) );

                    // reorder elements in alphabetical order
					std::sort( orderd.begin(), orderd.end()
                               , []( const std::pair< atom_type, size_t >& lhs, const std::pair< atom_type, size_t >& rhs ){
                                   return std::strcmp( lhs.first.second, rhs.first.second ) < 0 || lhs.first.first < rhs.first.first;
                               } );

                    std::basic_ostringstream<char_type> o;

                    for ( auto& p: orderd ) {
						std::basic_string<char_type> atom( p.first.second, p.first.second + std::strlen( p.first.second ) );
                        if ( p.first.first == 0 ) {
                            o << atom;
                            if ( p.second > 1 )  // omit '1' such as CH4, not C1H4
                                o << p.second;
                        } else {
                            o << atom;
                            if ( p.second > 1 ) 
                                o << p.second;
                            o << ' ';
                        }
                    }
                    return o.str();
                }
                return std::basic_string<char_type>();
            }
        };

        template<typename char_t> struct delimitors { const char_t * operator()() const; };
        template<> struct delimitors < wchar_t > { const wchar_t * operator()() const { return L"+-"; } };
        template<> struct delimitors < char > { const char * operator()() const { return "+-"; } };

        template< typename char_type > struct splitter {

            typename std::basic_string<char_type>::size_type operator()( /*typename*/ const std::basic_string<char_type>& formula
                                                                         , typename std::basic_string<char_type>::size_type pos = 0 ) {
                return formula.find_first_of( delimitors<char_type>()(), pos );
            }
            //<-------------------------------------------
            static void split( std::basic_string<char_type>& M
                               , std::pair< std::basic_string< char_type >, std::basic_string< char_type > >& adducts
                               , const std::basic_string< char_type >& formula ) {
                
                splitter< char_type > splitter;
                typename std::basic_string<char_type>::size_type pos;

                if ( (pos = splitter( formula )) != std::basic_string<char_type>::npos ) {
                    M = formula.substr( 0, pos );

                    while ( pos != std::basic_string<char_type>::npos ) {
                        typename std::basic_string< char_type >::size_type next = splitter( formula, pos + 1 );
                        if ( formula.at( pos ) == char_type( '+' ) )
                            adducts.first += formula.substr( pos + 1, next - pos - 1 ); // skip (+) sign
                        else if ( formula.at( pos ) == char_type( '-' ) )
                            adducts.second += formula.substr( pos + 1, next - pos - 1 ); // skip (-) sign
                        else
                            throw std::logic_error( "bug" );
                        pos = next;
                    }
                } else {
                    M = formula; // no adduct/lose specified
                }
            }
        };


        template<typename char_type> std::basic_string<char_type> make_adduct_string( const std::pair < std::basic_string<char_type>
                                                                                      , std::basic_string<char_type> >& adduct
                                                                                      , bool leading_plus ) {
            std::basic_string<char_type> result;
            if ( !adduct.first.empty() ) {
                if ( leading_plus )
                    result += char_type( '+' );
                std::for_each( adduct.first.begin(), adduct.first.end(), [&] ( const char_type& c ){ if ( c != char_type( '+' ) ) result += c; } );
            }
            if ( !adduct.second.empty() ) {
                result += char_type( '-' );
                std::for_each( adduct.second.begin(), adduct.second.end(), [&] ( const char_type& c ){ if ( c != char_type( '-' ) ) result += c; } );
            }
            return result;
        }

        template<typename char_type> std::basic_string<char_type> formatFormulae( const std::basic_string<char_type>& formula
                                                                                  , const char_type * dropped_delims
                                                                                  , const char_type * kept_delims
                                                                                  , bool richText ) {
            typedef boost::tokenizer< boost::char_separator< char_type >
                                      , std::basic_string< char_type >::const_iterator
                                      , std::basic_string< char_type > > tokenizer_t;
    
            boost::char_separator< char_type > separator( dropped_delims, kept_delims, boost::drop_empty_tokens );
            tokenizer_t tokens( formula, separator );
            std::basic_string< char_type > kept( kept_delims );

            std::basic_ostringstream<char_type> o;

            for( auto it = tokens.begin(); it != tokens.end(); ++it ) {
                if ( it->length() == 1 && kept.find( (*it) ) != std::basic_string< char_type >::npos )
                    o << *it;
                else
                    o << adcontrols::ChemicalFormula::formatFormula( *it, richText );
            }
            return o.str();
            
        }
    }

}

ChemicalFormula::~ChemicalFormula(void)
{
}

ChemicalFormula::ChemicalFormula() 
{
}

double
ChemicalFormula::getElectronMass() const
{
    return 5.4857990943e-4;
}

double
ChemicalFormula::getMonoIsotopicMass( const std::wstring& formula ) const
{
    return internal::ChemicalFormulaImpl::getMonoIsotopicMass( formula );
}

double
ChemicalFormula::getMonoIsotopicMass( const std::string& formula ) const
{
    return internal::ChemicalFormulaImpl::getMonoIsotopicMass( formula );
}

double
ChemicalFormula::getMonoIsotopicMass( const std::wstring& formula, const std::pair< std::wstring, std::wstring >& adducts ) const
{
    double mass = internal::ChemicalFormulaImpl::getMonoIsotopicMass( formula );
    if ( !adducts.first.empty() )
        mass += internal::ChemicalFormulaImpl::getMonoIsotopicMass( adducts.first );
    if ( !adducts.second.empty() )
        mass -= internal::ChemicalFormulaImpl::getMonoIsotopicMass( adducts.second );
    return mass;
}

double
ChemicalFormula::getMonoIsotopicMass( const std::string& formula, const std::pair< std::string, std::string >& adducts ) const
{
    double mass = internal::ChemicalFormulaImpl::getMonoIsotopicMass( formula );
    if ( !adducts.first.empty() )
        mass += internal::ChemicalFormulaImpl::getMonoIsotopicMass( adducts.first );
    if ( !adducts.second.empty() )
        mass -= internal::ChemicalFormulaImpl::getMonoIsotopicMass( adducts.second );
    return mass;
}

double
ChemicalFormula::getChemicalMass( const std::wstring& formula ) const
{
    return internal::ChemicalFormulaImpl::getChemicalMass( formula );
}

std::wstring
ChemicalFormula::standardFormula( const std::wstring& formula )
{
    return internal::ChemicalFormulaImpl::standardFormula( formula );
}

std::string
ChemicalFormula::standardFormula( const std::string& formula )
{
    return internal::ChemicalFormulaImpl::standardFormula( formula );
}

std::string
ChemicalFormula::formatFormula( const std::string& formula, bool richText )
{
    std::wstring wformula = formatFormula( adportable::utf::to_wstring( formula ), richText );
    return adportable::utf::to_utf8( wformula );
}

std::wstring
ChemicalFormula::formatFormula( const std::wstring& formula, bool richText )
{
    using adcontrols::internal::ChemicalFormulaImpl;
    internal::format_type fmt;

    if ( ChemicalFormulaImpl::format( formula, fmt ) ) {

        std::wostringstream o;
        if ( richText ) {
            for ( auto e: fmt ) {
                if ( std::strcmp( e.first.second, "(" ) == 0 ) {
                    o << L"(";
                } else if ( std::strcmp( e.first.second, ")" ) == 0 ) {
                    o << boost::wformat( L")<sub>%1%</sub>" ) % e.second;
                } else {
                    if ( e.first.first )
                        o << boost::wformat( L"<sup>%1%</sup>" ) % e.first.first;
                    o << adportable::utf::to_wstring( e.first.second ); // element name
                    if ( e.second > 1 )
                        o << boost::wformat( L"<sub>%1%</sub>" ) % e.second;
                }
            }
        } else {
            bool sol = true;
            for ( auto e: fmt ) {
                if ( std::strcmp( e.first.second, "(" ) == 0 ) {
                    o << L"(";
                    sol = true;
                } else if ( std::strcmp( e.first.second, ")" ) == 0 ) {
                    o << e.second;
                } else {
                    if ( e.first.first ) {
                        if ( !sol )
                            o << L" " << e.first.first;
                        else
                            o << e.first.first;
                    }
                    o << adportable::utf::to_wstring( e.first.second ); // element name
                    if ( e.second > 1 )
                        o << e.second;
                    sol = false;
                }
            }
        }
        return o.str();
    }
    return L"";    
}


namespace adcontrols {

	//struct is_connect {
	size_t bond_connect( const CTable::Bond& bond, size_t num ) {
		return ( size_t( bond.first_atom_number ) == num ) ? bond.second_atom_number
			: ( size_t( bond.second_atom_number ) == num ) ? bond.first_atom_number : 0;
	}
}

bool
ChemicalFormula::getComposition( std::vector< mol::element >& el, const std::string& formula )
{
    adportable::chem::comp_type comp;

	if ( internal::ChemicalFormulaImpl::parse( formula, comp ) ) {
        for ( auto& c: comp ) {
            // ignore isotope
            auto it = std::find_if( el.begin(), el.end(), [=]( const mol::element& e ){ return c.first.second == e.symbol(); }); 
            if ( it != el.end() ) {
                it->count( it->count() + c.second );
            } else {
                if ( mol::element e = TableOfElement::instance()->findElement( c.first.second ) ) {
                    e.count( c.second );
                    el.push_back( e );
                }
            }
        }
	}
    return !el.empty();
}

bool
ChemicalFormula::split( const std::string& formula, std::vector< std::string >& results, const char * dropped_delims, const char * kept_delims )
{
    typedef char char_type;

    typedef boost::tokenizer< boost::char_separator< char_type >
                              , std::basic_string< char_type >::const_iterator
                              , std::basic_string< char_type > > tokenizer_t;
    
    boost::char_separator< char > separator( dropped_delims, kept_delims, boost::keep_empty_tokens );

    tokenizer_t tokens( formula, separator );
    std::for_each( tokens.begin(), tokens.end(), [&] ( const tokenizer_t::value_type& it ){
        if ( !it.empty() )
            results.push_back( it );
    } );
    return !results.empty();
}

std::string
ChemicalFormula::splitFormula( std::pair< std::string, std::string >& adducts, const std::string& formula, bool bStandardFormat )
{
    std::string M;
    internal::splitter<char>::split( M, adducts, formula );
    if ( bStandardFormat ) {
        M = standardFormula( M );
        if ( !adducts.first.empty() )
            adducts.first = standardFormula( adducts.first );
        if ( !adducts.second.empty() )
            adducts.second = standardFormula( adducts.second );
    }
    return M;
}

std::wstring
ChemicalFormula::splitFormula( std::pair< std::wstring, std::wstring >& adducts, const std::wstring& formula, bool bStandardFormat )
{
    std::wstring M;
    internal::splitter<wchar_t>::split( M, adducts, formula );
    if ( bStandardFormat ) {
        M = standardFormula( M );
        if ( !adducts.first.empty() )
            adducts.first = standardFormula( adducts.first );
        if ( !adducts.second.empty() )
            adducts.second = standardFormula( adducts.second );
    }
    return M;
}

std::string
ChemicalFormula::formatFormulae( const std::string& formula, const char * delims, bool richText )
{
    return internal::formatFormulae<char>( formula, "", delims, richText );
}

std::wstring
ChemicalFormula::formatFormulae( const std::wstring& formula, const wchar_t * delims, bool richText )
{
    return internal::formatFormulae<wchar_t>( formula, L"", delims, richText );
}

std::string
ChemicalFormula::make_adduct_string( const std::pair< std::string, std::string >& adduct, bool leading_plus )
{
    return internal::make_adduct_string( adduct, leading_plus );
}

std::wstring
ChemicalFormula::make_adduct_string( const std::pair< std::wstring, std::wstring >& adduct, bool leading_plus )
{
    return internal::make_adduct_string( adduct, leading_plus );
}

///////////////
using namespace adcontrols::internal;

ChemicalFormulaImpl::ChemicalFormulaImpl() 
{
}


