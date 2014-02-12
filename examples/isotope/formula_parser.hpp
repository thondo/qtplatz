/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#if defined _MSC_VER
# pragma warning(disable:4100)
#endif

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <boost/format.hpp>

namespace chem {

    namespace qi = boost::spirit::qi;
    using boost::phoenix::bind;
    using boost::spirit::qi::_val;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;

    const char * element_table [] = {
        "H",                                                                                                              "He",
        "Li", "Be",                                                                         "B",  "C",  "N",  "O",  "F",  "Ne", 
        "Na", "Mg",                                                                         "Al", "Si", "P",  "S",  "Cl", "Ar",
        "K",  "Ca", "Sc", "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",  
        "Rb", "Sr", "Y",  "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I",  "Xe",  
        "Cs", "Ba", "Lu", "Hf", "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
        "Fr", "Ra", "Ac", "Th", "Pa", "U",  "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
        // Lanthanoids
        "La", "Ce", "Pr", "Nd", "Pm", "Sm",  "Eu", "Gd",  "Tb", "Dy",  "Ho", "Er" "Tm", "Yb",
        // Actinoids
        "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No"
    };

    typedef std::pair< int, const char * > atom_type;
    typedef std::map< atom_type, size_t > comp_type;

    struct formulaComposition {
        static void formula_add( comp_type& m, const std::pair<const atom_type, std::size_t>& p ) {
            m[ p.first ] += p.second;
        }
        
        static void formula_join( comp_type& m, map_type& a ) {
            for( map_type::value_type& p: a )
                m[ p.first ] += p.second;
        }
        
        static void formula_repeat( comp_type& m, std::size_t n ) {
            for( map_type::value_type& p: m )
                p.second *= n;
        }
    };

    template<typename Iterator, typename handler = formulaComposition, typename startType = comp_type>
    struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, startType() > {

        chemical_formula_parser() : chemical_formula_parser::base_type( molecule ), element( element_table, element_table )  {
            molecule =
				+ (
                    atoms            [ boost::phoenix::bind(&handler::formula_add, _val, qi::_1) ]
                    | repeated_group [ boost::phoenix::bind(&handler::formula_join, _val, qi::_1 ) ]
                    | space
                    )
                ;
            atoms = 
                atom >> ( qi::uint_ | qi::attr(1u) ) // default to 1
                ;
            atom =
                ( qi::uint_ | qi::attr(0u) ) >> element
                ;
            repeated_group %= // forces attr proparation
                '(' >> molecule >> ')'
                    >> qi::omit[ qi::uint_[ boost::phoenix::bind( handler::formula_repeat, qi::_val, qi::_1 ) ] ]
                ;
        }

        qi::rule<Iterator, atom_type() > atom;
        qi::rule<Iterator, std::pair< atom_type, std::size_t >() > atoms;
        qi::rule<Iterator, startType()> molecule, repeated_group;
        qi::symbols<char, const char *> element;
    };

}

