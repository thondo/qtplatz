/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "processmethodwidget.hpp"
#include "quandocument.hpp"
#include <adcontrols/processmethod.hpp>
#include <adwidgets/centroidform.hpp>

#include <QVBoxLayout>
#include <QGridLayout>
#include <boost/any.hpp>

using namespace quan;

ProcessMethodWidget::~ProcessMethodWidget()
{
}

ProcessMethodWidget::ProcessMethodWidget(QWidget *parent) :  QWidget(parent)
                                                          , layout_( new QGridLayout )
                                                          , form_( new adwidgets::CentroidForm )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    layout_->addWidget( form_ ); 
    form_->OnInitialUpdate();

    connect( form_, &adwidgets::CentroidForm::valueChanged, this, &ProcessMethodWidget::commit );

    QuanDocument::instance()->register_dataChanged( [this]( int id, bool load ){ handleDataChanged( id, load ); });
    
}


void
ProcessMethodWidget::handleDataChanged( int id, bool load )
{
    if ( id == idProcMethod && load ) {
        boost::any a( QuanDocument::instance()->procMethod() );
        form_->setContents( a );
    }
}

void
ProcessMethodWidget::commit()
{
    adcontrols::ProcessMethod pm;
    form_->getContents( pm );
    QuanDocument::instance()->procMethod( pm );
}