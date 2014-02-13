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

#include "formula_parser.hpp"
#include "element.hpp"
#include "molecule.hpp"
#include "tableofelement.hpp"
#include "isotopecluster.hpp"
#include <iostream>
#include <chrono>
#include <boost/format.hpp>

bool
scanner( std::string& line, molecule& mol )
{
    std::string::const_iterator it = line.begin();
    std::string::const_iterator end = line.end();

    chem::comp_type elemental_composition;
    chem::chemical_formula_parser< std::string::const_iterator > parser;
    if ( boost::spirit::qi::parse( it, end, parser, elemental_composition ) && it == end ) {

        for ( auto& atom: elemental_composition ) {

            const char * symbol = atom.first.second;
            uint32_t count = atom.second;

            if ( element e = tableofelement::findElement( symbol ) ) {

                e.count( count );  // number of atoms

                // make it alphabetical order comforming to organic chemistry standard
                auto it = std::lower_bound( mol.elements.begin(), mol.elements.end(), symbol, [](const element& lhs, const char * symbol ){
                        return std::strcmp( lhs.symbol(), symbol ) < 0;
                    });
                mol.elements.insert( it, e );

            }
        }
        return true;
    }
    return false;
}

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::string line;

    while (std::getline(std::cin, line))  {

        if (line.empty() || line[0] == 'q' || line[0] == 'Q')
            break;

        molecule mol;
        if ( scanner( line, mol ) ) {
            std::cout << "formula: ";
            std::for_each( mol.elements.begin(), mol.elements.end(), [&]( const element& e ){
                    std::cout << e.symbol() << e.count() << " ";
                });
            std::cout << std::endl;

            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

            isotopecluster cluster;
			cluster( mol );

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            
            auto it = std::max_element( mol.cluster.begin(), mol.cluster.end()
                                            , [](const mol::isotope& a, const mol::isotope& b){
                                            return a.abundance < b.abundance;});
            const double pmax = it->abundance;

            int idx = 0;
            for ( auto& i: mol.cluster ) {
                double ratio = i.abundance / pmax * 100.0;
                std::cout << boost::format( "[%2d] %.8f\t%.8e\t%.6f\n" ) % idx++ % i.mass % i.abundance % ratio;
                if ( ratio < 0.05 )
                    break;
            }
            std::cout << "processed in: "
                      << std::chrono::duration_cast< std::chrono::microseconds >( end - start ).count() << "us."
                      << std::endl;
        } else {
            std::cout << "Parse failed: " << line << std::endl;
        }
    }
    std::cout << "Bye... :-) \n\n";
	return 0;
}

