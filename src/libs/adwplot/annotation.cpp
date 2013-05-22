// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "annotation.hpp"
#include "dataplot.hpp"
#include <qtwrapper/qstring.hpp>
#include <qwt_plot_marker.h>
#include <qwt_text.h>

using namespace adwplot;

Annotation::Annotation( Dataplot& plot
                       , const std::wstring& label
                       , double x, double y
                       , Qt::GlobalColor color )
                       : plot_( &plot )
                       , marker_( new QwtPlotMarker )
{
    marker_->setValue( x, y );
    marker_->setLineStyle( QwtPlotMarker::NoLine );
    marker_->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    // marker_->setLinePen( QPen( Qt::green, 0, Qt::DashDotLine) )
    QwtText text( qtwrapper::qstring::copy( label ) );
    //text.setFont( QFont("Helvetica", 8, QFont::Normal ) );
    text.setFont( QFont("Calibri", 9, QFont::Normal ) );
    text.setColor( color );
    marker_->setLabel( text );
    marker_->attach( plot_ );
}

Annotation::Annotation( const Annotation& t ) : plot_( t.plot_ )
                                              , marker_( t.marker_ )
{
} 

void
Annotation::setLabelAlighment( Qt::Alignment align )
{
    marker_->setLabelAlignment( align );
}