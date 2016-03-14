/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "mschromatogramwidget.hpp"
#include "mschromatogramform.hpp"
#include "moltable.hpp"
#include "targetingadducts.hpp"
#include <adportable/is_type.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/moltable.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <QMenu>

using namespace adwidgets;

MSChromatogramWidget::MSChromatogramWidget(QWidget *parent) : QWidget(parent)
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MSChromatogramForm ) ); 
            //splitter->addWidget( ( new TargetingTable ) );
            splitter->addWidget( ( new MolTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    if ( auto form = findChild< MSChromatogramForm * >() ) {
        //if ( auto table = findChild< TargetingTable *>() ) {
        if ( auto table = findChild< MolTable *>() ) {
            table->setColumnHidden( MolTable::c_abundance, true );
            connect( form, &MSChromatogramForm::onEnableLockMass, [table]( bool enable ) { table->setColumnHidden( MolTable::c_msref, !enable ); } );
        }

        connect( form, &MSChromatogramForm::triggerProcess, [this] { run(); } );
    }
}

MSChromatogramWidget::~MSChromatogramWidget()
{
}

QWidget *
MSChromatogramWidget::create( QWidget * parent )
{
    return new MSChromatogramWidget( parent );
}

void
MSChromatogramWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramWidget::OnInitialUpdate()
{
    if ( auto form = findChild< MSChromatogramForm * >() ) 
        form->OnInitialUpdate();

    if ( auto table = findChild< MolTable *>() ) {
        table->onInitialUpdate();
    }
}

void
MSChromatogramWidget::onUpdate( boost::any&& )
{
}

void
MSChromatogramWidget::OnFinalClose()
{
}

bool
MSChromatogramWidget::getContents( boost::any& a ) const
{
    if ( auto form = findChild< MSChromatogramForm * >() ) {
        
        if ( form->getContents( a ) ) {
            if ( auto pm = boost::any_cast<adcontrols::ProcessMethod *>( a ) ) {

                if ( auto cm = pm->find< adcontrols::MSChromatogramMethod >() ) {

                    if ( auto table = findChild< MolTable * >() ) {
                        table->getContents( cm->molecules() );
                        return true;
                    }
                }
            }
        }

    }
    return false;
}

bool
MSChromatogramWidget::setContents( boost::any&& a )
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->setContents( std::move(a) );

    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast<adcontrols::ProcessMethod&>( a );
        
        if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {
            if ( auto table = findChild< MolTable *>() ) {
                table->setContents( cm->molecules() );
                return true;
            }
        }
    }
    return false;
}

void
MSChromatogramWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Run generate chromatograms", this, SLOT( run() ) );
}

void
MSChromatogramWidget::run()
{
    emit triggerProcess( "MSChromatogramWidget" );
}

