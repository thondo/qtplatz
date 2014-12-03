/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "u5303amethodwidget.hpp"
#include "u5303aform.hpp"
#include "u5303amethodtable.hpp"
#include "document.hpp"
#include <u5303a/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adinterface/controlserver.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <u5303a/digitizer.hpp>
#include <QSplitter>
#include <QBoxLayout>

using namespace u5303a;

u5303AMethodWidget::u5303AMethodWidget(QWidget *parent) : QWidget(parent)
{
    if ( QSplitter * splitter = new QSplitter ) {

        splitter->addWidget( new u5303AForm(this) );
        splitter->addWidget( new u5303AMethodTable(this) );
        splitter->setOrientation( Qt::Horizontal );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 4 );

        if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {
            layout->setMargin( 0 );
            layout->setSpacing( 0 );
            layout->addWidget( splitter );
        }
    }
}

void
u5303AMethodWidget::onInitialUpdate()
{
    if ( auto form = findChild< u5303AForm * >() ) {
        form->onInitialUpdate();
        connect( form, &u5303AForm::trigger_apply, this, &u5303AMethodWidget::handle_trigger_apply );
    }

    if ( auto table = findChild< u5303AMethodTable * >() ) {
        table->onInitialUpdate();
        table->setContents( u5303a::method() );
    }
}

void
u5303AMethodWidget::onStatus( int st )
{
    if ( auto form = findChild< u5303AForm * >() )
        form->onStatus( st );        
}

void
u5303AMethodWidget::handle_trigger_apply()
{
    document::instance()->prepare_for_run();
    /*
    if ( auto table = findChild< u5303AMethodTable * >() ) {
        u5303a::method m;
        if ( table->getContents( m ) ) {
            document::instance()->prepare_for_run( m );
        }
    }
    */
}

void
u5303AMethodWidget::OnCreate( const adportable::Configuration& )
{
}

void
u5303AMethodWidget::OnInitialUpdate()
{
    onInitialUpdate();
}

void
u5303AMethodWidget::OnFinalClose()
{
}

void
u5303AMethodWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

bool
u5303AMethodWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);        

        u5303a::method m;
        
        std::string device;
        adportable::serializer< u5303a::method >::serialize( m, device );

        adcontrols::controlmethod::MethodItem item;
        item.modelname( "u5303a" );
        item.isInitialCondition( true );
        item.itemLabel( "u5303a" );
        item.unitnumber( 1 );
        item.funcid( 1 );
        item.data( device.data(), device.size() );
        ptr->insert( item );
        return true;        
    }
    else if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_pointer( a ) ) {

        auto pi = boost::any_cast<adcontrols::controlmethod::MethodItem * >( a );                

        u5303a::method m;
        if ( auto table = findChild< u5303AMethodTable * >() ) {
            table->getContents( m );

            std::string device;
            adportable::serializer< u5303a::method >::serialize( m, device );
            
            pi->modelname( "u5303a" );
            pi->itemLabel( "u5303a" );
            pi->unitnumber( 1 );
            pi->funcid( 1 );
            pi->data( device.data(), device.size() );
            return true;
        }
    }
    return false;
}

bool
u5303AMethodWidget::setContents( boost::any& a )
{
    const adcontrols::controlmethod::MethodItem * pi(0);
    if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_pointer( a ) ) {
        pi = boost::any_cast<const adcontrols::controlmethod::MethodItem * >( a );             
    } else if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_a( a ) ) {   
        pi = &boost::any_cast<const adcontrols::controlmethod::MethodItem& >( a );
    }
    if ( pi ) {
        u5303a::method m;
        if ( adportable::serializer< u5303a::method >::deserialize( m, pi->data(), pi->size() ) ) {
            if ( auto table = findChild< u5303AMethodTable * >() ) {
                table->setContents( m );
                return true;
            }
        }
    }
    return false;
}
