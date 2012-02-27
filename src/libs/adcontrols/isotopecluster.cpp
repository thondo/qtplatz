// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "isotopecluster.hpp"
#include "tableofelements.hpp"
#include "massspectrum.hpp"
#include "chemicalformula.hpp"
#include "element.hpp"
#include <adportable/combination.hpp>
#include <boost/foreach.hpp>
#include <cmath>
#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

using namespace adcontrols;

namespace adcontrols {

    class IsotopeClusterImpl {
    public:
        ~IsotopeClusterImpl();
        IsotopeClusterImpl();
    };
}

IsotopeCluster::~IsotopeCluster()
{
    delete impl_;
}

IsotopeCluster::IsotopeCluster() : impl_( new IsotopeClusterImpl )
{
}

bool
IsotopeCluster::Compute( const std::wstring& formula, double threshold, bool resInDa, double rp, MassSpectrum& ms, size_t& nPeaks )
{
    (void)formula;
	(void)threshold;
	(void)resInDa;
	(void)rp;
	(void)ms;
	(void)nPeaks;
    return false;
}

bool
IsotopeCluster::Compute( const std::wstring& formula, double threshold, bool resInDa, double rp, MassSpectrum& ms, const std::wstring& adduct, size_t charges, size_t& nPeaks, bool bAccountForElectrons )
{
    (void)formula;
	(void)threshold;
	(void)resInDa;
	(void)rp;
	(void)ms;
	(void)nPeaks;
	(void)bAccountForElectrons;
	(void)charges;
	(void)adduct;
	(void)bAccountForElectrons;
    return false;
}

void
IsotopeCluster::clearFormulae()
{
}

bool
IsotopeCluster::addFormula( const std::wstring& formula, const std::wstring& adduct, size_t chargeState, double relativeAmount )
{
	(void)formula;
	(void)adduct;
	(void)chargeState;
	(void)relativeAmount;
    return false;
}

bool
IsotopeCluster::computeFormulae(double threshold, bool resInDa, double rp,	MassSpectrum& ms, size_t& nPeaks, bool bAccountForElectrons, double ra )
{
	(void)threshold;
	(void)resInDa;
	(void)rp;
	(void)ms;
	(void)nPeaks;
	(void)bAccountForElectrons;
	(void)ra;
    return false;
}

namespace adcontrols {

	struct atom {
		size_t idx;
		const Element * element;
		Element::vector_type::const_iterator iso;
		atom( size_t id, const Element& e ) : idx( id ), element( &e ), iso( e.begin() + id ) {
		}
		atom( const atom& t ) : idx( t.idx ), element( t.element ), iso( t.iso ) {
		}
	};

	struct combination {
		template<class iterator_t>
		static inline void init( iterator_t it, iterator_t& end, size_t nmols ) {
			*it++ = nmols;
			while ( it != end )
				*it++ = 0;
		} 

		template<class iterator_t>
		static inline bool next( iterator_t it, iterator_t& end, size_t nmols ) {
			if ( it == end )
				return false;
			size_t nfree = nmols - *it;
			if ( nfree == 0 || *(end - 1) == nfree ) {
				if ( ( *it ) == 0 )
					return false;
				(*it)--;     // decriment
				init( ++it, end, nfree + 1 );
				return true;
			}
			return next( boost::next( it ), end, nfree );
		}
	};

	struct partial_molecular_mass {
		template<class iterator_t>
		static std::pair<double, double> calculate( const Element& e, iterator_t it, iterator_t end ) {
			double m = 0;
			double a = 0;
			for ( Element::vector_type::const_iterator iso
				= e.begin(); it != end && iso != e.end(); ++it, ++iso ) {
					m += (*it) * iso->mass_;
					a += std::pow( iso->abundance_, int(*it) );
			}
			return std::make_pair( m, a );
		}
	};

}

bool
IsotopeCluster::isotopeDistribution( adcontrols::MassSpectrum& ms
									, const std::wstring& formula
									, size_t charges, bool accountElectron ) 
{
    (void)charges;
    (void)accountElectron;
    (void)ms;
	adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
	ChemicalFormula::elemental_composition_map_t ecomp = ChemicalFormula::getComposition( formula );
    
	double mass = 0;
	for ( ChemicalFormula::elemental_composition_map_t::iterator it = ecomp.begin(); it != ecomp.end(); ++it  ) {

		const Element& element = toe->findElement( it->first );

		do {
			std::vector< size_t > counts( element.isotopeCount(), 0 );

			combination::init( counts.begin(), counts.end(), it->second );
			do {
				std::pair< double, double > m = partial_molecular_mass::calculate( element, counts.begin(), counts.end() );
				std::wcout << std::setw(4) << element.symbol() << ": " 
					<< std::setprecision( 6 ) << std::fixed << std::setw( 10 ) 
					<< m.first << " (" << std::setprecision(4) << m.second << ")\t";
				for ( int i = 0; i < element.isotopeCount(); ++i ) {
					std::cout << counts[i];
					if ( i + 1 < element.isotopeCount() )
						std::cout << ", ";
				}
				std::cout << std::endl;

			} while ( combination::next( counts.begin(), counts.end(), it->second ) );

		} while ( 0 ); // boost::next_mapping( v.begin(), v.end(), vec.begin(), vec.end() ) );


	}
    (void)mass;
	return true;
}

//////////////////////

IsotopeClusterImpl::~IsotopeClusterImpl()
{
}

IsotopeClusterImpl::IsotopeClusterImpl()
{
}
