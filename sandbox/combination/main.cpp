/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <QtCore/QCoreApplication>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <adportable/combination.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>

#  if defined NDEBUG
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adportable.lib")
#  else
#     pragma comment(lib, "../../lib/qtplatz/adcontrolsd.lib")
#  endif

int
main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	std::vector< std::string > isotopes;

    isotopes.push_back( "16O" );
    isotopes.push_back( "17O" );    
    isotopes.push_back( "18O" );
  
	std::vector< std::vector< std::string >::const_iterator > v( 2, isotopes.begin() );

	do {
        for ( int i = 0; i < 2; ++i )
			std::cout << *v[i] << ", ";
		std::cout << std::endl;
	//} while ( boost::next_partial_permutation( isotopes.begin(), isotopes.begin() + 2, isotopes.end() ) );
	} while ( boost::next_mapping( v.begin(), v.end(), isotopes.begin(), isotopes.end() ) );

	//
	adcontrols::MassSpectrum ms;
	adcontrols::IsotopeCluster::isotopeDistribution( ms, L"C6H5Cl" );
    
    return a.exec();
}
