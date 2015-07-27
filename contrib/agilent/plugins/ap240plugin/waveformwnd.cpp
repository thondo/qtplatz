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
#include <ap240/digitizer.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/trace.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <coreplugin/minisplitter.h>
#include <QSplitter>
#include <QBoxLayout>
#include <boost/format.hpp>
#include <chrono>
#include <sstream>

using namespace ap240;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , spw_( new adplot::SpectrumWidget )
                                             , tpw_( new adplot::ChromatogramWidget )
{
    for ( auto& tp: tp_ )
        tp = std::make_shared< adcontrols::Trace >();
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
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 3 );
        splitter->setOrientation( Qt::Vertical );
    } while(0);

    spw_->setAxisTitle( QwtPlot::yLeft, tr( "mV" ) );
    spw_->enableAxis( QwtPlot::yRight, true );
    spw_->setAxisTitle( QwtPlot::yRight, tr( "mV" ) );
    
    spw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    spw_->setKeepZoomed( false );

    tpw_->setAxisTitle( QwtPlot::yLeft, tr( "mV" ) );
    tpw_->setAxisTitle( QwtPlot::yRight, tr( "mV" ) );
    spw_->enableAxis( QwtPlot::yRight, true );

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
    spw_->setKeepZoomed( false );
}

void
WaveformWnd::handle_waveform()
{
    auto pair = document::instance()->findWaveform();

    std::array< double, 2 > levels = { document::instance()->threshold( 0 ) * 1000
                                       , document::instance()->threshold( 1 ) * 1000 }; // mV

    std::ostringstream o;

    uint64_t duration(0);
    
    for ( auto waveform: { pair.first, pair.second } ) {

        if ( waveform ) {
            double dbase(0), rms(0), tic(0);
            double timestamp = waveform->meta_.initialXTimeSeconds;
            int channel = waveform->meta_.channel - 1;
            if ( channel > 2 )
                continue; // this should not be happend.
            
            sp_[ channel ] = std::make_shared< adcontrols::MassSpectrum >();
            auto sp = sp_[ channel ];
            auto tp = tp_[ channel ];

            auto tp0 = std::chrono::steady_clock::now();
            
            sp->setCentroid( adcontrols::CentroidNone );
            sp->resize( waveform->size() );

            if ( waveform->meta_.dataType == 1 ) {
                size_t idx = 0;
                for ( auto it = waveform->begin<int8_t>(); it != waveform->end<int8_t>(); ++it )
                    sp->setIntensity( idx++, waveform->toVolts( *it ) * 1000.0 ); // mV
            } else if ( waveform->meta_.dataType == 2 ) {
                size_t idx = 0;                
                for ( auto it = waveform->begin<int16_t>(); it != waveform->end<int16_t>(); ++it )
                    sp->setIntensity( idx++, *it );
            } else if ( waveform->meta_.dataType == 4 ) {
                size_t idx = 0;                
                for ( auto it = waveform->begin<int32_t>(); it != waveform->end<int32_t>(); ++it )
                    sp->setIntensity( idx++, *it );
            }
            
            adcontrols::MSProperty prop = sp->getMSProperty();
            adcontrols::MSProperty::SamplingInfo info( 0
                                                       , uint32_t( waveform->meta_.initialXOffset / waveform->meta_.xIncrement + 0.5 )
                                                       , uint32_t( waveform->size() )
                                                       , waveform->meta_.actualAverages
                                                       , 0 );
            
            info.fSampInterval( waveform->meta_.xIncrement );
            info.horPos( waveform->meta_.horPos );
            
            prop.setSamplingInfo( info );
            prop.acceleratorVoltage( 3000 );

            using namespace adcontrols::metric;
            prop.setTimeSinceInjection( timestamp * 1.0e6 ); // microseconds
            prop.setTimeSinceEpoch( waveform->timeSinceEpoch_ );
            prop.setDataInterpreterClsid( "ap240" );
            sp->setMSProperty( prop );

            const double * p = sp->getIntensityArray();
            bool flag = levels[ channel ] < (*p);
            int stage = 0;
            std::array< size_t, 2 > th = { 0, 0 };
            for ( auto it = p; it != p + sp->size(); ++it ) {
                if ( ( levels[ channel ] < *it ) != flag ) {
                    flag = !flag;
                    th[ stage++ ] = it - p;
                    if ( stage >= th.size() )
                        break;
                }
            }

            double t0 = sp->getTime( th[0] ) * 1.0e6;
            double t1 = sp->getTime( th[1] ) * 1.0e6;

            duration += std::chrono::nanoseconds( std::chrono::steady_clock::now() - tp0 ).count();
            
            if ( o.str().empty() )
                o << boost::format( "Time: %.3lf" ) % waveform->meta_.initialXTimeSeconds;
            
            o << boost::format( " CH%d RMS: %.3f %5.1fmV level: [%.0fmV]=(%.4f:%4f)(us); " )
                % ( channel + 1 ) % rms % tic % levels[ channel ] % t0 % t1;

            tic = adportable::spectrum_processor::tic( sp->size(), sp->getIntensityArray(), dbase, rms );
            tp->push_back( waveform->serialnumber_, timestamp, tic );
            tpw_->setData( *tp, channel, channel );
            
            // prop.setDeviceData(); TBA
            spw_->setData( sp, channel, channel );
            spw_->setKeepZoomed( true );
        }
        std::cout << "duration: " << double(duration) / 1000.0 << "(us)" << std::endl;

        spw_->setTitle( o.str() );
    }
}
