/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "ap240form.hpp"
#include "ap240horizontalform.hpp"
#include "ap240verticalform.hpp"
#include "ap240triggerform.hpp"
#include "ui_ap240form.h"
#include <acqrscontrols/ap240/method.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/threshold_action.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <adwidgets/thresholdactionform.hpp>
#include <adwidgets/findslopeform.hpp>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPair>
#include <boost/exception/all.hpp>

using namespace acqrswidgets;

ap240form::ap240form(QWidget *parent) : QWidget(parent)
                                      , ui( new Ui::ap240form )
{
    ui->setupUi(this);

    // Software TDC (Slope Time Converter) UI
    if ( auto layout = new QVBoxLayout( ui->groupBox_2 ) ) {    

        ///////////// Slope detect ////////////////
        if ( auto tab = new QTabWidget() ) {
            layout->addWidget( tab );
            int idx = 0;
            for ( auto& title : { tr( "CH1" ), tr( "CH2" ) } ) {
                auto ch = new adwidgets::findSlopeForm();
                ch->setTitle( idx++, title );
                ch->setObjectName( title );
                tab->addTab( ch, title );
                // enable|disable
                connect( ch, &adwidgets::findSlopeForm::valueChanged, [this] ( int ch ) {
                    emit valueChanged( idSlopeTimeConverter, ch ); } );
            }

            ////////// Threshold Action ///////////
            auto form = new adwidgets::ThresholdActionForm();
            form->setObjectName( "ThresholdActionForm" );
            tab->addTab( form, tr( "Action" ) );
            connect( form, &adwidgets::ThresholdActionForm::valueChanged, [this] () { emit valueChanged( idThresholdAction, 0 ); } );
        }
    }

    // Vertical configuration
    if ( auto layout = new QVBoxLayout( ui->groupBox_3 ) ) {
        // ui->horizontalLayout_2->insertLayout( 2, layout );
        if ( auto tab = new QTabWidget() ) {
            layout->addWidget( tab );
            for ( auto& channel :
                { std::make_pair( 1, tr( "CH-1" ) ), std::make_pair( 2, tr( "CH-2" ) ), std::make_pair( -1, tr( "Ext") ) } )
                if ( auto gbox = new QGroupBox( channel.second, this ) ) {
                    gbox->setObjectName( channel.second );
                    gbox->setCheckable( true );
                    gbox->setTitle( channel.second );
                    
                    auto layout = new QVBoxLayout( gbox );
                    layout->setMargin( 0 );
                    layout->setSpacing( 0 );

                    auto w = new ap240VerticalForm();
                    w->setChannel( channel.first );
                    layout->addWidget( w );
                    tab->addTab( gbox, channel.second );

                    connect( w, &ap240VerticalForm::valueChanged, [this] ( ap240VerticalForm::idItem id, int channel, const QVariant& ) {
                            emit valueChanged( idVertical, channel ); } );
                    connect( gbox, &QGroupBox::toggled, [this] ( bool on ) { emit valueChanged( idChannels, -1 ); } );
                }
        }
    }
    
    if ( auto layout = new QVBoxLayout( ui->groupBox ) ) {
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        auto w = new ap240TriggerForm();
        layout->addWidget( w );
        connect( w, &ap240TriggerForm::valueChanged, [this] ( ap240TriggerForm::idItem id, const QVariant& d ) {
                emit valueChanged( idTrigger, -1 );
            } );
    }

    if ( auto layout = new QVBoxLayout( ui->groupBox_4 ) ) {
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        auto w = new ap240HorizontalForm();
        layout->addWidget( w );
        connect( w, &ap240HorizontalForm::valueChanged, [this] ( ap240HorizontalForm::idItem id, const QVariant& d ) {
                emit valueChanged( idHorizontal, -1 );
            } );

    }
    set( acqrscontrols::ap240::method() );
}

ap240form::~ap240form()
{
    delete ui;
}

// LifeCycle
void
ap240form::OnCreate( const adportable::Configuration& )
{
}

void
ap240form::OnInitialUpdate()
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm * >() )
        form->OnInitialUpdate();

    // don't response to each key strokes on DoubleSpinBox
    for ( auto spin : findChildren< QDoubleSpinBox * >() )
        spin->setKeyboardTracking( false );
}

void
ap240form::OnFinalClose()
{
}

bool
ap240form::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);        
        
        acqrscontrols::ap240::method m;
        get( m );
        adcontrols::ControlMethod::MethodItem item( m.clsid(), m.modelClass() );
        item.setItemLabel( "ap240" );
        item.set<>( item, m ); // serialize
        ptr->insert( item );
        
        return true;
        
    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {
        
        auto pi = boost::any_cast<adcontrols::ControlMethod::MethodItem *>( a );
        acqrscontrols::ap240::method m;
        get( m );
        pi->setModelname( "ap240" );
        pi->setItemLabel( "ap240" );
        pi->unitnumber( 1 );
        pi->funcid( 1 );
        pi->set<>( *pi, m ); // serialize
        return true;
    } else if ( adportable::a_type< acqrscontrols::ap240::method >::is_pointer( a ) ) {

        auto pm = boost::any_cast<acqrscontrols::ap240::method *>( a );
        get( *pm );
        return true;

    }
    return false;
}

bool
ap240form::setContents( boost::any&& a )
{
    const adcontrols::ControlMethod::MethodItem * pi(0);

    if ( adportable::a_type < std::shared_ptr< const adcontrols::ControlMethod::Method > >::is_a( a ) ) {
        // adcontrols::ControlMethod::Method
        // find first one
        if ( auto ptr = boost::any_cast<std::shared_ptr< const adcontrols::ControlMethod::Method>>( a ) ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::ap240::method::clsid() );
            if ( it != ptr->end() )
                pi = &( *it );
        }

    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {

        pi = boost::any_cast<const adcontrols::ControlMethod::MethodItem * >( a );             

    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_a( a ) ) {   

        pi = &boost::any_cast<const adcontrols::ControlMethod::MethodItem& >( a );
    }

    if ( pi ) {
        acqrscontrols::ap240::method m;
		try {
            pi->get<>( *pi, m );
            set( m );
		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}

    }
    return true;
}

void
ap240form::onInitialUpdate()
{
}

void
ap240form::onStatus( int )
{
}

void
ap240form::set( const acqrscontrols::ap240::method& m )
{
    if ( auto gbox = findChild< QGroupBox * >( "CH-1" ) ) {
        QSignalBlocker block( gbox );
        gbox->setChecked( m.channels_ & 0x01 );
    }
    if ( auto gbox = findChild< QGroupBox * >( "CH-2" ) ) {
        QSignalBlocker block( gbox );
        gbox->setChecked( m.channels_ & 0x02 );
    }
    if ( auto gbox = findChild< QGroupBox * >( "Ext" ) ) {
        QSignalBlocker block( gbox );
        gbox->setChecked( m.trig_.trigPattern & 0x80000000 );
    }

    if ( auto form = findChild< ap240HorizontalForm *>() ) {
        QSignalBlocker block( form );
        form->set( m );
    }
    
    if ( auto form = findChild< ap240TriggerForm *>() ) {
        QSignalBlocker block( form );
        form->set( m );        
    }
    
    for ( auto form : findChildren< ap240VerticalForm * >() ) {
        QSignalBlocker block( form );
        form->set( m );        
    }
    set( 0, m.slope1_ );
    set( 1, m.slope2_ );
    set( m.action_ );
}

void
ap240form::get( acqrscontrols::ap240::method& m ) const
{
    uint32_t channels( 0 );
    
    if ( auto gbox = findChild< QGroupBox * >( "CH-1" ) ) {
        if ( gbox->isChecked() )
            channels |= 1;             
    }
    if ( auto gbox = findChild< QGroupBox * >( "CH-2" ) ) {
        if ( gbox->isChecked() )
            channels |= 2;
    }    
    m.channels_ = channels;
 
    if ( auto form = findChild< ap240HorizontalForm *>() ) {
        form->get( m );
    }
    
    if ( auto form = findChild< ap240TriggerForm *>() ) {
        form->get( m );        
    }
    
    for ( auto form : findChildren< ap240VerticalForm * >() ) {
        form->get( m );        
    }

    get( 0, m.slope1_ );
    get( 1, m.slope2_ );
    get( m.action_ );
}

void
ap240form::get( int ch, adcontrols::threshold_method& m ) const
{
    const QString names[] = { "CH1", "CH2" };

    if ( auto form = findChild< adwidgets::findSlopeForm * >( names[ ch ] ) ) {
        form->get( m );
    }
}

void
ap240form::set( int ch, const adcontrols::threshold_method& m )
{
    const QString names[] = { "CH1", "CH2" };

    if ( auto form = findChild< adwidgets::findSlopeForm * >( names[ ch ] ) ) {
        form->set( m );
    }
}

void
ap240form::get( adcontrols::threshold_action& m ) const
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->get( m );
}

void
ap240form::set( const adcontrols::threshold_action& m )
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->set( m );
}
