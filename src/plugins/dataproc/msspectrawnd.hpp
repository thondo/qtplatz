// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <QWidget>
#include <memory>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
	class PeakResult;
    class ProcessMethod;
}

namespace adwidgets { class MSMergedTable; }
namespace adwplot { class SpectrumWidget;  }

namespace dataproc {

    class Dataprocessor;

    class MSSpectraWnd : public QWidget {
        Q_OBJECT
    public:
        explicit MSSpectraWnd( QWidget *parent = 0);
        ~MSSpectraWnd();
        
        void draw1( std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw2( std::shared_ptr< adcontrols::MassSpectrum >& );
        
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handleAxisChanged( int );
        void handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool isChecked );
        
    private:
        void init();

        std::unique_ptr< adwidgets::MSMergedTable > table_;
        std::unique_ptr< adwplot::SpectrumWidget > plot_;
    };
}

