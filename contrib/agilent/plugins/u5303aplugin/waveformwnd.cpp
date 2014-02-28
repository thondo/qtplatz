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

#include "waveformwnd.hpp"
#include "document.hpp"
#include <u5303a/digitizer.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adwplot/tracewidget.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/spectrum_processor.hpp>
#include <coreplugin/minisplitter.h>
#include <QSplitter>
#include <QBoxLayout>

using namespace u5303a;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , spw_( new adwplot::SpectrumWidget )
                                             , tpw_( new adwplot::TraceWidget )
{
    init();
}

WaveformWnd::~WaveformWnd()
{
    fini();
    delete tpw_;
    delete spw_;
}

void
WaveformWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    do {
        splitter->addWidget( tpw_ );
        splitter->addWidget( spw_ );
        spw_->setAxis( adwplot::SpectrumWidget::HorizontalAxisTime );
        spw_->setKeepZoomed( false );
        splitter->setOrientation( Qt::Vertical );
    } while(0);
    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 2 );
    layout->addWidget( splitter );
}

void
WaveformWnd::fini()
{
}

void
WaveformWnd::onInitialUpdate()
{
}

void
WaveformWnd::handle_waveform()
{
    if ( auto waveform = document::instance()->findWaveform() ) {
        double dbase(0), rms(0);
		const int32_t * data = waveform->d_.data();
		double tic = adportable::spectrum_processor::tic( waveform->d_.size(), data, dbase, rms );

		// data interpreter need to be implementd --- TBD

        sp_ = std::make_shared< adcontrols::MassSpectrum >();
		sp_->setCentroid( adcontrols::CentroidNone );

        adcontrols::MSProperty prop = sp_->getMSProperty();
		adcontrols::MSProperty::SamplingInfo info( 0
                                                   , waveform->method_.delay_to_first_s
                                                   , uint32_t(waveform->d_.size())
                                                   , waveform->method_.nbr_of_averages + 1
                                                   , 0 );
		info.fSampInterval( 1.0 / waveform->method_.samp_rate );
        prop.acceleratorVoltage( 3000 );
		prop.setSamplingInfo( info );
        using namespace adcontrols::metric;
        prop.setTimeSinceInjection( double( waveform->timestamp_ ) ); // to base
        prop.setDataInterpreterClsid( "u5303a" );
        // prop.setDeviceData(); TBA
        sp_->setMSProperty( prop );

		sp_->resize( waveform->d_.size() );
		int idx = 0;
		for ( auto y: waveform->d_ )
			sp_->setIntensity( idx++, y );
		spw_->setData( sp_, 0 );
    }
}