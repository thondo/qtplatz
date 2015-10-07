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

#include "u5303aform.hpp"
#include "ui_u5303aform.h"
#include <acqrscontrols/u5303a/method.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <QSignalBlocker>

using namespace acqrswidgets;

u5303AForm::u5303AForm( QWidget *parent ) : QWidget( parent )
                                          , ui( new Ui::u5303AForm )
                                          , sampRate_( 3.2e9 )
{
    ui->setupUi( this );

    ui->spinBox->setStepBy( []( adwidgets::SpinBox * _this, int step ) {
        int index = _this->value() >= 8 ? ( _this->value() ) / 8 + 7 : _this->value();
        index += step;
        int value = index <= 8 ? index : ( index - 7 ) * 8;
        _this->setValue( value );
    });

    // start delay
    connect( ui->doubleSpinBox_1, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double d ) {
            emit valueChanged( idU5303AStartDelay, 0, QVariant( adcontrols::metric::scale_to_base( d, adcontrols::metric::micro ) ) ); } );

    // width
    connect( ui->doubleSpinBox_2, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double d ) {
            emit valueChanged( idU5303AWidth, 0, QVariant( adcontrols::metric::scale_to_base( d, adcontrols::metric::micro ) ) ); } );

    // number of average
    connect( ui->spinBox, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int d ) {
        if ( d > 8 && ( d % 8 ) ) {
            QSignalBlocker block( ui->spinBox );
            ui->spinBox->setValue( d & ~07 );
        }
        emit valueChanged( idNbrAverages, 0, QVariant( d ) ); } );

    // mode
    connect( ui->checkBox_Avg, &QCheckBox::toggled, [this] ( bool flag ) { emit valueChanged( idU5303AMode, 0, QVariant( flag ) ); } );
}

u5303AForm::~u5303AForm()
{
    delete ui;
}

void
u5303AForm::onInitialUpdate()
{
}

void
u5303AForm::setContents( const acqrscontrols::u5303a::method& m )
{
    QSignalBlocker blocks[] = {
        QSignalBlocker( ui->doubleSpinBox_1 ), QSignalBlocker( ui->doubleSpinBox_2 )
        , QSignalBlocker( ui->spinBox ), QSignalBlocker( ui->checkBox_Avg ) };

    ui->doubleSpinBox_1->setValue( adcontrols::metric::scale_to_micro( m.method_.delay_to_first_sample_ ) );

    sampRate_ = m.method_.samp_rate;
    double width = m.method_.nbr_of_s_to_acquire_ / sampRate_;
    ui->doubleSpinBox_2->setValue( adcontrols::metric::scale_to_micro( width ) );

    ui->spinBox->setValue( m.method_.nbr_of_averages );

    ui->checkBox_Avg->setChecked( !( m.mode_ == 0 ) );
}

void
u5303AForm::getContents( acqrscontrols::u5303a::method& m )
{
    m.method_.delay_to_first_sample_ = adcontrols::metric::scale_to_base( ui->doubleSpinBox_1->value(), adcontrols::metric::micro );

    double width = adcontrols::metric::scale_to_base( ui->doubleSpinBox_2->value(), adcontrols::metric::micro );
    m.method_.nbr_of_s_to_acquire_ = uint32_t( width * m.method_.samp_rate + 0.5 );

    m.method_.nbr_of_averages = ui->spinBox->value();

    m.mode_ = ui->checkBox_Avg->isChecked() ? 2 : 0;
}


void
u5303AForm::onHandleValue( idCategory id, int channel, const QVariant& value )
{
    switch ( id ) {
    case idU5303AStartDelay:
        do {
            QSignalBlocker block( ui->doubleSpinBox_1 );
            ui->doubleSpinBox_1->setValue( adcontrols::metric::scale_to_micro( value.toDouble() ) );
        } while ( 0 );
        break;
        
    case idU5303ANbrSamples:
        do {
            QSignalBlocker block( ui->doubleSpinBox_2 );
            double width = value.toInt() / sampRate_;
            ui->doubleSpinBox_2->setValue( adcontrols::metric::scale_to_micro( width ) );
        } while ( 0 );
        break;

    case idU5303ASampRate:
        sampRate_ = value.toDouble();
        break;

    case idNbrAverages:
        do {
            QSignalBlocker block( ui->spinBox );
            ui->spinBox->setValue( value.toInt() );
        } while ( 0 );
        break;
    }
}