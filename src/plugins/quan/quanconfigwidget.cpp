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

#include "quanconfigwidget.hpp"
#include "quanconfigform.hpp"
#include "quandocument.hpp"
#include "quanconstants.hpp"
#include "quanmethodcomplex.hpp"
#include "paneldata.hpp"
#include <utils/styledbar.h>
#include <adcontrols/quanmethod.hpp>
#include <adportable/profile.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <fstream>

using namespace quan;

QuanConfigWidget::~QuanConfigWidget()
{
}

QuanConfigWidget::QuanConfigWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QGridLayout )
                                                    , form_( new QuanConfigForm )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    connect( form_->spinLevels(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this] ( int value ){ emit onLevelChanged( value ); } );
    connect( form_->spinReplicates(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this] ( int value ){ emit onReplicatesChanged( value ); } );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        auto label = new QLabel;
        // label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Configuration" );
        toolBarLayout->addWidget( label );

        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
            btnOpen->setToolTip( tr( "Import configuration..." ) );
            toolBarLayout->addWidget( btnOpen );
            connect( btnOpen, &QToolButton::clicked, this, [this](bool){ importQuanMethod(); } );
        }

        if ( auto btnSave = new QToolButton ) {
            btnSave->setDefaultAction( Core::ActionManager::instance()->command( Constants::QUAN_METHOD_SAVE )->action() );
            btnSave->setToolTip( tr( "Save Quan Method..." ) );
            toolBarLayout->addWidget( btnSave );
        }
        
        auto edit = new QLineEdit;
        edit->setObjectName( Constants::editQuanMethodName );
        toolBarLayout->addWidget( edit );
        layout_->addWidget( toolBar );            
    }

    QuanDocument::instance()->register_dataChanged( [this]( int id, bool fnChanged ){ handleDataChanged( id, fnChanged ); });
    const int row = layout_->rowCount();
    layout_->addWidget( form_.get(), row, 0 );
    form_->setContents( QuanDocument::instance()->quanMethod() );

}

QWidget *
QuanConfigWidget::fileSelectionBar()
{
    if ( auto toolBar = new QWidget ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        auto label = new QLabel;
        label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Configuration" );
        toolBarLayout->addWidget( label );

        auto btnOpen = new QToolButton;
        btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
        btnOpen->setToolTip( tr("Open configuration...") );
        toolBarLayout->addWidget( btnOpen );

        auto btnSave = new QToolButton;
        btnSave->setIcon( QIcon( ":/quan/images/filesave.png" ) );
        btnSave->setToolTip( tr( "Save Quan Method..." ) );
        toolBarLayout->addWidget( btnSave );
        
        auto edit = new QLineEdit;
        edit->setObjectName( "editQuanMethodName" );
        toolBarLayout->addWidget( edit );

        connect( btnOpen, &QToolButton::clicked, this, [&] ( bool ){

            QString name = QFileDialog::getOpenFileName( this
                                                         , tr( "Import Quan Configuration..." )
                                                         , QuanDocument::instance()->lastMethodDir()
                                                         , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
            if ( !name.isEmpty() ) {
                    QuanMethodComplex m;
                    QuanDocument::instance()->load( name.toStdWString(), m );
                    QuanDocument::instance()->quanMethod( *m.quanMethod() );
                    QuanDocument::instance()->setMethodFilename( idQuanMethod, name.toStdWString() );
                    if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodName ) )
                        edit->setText( name );
                }
            } );
#if 0        
        connect( btnSave, &QToolButton::clicked, this, [&] ( bool ){
                QString name = QFileDialog::getSaveFileName( this
                                                             , tr( "Save Quantitative Analysis Configuration file" )
                                                             , QuanDocument::instance()->lastMethodDir()
                                                             , tr( "Quan Method Files(*.qmth);;XML File(*.xml)" ) );
                QuanDocument::instance()->save()
                if ( !name.isEmpty() ) {
                    QuanDocument::instance()->setMethodFilename( idQuanMethod, name.toStdWString() );
                    //QuanDocument::instance()->save( name.toStdWString(), QuanDocument::instance()->quanMethod() );
                    if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodFilename ) )
                        edit->setText( name );
                }
            } );
#endif        
        return toolBar;
    }
    return 0;
}

void
QuanConfigWidget::commit()
{
    adcontrols::QuanMethod m;
    form_->getContents( m );
    QuanDocument::instance()->quanMethod( m );
}

void
QuanConfigWidget::handleDataChanged( int id, bool )
{
#if 0
    if ( id == idQuanMethod ) {
        if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodName ) ) {
            edit->setText( QString::fromStdWString( QuanDocument::instance()->quanMethod().quanMethodFilename() ) );
            form_->setContents( QuanDocument::instance()->quanMethod() );
        }
    }
#endif
}

void
QuanConfigWidget::importQuanMethod()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Import Quan Configuration..." )
                                                 , QuanDocument::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        QuanMethodComplex m;
        QuanDocument::instance()->load( name.toStdWString(), m );
        QuanDocument::instance()->method( m.quanMethod() );
    }
}
