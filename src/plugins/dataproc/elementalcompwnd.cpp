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

#include "elementalcompwnd.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/timeutil.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adlog/logger.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/polfit.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/folium.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <QBoxLayout>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

using namespace dataproc;

namespace dataproc {

    class ElementalCompWndImpl {
    public:
        ~ElementalCompWndImpl() {}
        ElementalCompWndImpl() : ticPlot_(0)
                               , referenceSpectrum_(0)
                               , processedSpectrum_(0)
                               , drawIdx_(0) {
        }
      
        adplot::ChromatogramWidget * ticPlot_;
        adplot::SpectrumWidget * referenceSpectrum_;
        adplot::SpectrumWidget * processedSpectrum_;
        int drawIdx_;
    };

}

ElementalCompWnd::~ElementalCompWnd()
{
    delete pImpl_;
}

ElementalCompWnd::ElementalCompWnd(QWidget *parent) : QWidget(parent)
                                                    , pImpl_(0)
{
    init();
}

void
ElementalCompWnd::init()
{
    pImpl_ = new ElementalCompWndImpl;
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( ( pImpl_->processedSpectrum_ = new adplot::SpectrumWidget(this) ) )
            splitter->addWidget( pImpl_->processedSpectrum_ );
        if ( ( pImpl_->referenceSpectrum_ = new adplot::SpectrumWidget(this) ) )
            splitter->addWidget( pImpl_->referenceSpectrum_ );

        pImpl_->processedSpectrum_->setMinimumHeight( 80 );
        pImpl_->referenceSpectrum_->setMinimumHeight( 80 );        
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
		pImpl_->referenceSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );
		pImpl_->referenceSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );        

        pImpl_->processedSpectrum_->link( pImpl_->referenceSpectrum_ );
        pImpl_->referenceSpectrum_->link( pImpl_->processedSpectrum_ );

        splitter->setOrientation( Qt::Vertical );
    }
  
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
ElementalCompWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    pImpl_->referenceSpectrum_->setData( ptr, 0 );
}

void
ElementalCompWnd::estimateScanLaw( adutils::MassSpectrumPtr& ptr )
{
    std::cout << "estimateScanLaw" ;

    auto annots = ptr->get_annotations();
    std::vector< std::pair< int, std::string >  > ids;
    std::for_each( annots.begin(), annots.end(), [&ids] ( const adcontrols::annotation& a ) {
            if ( a.dataFormat() == adcontrols::annotation::dataFormula && a.index() >= 0 )
                ids.push_back( std::make_pair( a.index(), a.text() ) );
        });
    
    std::vector< std::pair< double, double > > time_mass_v;
    for ( auto& id : ids ) {
        //double time = ptr->getTime( id.first );
        //std::pair<std::string, std::string> adduct;
        //auto formula = adcontrols::ChemicalFormula::splitFormula( adduct, id.second, false );
        //double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula, adduct );
        //time_mass_v.push_back( std::make_pair( time, exactMass ) );
    }
    if ( time_mass_v.empty() )
        return;
    if ( time_mass_v.size() == 1 ) {
        double va = adportable::TimeSquaredScanLaw::acceleratorVoltage( time_mass_v[0].first, time_mass_v[0].second, 0.5, 0 );
        ADTRACE() << "Estimated scanLaw: Vacc=" << va;
    } else {
        std::vector<double> x, y, coeffs;
        for ( auto& xy: time_mass_v ) {
            x.push_back( std::sqrt( xy.second ) * 0.5 );
            y.push_back( xy.first );
        }
        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {
            double t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 );
            double va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );
        }
    }
}

void
ElementalCompWnd::handleAxisChanged( int axis )
{
    using adplot::SpectrumWidget;
    pImpl_->referenceSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
    pImpl_->processedSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
}

void
ElementalCompWnd::handleSessionAdded( Dataprocessor * )
{
}

void
ElementalCompWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
ElementalCompWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    pImpl_->drawIdx_ = 0;

    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

    if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            if ( ptr->size() > 0
                 && adportable::compare<double>::approximatelyEqual( ptr->getMass( ptr->size() - 1 ), ptr->getMass( 0 ) ) ) {
                // no mass assigned
                pImpl_->processedSpectrum_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime, false );
            } else {
                pImpl_->processedSpectrum_->setAxis( adplot::SpectrumWidget::HorizontalAxisMass, false );
            }
            pImpl_->referenceSpectrum_->enableAxis( QwtPlot::yRight, true );            
            pImpl_->processedSpectrum_->enableAxis( QwtPlot::yRight, true );
            pImpl_->processedSpectrum_->setData( ptr, 1, true );
            pImpl_->processedSpectrum_->setAlpha( 1, 0x20 );

            estimateScanLaw( ptr );
        }

        portfolio::Folio attachments = folium.attachments();
        if ( auto fcentroid = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                    return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
        
            if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
                if ( centroid->isCentroid() ) {
                    pImpl_->processedSpectrum_->setData( centroid, 0, false );
                }
            }
        }

    }
}

void
ElementalCompWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
ElementalCompWnd::simulate( const adcontrols::MSSimulatorMethod& m )
{
    std::vector< std::string > display_formulae; // will expand from molecule + adduct combination
    std::vector< std::pair< std::string, double > > formula_abundances;

    for ( auto& mol : m.molecules().data() ) {
        if ( mol.enable ) {
            if ( !mol.adducts.empty() ) {
                std::vector< std::string > alist;
                auto v = adcontrols::ChemicalFormula::standardFormulae( mol.formula, mol.adducts, alist );
                for ( size_t i = 0; i < v.size(); ++i ) {
                    formula_abundances.push_back( std::make_pair( v[ i ], mol.abundance ) );
                    display_formulae.push_back( mol.formula + alist[ i ] );
                }
            } else {
                formula_abundances.push_back( std::make_pair( mol.formula, mol.abundance ) );
                display_formulae.push_back( mol.formula );
            }
        }
    }

    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    ms->setCentroid( adcontrols::CentroidNative );

    adcontrols::isotopeCluster()( *ms, formula_abundances, m.resolving_power() );

    // annotation
    for ( size_t i = 0; i < formula_abundances.size(); ++i ) {
        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula_abundances[ i ].first );
        auto pos = ms->find( mass, mass / m.resolving_power() );
        if ( pos != adcontrols::MassSpectrum::npos ) {
            auto& annots = ms->get_annotations();
            annots << adcontrols::annotation( display_formulae[ i ], mass, ms->getIntensity( pos ), int( pos ), 0, adcontrols::annotation::dataFormula );
        }
    }

	double lMass = ms->getMass( 0 );
	double hMass = ms->getMass( ms->size() - 1 );
    lMass = m.lower_limit() > 0 ? m.lower_limit() : double( int( lMass / 10 ) * 10 );
    hMass = m.upper_limit() > 0 ? m.upper_limit() : double( int( ( hMass + 10 ) / 10 ) * 10 );
    ms->setAcquisitionMassRange( lMass, hMass );
    draw1( ms );
}
