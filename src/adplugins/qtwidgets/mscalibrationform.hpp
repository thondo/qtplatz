// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <adplugin/lifecycle.hpp>
#include <memory>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class MSCalibrateMethod;
    class ProcessMethod;
}

namespace Ui {
    class MSCalibrationForm;
}

namespace qtwidgets {

    class MSCalibrateDelegate;

    class MSCalibrationForm : public QWidget
                            , public adplugin::LifeCycle {

        Q_OBJECT

    public:
        explicit MSCalibrationForm(QWidget *parent = 0);
        ~MSCalibrationForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

        //<--

        // QWidget
        virtual QSize sizeHint() const;
        //<----

    public slots:
        void getContents( adcontrols::ProcessMethod& );
        void handleMSReferencesChanged( const QModelIndex& );
        void getLifeCycle( adplugin::LifeCycle*& );

    signals:
	   void apply( adcontrols::ProcessMethod & );

    private:
        void OnMSReferencesUpdated( const QModelIndex& );

    private:
        Ui::MSCalibrationForm *ui;
        std::unique_ptr< QStandardItemModel > pModel_;
        std::unique_ptr< adportable::Configuration > pConfig_;
        std::unique_ptr< adcontrols::MSCalibrateMethod > pMethod_;
        std::unique_ptr< MSCalibrateDelegate > pDelegate_;

        void update_data( const adcontrols::MSCalibrateMethod& );
    };

}


