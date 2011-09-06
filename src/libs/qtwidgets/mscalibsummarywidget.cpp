// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "mscalibsummarywidget.hpp"
#include "mscalibsummarydelegate.hpp"
#include <QStandardItemModel>
#include <QStandardItem>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <qtwrapper/qstring.hpp>

using namespace qtwidgets;

MSCalibSummaryWidget::~MSCalibSummaryWidget()
{
}

MSCalibSummaryWidget::MSCalibSummaryWidget(QWidget *parent) : QTableView(parent)
                                                            , pModel_( new QStandardItemModel )
                                                            , pDelegate_( new MSCalibSummaryDelegate ) 
{
    this->setModel( pModel_.get() );
    this->setItemDelegate( pDelegate_.get() );
}

void
MSCalibSummaryWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSCalibSummaryWidget::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QTableView& tableView = *this;

    QStandardItem * rootNode = model.invisibleRootItem();
    tableView.setRowHeight( 0, 7 );
    rootNode->setColumnCount( 10 );
    
    int col = 0;
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "m/z(observed)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "Intensity" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "m/z(exact)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "m/z(calibrated)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "error(mDa) w/ before calib" ) );
}

void
MSCalibSummaryWidget::OnUpdate( boost::any& )
{
}

void
MSCalibSummaryWidget::OnFinalClose()
{
}

void
MSCalibSummaryWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

void
MSCalibSummaryWidget::setData( const adcontrols::MSCalibrateResult& res, const adcontrols::MassSpectrum& ms )
{
    QStandardItemModel& model = *pModel_;
    model.removeRows( 0, model.rowCount() );

    if ( ! ms.isCentroid() )
        return;

    const double * intensities = ms.getIntensityArray();
    const double * times = ms.getTimeArray();
    const double * masses = ms.getMassArray();

    std::vector< size_t > indecies;
    for ( size_t i = 0; i < ms.size(); ++i ) {
        if ( intensities[i] >= res.threshold() )
            indecies.push_back( i ); 
    }

    model.insertRows( 0, indecies.size() );

    for ( size_t row = 0; row < indecies.size(); ++row ) {
        size_t idx = indecies[ row ];
        int col = 0;
        model.setData( model.index( row, col++ ), masses[ idx ] );
        model.setData( model.index( row, col++ ), times[ idx ] * 1.0e6); // s -> us
        model.setData( model.index( row, col++ ), intensities[ idx ] );
    }

    const adcontrols::MSReferences& ref = res.references();
    const adcontrols::MSAssignedMasses& assigned = res.assignedMasses();
    adcontrols::MSReferences::vector_type::const_iterator refIt = ref.begin();

    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it, ++refIt ) {
        int col = 3;
        const adcontrols::MSAssignedMass& a = *it;

        std::vector< size_t >::iterator index
            = std::lower_bound( indecies.begin(), indecies.end(), a.idMassSpectrum() );
        size_t row = std::distance( indecies.begin(), index );

        model.setData( model.index( row, col++ ), qtwrapper::qstring::copy( a.formula() ) );
        model.setData( model.index( row, col++ ), refIt->exactMass() );
        model.setData( model.index( row, col++ ), a.mass() );
        model.setData( model.index( row, col++ ), ( a.mass() - refIt->exactMass() ) * 1000 ); // mDa
        model.setData( model.index( row, col++ ), ( a.mass() - masses[ a.idMassSpectrum() ] ) * 1000 ); // mDa
        model.setData( model.index( row, col++ ), refIt->enable() );
    }

}

