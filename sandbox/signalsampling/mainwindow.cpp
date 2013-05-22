// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "mainwindow.hpp"
#include "plot.hpp"
#include <QtCore/QCoreApplication>
#include <QDeclarativeView>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDockWidget>
#include <QUrl>
#include <QMessageBox>
#include <boost/math/distributions/normal.hpp>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , plot_(0)
    , zoomer_( 0 )
    , picker_( 0 )
    , panner_( 0 )
{
    plot_ = new Plot(this);
    setCentralWidget(plot_);

    zoomer_ = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, plot_->canvas() );
    zoomer_->setMousePattern( QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier );
    zoomer_->setMousePattern( QwtEventPattern::MouseSelect3, Qt::RightButton );
    zoomer_->setRubberBand( QwtPicker::RectRubberBand );

    // int res = qmlRegisterType< CentroidMethodModel >( "com.scienceliaison.qml", 1, 0, "CentroidModel" );

    QDeclarativeView * view = new QDeclarativeView;
    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QDeclarativeContext * ctxt = view->rootContext();
    //ctxt->setContextProperty( "centroidModel", pModel_ );

    //view->setSource( QUrl::fromLocalFile( "qml/application.qml" ) );
    view->setSource( QUrl( "qrc:/files/qml/application.qml") );

    QList< QDeclarativeError> errors = view->errors();
    for ( QList<QDeclarativeError>::const_iterator it = errors.begin(); it != errors.end(); ++it )
        QMessageBox::warning( parent, "QDeclarativeError", it->description() );

    view->setMinimumSize( 200, 250 );
    view->resize( QSize( 200, 350 ) );
    QDockWidget * dock = new QDockWidget;
    dock->setWidget( view );

    addDockWidget ( Qt::BottomDockWidgetArea, dock );
    draw_spectrum();
}

MainWindow::~MainWindow()
{

}

void
MainWindow::draw_spectrum()
{
    x_.clear();
    y0_.clear();

    boost::math::normal_distribution<double> nd( to_time( 500 ), to_time( 20 ) );

    double max = boost::math::pdf( nd, to_time( 500 ) );
    const int b18fs = 0x3ffff / 16;

    qDebug() << boost::math::pdf( nd, to_time( 500 ) ) / max * 100;
    qDebug() << boost::math::pdf( nd, to_time( 500 - 5 ) ) / max * 100;
    qDebug() << boost::math::pdf( nd, to_time( 500 - 10 ) ) / max * 100;

    for ( int i = 0; i < 1100; ++i ) {
        x_.push_back( to_time( i ) );
        double y = boost::math::pdf( nd, to_time( i ) ) / max;
        y0_.push_back( y );
        y1_.push_back( int( y * b18fs ) / double(b18fs) );
    }

    plot_->setData( &x_[100], &y0_[0], x_.size() - 100, 0 );
    plot_->setData( &x_[0], &y1_[0], x_.size() - 100, 1 );

    zoomer_->setZoomBase();

}


double
MainWindow::to_time( size_t x )
{
    return x * 2.5;  // microseconds
}

