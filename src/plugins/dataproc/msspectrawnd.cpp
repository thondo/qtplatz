/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "msspectrawnd.hpp"
#include "datafolder.hpp"
#include "dataprocessor.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include "qtwidgets_name.hpp"
#include "selchanged.hpp"
#include "sessionmanager.hpp"

#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adlog/logger.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/mspeaktree.hpp>
#include <adwidgets/msquantable.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <adportable/unique_ptr.hpp>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMenu>
#include <QPrinter>
#include <functional>

namespace dataproc {

    class MSSpectraWnd::impl {
    public:
        impl( MSSpectraWnd * p ) : pThis_( p )
                                 , isTimeAxis_( false )
                                 , dirty_( false ) {

            for ( size_t i = 0; i < plots_.size(); ++i ) {

                plots_[ i ] = std::make_unique< adplot::SpectrumWidget >();
                plots_[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
                plots_[ i ]->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );

                markers_[ i ] = std::make_unique< adplot::PeakMarker >();

            }

        }

        ~impl() {
        }

        MSSpectraWnd * pThis_;

        std::vector< datafolder > data_;
        // std::map< boost::uuids::uuid, datafolder  > dataIds_;

        std::pair< std::wstring, datafolder > profile_;
        std::array< std::unique_ptr< adplot::SpectrumWidget >, 2 > plots_;
        std::array< std::unique_ptr< adplot::PeakMarker >, 2 > markers_;
        bool isTimeAxis_;
        bool dirty_;

    };

}

using namespace dataproc;

MSSpectraWnd::~MSSpectraWnd()
{
    delete impl_;
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) : QWidget(parent)
                                              , impl_( new impl( this ) )
{
    init();
}

void
MSSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {

        for ( size_t i = 0; i < impl_->plots_.size(); ++i ) {

            auto& plot = impl_->plots_[i];
            auto& marker = impl_->markers_[i];

            connect( plot.get()
                     , static_cast< void(adplot::SpectrumWidget::*)(const QRectF&)>(&adplot::SpectrumWidget::onSelected)
                     , [&plot,this]( const QRectF& rc ) { handleSelected( rc, plot.get() ); } );

            // plot->enableAxis( QwtPlot::yRight );
            plot->setMinimumHeight( 80 );
            marker->attach( plot.get() );
            marker->visible( true );
            marker->setYAxis( QwtPlot::yRight );

            if ( i )
                impl_->plots_[ 0 ]->link( plot.get() );

            splitter->addWidget( plot.get() );

        }

        // splitter->addWidget( impl_->table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}


void
MSSpectraWnd::handleSessionAdded( Dataprocessor * processor )
{
    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( auto folder = processor->portfolio().findFolder( L"Spectra" ) ) {
        for ( auto& folium: folder.folio() ) {

            if ( folium.attribute( L"isChecked" ) == L"true" ) {

                if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                    auto it = std::find_if( impl_->data_.begin(), impl_->data_.end()
                                            , [&](const auto& a){ return a.id() == folium.uuid(); });
                    if ( it == impl_->data_.end() ) {
                        impl_->data_.emplace_back( datafolder( processor->filename(), folium ) );
                    }
                }
            }
        }
    }

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    redraw();
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    if ( ! portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) )
        return;

    auto data = datafolder( processor->filename(), folium );

    if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
        auto& plot = impl_->plots_[ 0 ];
        plot->clear();
        plot->setTitle( data.display_name() );
        if ( auto profile = data.get_profile() ) {
            plot->setData( profile->first, 0, QwtPlot::yLeft );
            plot->setAxisTitle( QwtPlot::yLeft, profile->second ? QwtText("Counts") : QwtText( "Intensity (a.u.)" ) );
        }
    } else {
        return;
    }

    auto it = std::find_if( impl_->data_.begin(), impl_->data_.end(), [&]( const auto& a ){ return a.id() == folium.uuid(); } );

    if ( folium.attribute( L"isChecked" ) == L"false" ) {
        if ( it != impl_->data_.end() ) {
            impl_->data_.erase( it );
            impl_->dirty_ = true;
        }
    } else {
        if ( it == impl_->data_.end() ) {
            if ( auto profile = data.get_profile() ) {
                if (( data.overlaySpectrum_ = std::make_shared< adcontrols::MassSpectrum >( *profile->first ) )) {
                    double yMax = adcontrols::segments_helper::max_intensity( *data.overlaySpectrum_ );
                    for ( auto& sp: adcontrols::segment_wrapper<>( *data.overlaySpectrum_ ) ) {
                        for ( size_t i = 0; i < sp.size(); ++i )
                            sp.setIntensity( i, 100 * sp.intensity( i ) / yMax );
                    }
                }
            }
            impl_->data_.emplace_back( data );
            impl_->dirty_ = true;
        }
    }

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( impl_->dirty_ ) {
        redraw();
    }
}

void
MSSpectraWnd::handleSelected( const QRectF& rc, adplot::SpectrumWidget * plot )
{
    // impl_->table_->handleSelected( rc, impl_->isTimeAxis_ );
    auto d = std::abs( plot->transform( QwtPlot::xBottom, rc.left() ) - plot->transform( QwtPlot::xBottom, rc.right() ) );
    if ( d <= 2 ) {

		QMenu menu;
        typedef std::pair < QAction *, std::function<void()> > action_type;
        std::vector < action_type > actions;

        actions.emplace_back( menu.addAction( tr("Copy image to clipboard") ), [=] () { adplot::plot::copyToClipboard( plot ); } );

        actions.emplace_back( menu.addAction( tr( "Save SVG File" ) ) , [=] () {
            QString name = QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File"
                                                         , MainWindow::makePrintFilename( impl_->profile_.first, L"_" )
                                                         , tr( "SVG (*.svg)" ) );
            if ( ! name.isEmpty() )
                adplot::plot::copyImageToFile( plot, name, "svg" );
        });

        actions.emplace_back( menu.addAction( tr("setAxisScale(0,100)") ), [=](){
            plot->setAxisScale( QwtPlot::yRight, 0, 100 );
            plot->replot();
        });

        actions.emplace_back( menu.addAction( tr("replot") ), [=](){
            plot->replot();
        });

        QAction * selected = menu.exec( QCursor::pos() );
        if ( selected ) {
            auto it = std::find_if( actions.begin(), actions.end(), [selected] ( const action_type& a ){ return a.first == selected; } );
            if ( it != actions.end() )
                (it->second)();
        }

    }

}

void
MSSpectraWnd::redraw()
{
    QString title;

    size_t idx(0);
    for ( auto& data: impl_->data_ ) {
        auto traceid = idx++;

        if ( !title.isEmpty() ) {
            title += ";;";
        }

        title += data.display_name();

        QColor color = impl_->plots_[ 1 ]->index_color( traceid );
        if ( auto profile = data.get_profile() ) {
#if __cplusplus >= 201703L
            auto [ ms, isCounts ] = *profile;
#else
            std::shared_ptr< const adcontrols::MassSpectrum > ms; bool isCounts;
            std::tie( ms, isCounts ) = *profile;
#endif
            if ( impl_->data_.size() == 1 ) {
                impl_->plots_[ 1 ]->setData( ms, traceid, QwtPlot::yLeft );
                impl_->plots_[ 1 ]->setColor( traceid, color );
                impl_->plots_[ 1 ]->setAxisTitle( QwtPlot::yLeft, isCounts ? "Counts" : "Intensity (a.u.)");
            } else {
                impl_->plots_[ 1 ]->setData( data.overlaySpectrum_, traceid, QwtPlot::yLeft );
                impl_->plots_[ 1 ]->setColor( traceid, color );
                impl_->plots_[ 1 ]->setAxisTitle( QwtPlot::yLeft, "Intensity (R.A.)" );
            }
        }
    }
    impl_->plots_[ 1 ]->setTitle( title );
    impl_->dirty_ = false;
}

void
MSSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSSpectraWnd::handleAxisChanged( adcontrols::hor_axis axis )
{
    impl_->isTimeAxis_ = ( axis == adcontrols::hor_axis_time );
    for ( auto& plot: impl_->plots_ )
        plot->setAxis( adplot::SpectrumWidget::HorizontalAxis( axis ), true );
}

void
MSSpectraWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
    // check state on navigator
    // will handle at handleSelectionChanged
}

void
MSSpectraWnd::handleCurrentChanged( const QString& guid, int idx, int fcn )
{
    // deprecated -- used to work with quan table
}

void
MSSpectraWnd::onPageSelected()
{
    if ( impl_->dirty_ ) {
        redraw();
    }
}

void
MSSpectraWnd::handleProcessed( Dataprocessor * processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSSpectraWnd::onDataChanged( const QString& foliumGuid, const QString& attGuid, int idx, int fcn )
{
    // data changed on MSPeakTable via MSProcessingWnd
}

void
MSSpectraWnd::handleDataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& data )
{
}

///////////////////////////

void
MSSpectraWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QSizeF sizeMM( 180, 80 );

    int resolution = 85;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );

    printer.setDocName( "QtPlatz Process Report" );
    printer.setOutputFileName( pdfname );
    printer.setResolution( resolution );

    QPainter painter( &printer );

	QRectF boundingRect;
	QRectF drawRect( 0.0, 0.0, printer.width(), (12.0/72)*printer.resolution() );

    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
        painter.drawText( drawRect, Qt::TextWordWrap, QString::fromStdWString( dp->portfolio().fullpath()), &boundingRect );
    }

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

	drawRect.setTop( boundingRect.bottom() );
	drawRect.setHeight( size.height() );
	drawRect.setWidth( size.width() );

    for ( auto& plot : impl_->plots_ ) {

        renderer.render( plot.get(), &painter, drawRect );

        drawRect.setTop( drawRect.bottom() );
        drawRect.setHeight( size.height() );
    }

}
