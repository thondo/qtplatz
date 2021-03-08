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

#pragma once

#include <string>
// #include <compiler/diagnostic_push.h>
// #include <compiler/disable_unused_parameter.h>
// #include <compiler/diagnostic_pop.h>
// #include <memory>
#include <QtCore>
#include <qwt_plot.h>

class QwtPlotMarker;
class QwtText;
class QPointF;

namespace adplot {

    class plot;

    class Annotation {
    public:
        // explicit Annotation( plot&
        //                      , const std::wstring&
        //                      , double x = 0.0
        //                      , double y = 0.0
        //                      , QwtPlot::Axis = QwtPlot::yLeft
        //                      , Qt::GlobalColor color = Qt::darkGreen );
        explicit Annotation( plot&
                             , const QwtText&
                             , const QPointF&
                             , QwtPlot::Axis
                             , Qt::Alignment align = Qt::AlignTop | Qt::AlignHCenter );

        explicit Annotation( plot&
                             , QwtText&&
                             , QPointF&&
                             , QwtPlot::Axis
                             , Qt::Alignment align = Qt::AlignTop | Qt::AlignHCenter );

        ~Annotation();
        Annotation( const Annotation& );

        void setLabelAlighment( Qt::Alignment );
        QwtPlotMarker * getPlotMarker() const;

        static QFont font();

    private:
        class impl;
        impl * impl_;
    };

}
