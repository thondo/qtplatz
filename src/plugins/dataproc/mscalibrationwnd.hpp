// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#ifndef MSCALIBRATIONWND_H
#define MSCALIBRATIONWND_H

#include <QWidget>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class MassSpectrum;
    class MSCalibrateResult;
    class ProcessMethod;
}

namespace adportable {  class Configuration; }
namespace portfolio  { class Folium; }

namespace dataproc {

    class Dataprocessor;
    class MSCalibrationWndImpl;

    class MSCalibrationWnd : public QWidget {
        Q_OBJECT
        public:
        // explicit MSCalibrationWnd(QWidget *parent = 0);
        MSCalibrationWnd( const adportable::Configuration& c, const std::wstring& apppath, QWidget * parent = 0 );
        void init( const adportable::Configuration& c, const std::wstring& apppath );
      
    signals:
        void fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
      
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );

    private slots:
        void handleSelSummary( size_t );
        void handleManuallyAssigned();
        void handleValueChanged();

    private:
        boost::shared_ptr<MSCalibrationWndImpl> pImpl_;
    };

}

#endif // MSCALIBRATIONWND_H
