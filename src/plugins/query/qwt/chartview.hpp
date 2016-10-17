/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#pragma once

#include <qwt_plot.h>
#include <memory>

class XYSeriesData;
class QwtPlotCurve;
class QAbstractItemModel;

namespace query {

    namespace qwt {

        class ChartView : public QwtPlot {

            Q_OBJECT

        public:
            ChartView( QWidget * parent = 0 );
            
            ~ChartView();

            void setData( QAbstractItemModel *, const QString& title, int x, int y
                          , const QString& xLabel, const QString& yLabel, const QString& chartType );

            void clear();
        private:
            std::vector< std::shared_ptr< QwtPlotCurve > > plots_;
        };

    }

}
