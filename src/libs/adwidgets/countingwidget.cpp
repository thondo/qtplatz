/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "countingwidget.hpp"
#include "countingform.hpp"
#include "moltableview.hpp"
#include "targetingadducts.hpp"
#include <adportable/is_type.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/element.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/scanlaw.hpp>
#include <QBoxLayout>
#include <QMenu>
#include <QStandardItemModel>
#include <QSplitter>
#include <QGroupBox>
#include <ratio>

namespace adwidgets {

    using adcontrols::CountingMethod;
    
    struct CountingHelper {
        static bool readRow( int row, CountingMethod::value_type&, const QStandardItemModel& model );
        static bool setRow( int row, const CountingMethod::value_type&, QStandardItemModel& model );
        static bool setTime( int row, double mass, std::shared_ptr< const adcontrols::MassSpectrometer > sp, QStandardItemModel& model );
    };

    enum {
        c_formula
        , c_mass
        , c_time
        , c_width
        , c_protocol
    };

}

using namespace adwidgets;

CountingWidget::CountingWidget(QWidget *parent) : QWidget(parent)
{
    if ( QVBoxLayout * topLayout = new QVBoxLayout( this ) ) {

        topLayout->setMargin(5);
        topLayout->setSpacing(4);

        auto gbox = new QGroupBox;
        gbox->setTitle( tr( "Counting range list" ) );
        gbox->setCheckable( true );

        topLayout->addWidget( gbox );
        
        if ( QVBoxLayout * vboxLayout = new QVBoxLayout( gbox ) ) {
            vboxLayout->setMargin( 2 );
            vboxLayout->setSpacing( 2 );

            auto moltable = new MolTableView();
            setup( moltable );

            // QDoubleSpinBox -->
            connect( moltable
                     , static_cast< void(MolTableView::*)( const QModelIndex&, double )>( &MolTableView::valueChanged )
                     , this
                     , static_cast< void(CountingWidget::*)( const QModelIndex&, double )>( &CountingWidget::handleValueChanged ) );

            // ModelData -->
            connect( moltable
                     , static_cast< void(MolTableView::*)( const QModelIndex& )>( &MolTableView::valueChanged )
                     , this
                     , static_cast< void(CountingWidget::*)( const QModelIndex& )>( &CountingWidget::handleValueChanged ) );
            
            vboxLayout->addWidget( moltable );
        }
    }
}

CountingWidget::~CountingWidget()
{
}

void
CountingWidget::OnCreate( const adportable::Configuration& )
{
}

void
CountingWidget::OnInitialUpdate()
{
    if ( auto table = findChild< MolTableView *>() ) {
        table->onInitialUpdate();
        // table->setContents( adcontrols::CountingMethod() );
    }
}

void
CountingWidget::onUpdate( boost::any&& )
{
}

void
CountingWidget::OnFinalClose()
{
}

bool
CountingWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( a ) ) {

        if ( adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( a ) ) {

            adcontrols::CountingMethod m;
            getContents( m );
            pm->appendMethod( m );

            return true;
        }
    }
    return false;
}

bool
CountingWidget::setContents( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( a );
      
        if ( const adcontrols::CountingMethod * t = pm.find< adcontrols::CountingMethod >() ) {

            return setContents( *t );

        }
    }
    return false;
}

bool
CountingWidget::getContents( adcontrols::CountingMethod& t ) const
{
    t.clear();
    adcontrols::CountingMethod::value_type v;
    for ( int i = 0; i < model_->rowCount(); ++i ) {
        CountingHelper::readRow( i, v, *model_ );
        t << std::move( v );
    }

    return true;
}

bool
CountingWidget::setContents( adcontrols::CountingMethod&& t )
{
    model_->setRowCount( t.size() );

    int idx(0);

    for ( const auto& value: t ) 
        CountingHelper::setRow( idx++, value, *model_ );

    return true;    
}

void
CountingWidget::setup( MolTableView * table )
{
    model_ = std::make_unique< QStandardItemModel >();
    model_->setColumnCount( 5 );

    model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
    model_->setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "<i>m/z<i>" ) );
    model_->setHeaderData( c_time, Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
    model_->setHeaderData( c_width, Qt::Horizontal, QObject::tr( "Width(&mu;s)" ) );
    model_->setHeaderData( c_protocol, Qt::Horizontal, QObject::tr( "Protocol" ) );

    table->setModel( model_.get() );
    
    table->setColumnField( c_formula, ColumnState::f_formula, true, true ); // checkable
    table->setColumnField( c_mass, ColumnState::f_mass );
    table->setColumnField( c_time, ColumnState::f_time );
    table->setPrecision( c_time, 3 );
    table->setColumnField( c_width, ColumnState::f_time );
    table->setPrecision( c_width, 3 );
    table->setColumnField( c_protocol, ColumnState::f_protocol );

    table->setContextMenuHandler( [&]( const QPoint& pt ){
            QMenu menu;
            menu.addAction( tr( "Add row" ), this, SLOT( addRow() ) );
            if ( auto table = findChild< MolTableView * >() )
                menu.exec( table->mapToGlobal( pt ) );
        });
}

void
CountingWidget::addRow()
{
    int row = model_->rowCount();
    model_->setRowCount( row + 1 );
    adcontrols::CountingMethod::value_type v( std::make_tuple( true, std::string(), std::make_pair( 0.0, 0.1e-6 ), 0 ) );
    if ( row > 0 )
        CountingHelper::readRow( row - 1, v, *model_ );
    CountingHelper::setRow( row, v, *model_ );
}

void
CountingWidget::setSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    spectrometer_ = p;
}

void
CountingWidget::handleValueChanged( const QModelIndex& index, double data )
{
    if ( index.column() == c_time || index.column() == c_width )
        data /= std::micro::den; // second from microsecond

    emit valueChanged( index.row(), column_type( index.column() ), data );
}

void
CountingWidget::handleValueChanged( const QModelIndex& index )
{
    if ( index.column() == c_time || index.column() == c_width )
        handleValueChanged( index, index.data( Qt::EditRole ).toDouble() );

    QSignalBlocker block( model_.get() );
    
    if ( index.column() == c_formula ) {

        std::vector< adcontrols::mol::element > elements;
        int charge;
        if ( adcontrols::ChemicalFormula::getComposition( elements, index.data().toString().toStdString(), charge ) && charge == 0 ) {
            QString f = QString( "[%1]+" ).arg( index.data().toString() );
            model_->setData( index, f, Qt::EditRole );
        }

        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( model_->data( index, Qt::EditRole ).toString().toStdString() );
        model_->setData( model_->index( index.row(), c_mass ), mass, Qt::EditRole );

        CountingHelper::setTime( index.row(), mass, spectrometer_.lock(), *model_ );
    }

    if ( index.column() == c_mass || index.column() == c_protocol ) {
        CountingHelper::setTime( index.row(), index.data().toDouble(), spectrometer_.lock(), *model_ );        
    }

    emit valueChanged( index.row(), column_type( index.column() ), index.data( Qt::EditRole ) );
}

bool
CountingHelper::readRow( int row, adcontrols::CountingMethod::value_type& v, const QStandardItemModel& model )
{
    using adcontrols::CountingMethod;

    std::get< CountingMethod::CountingEnable >( v ) = model.index( row, 0 ).data( Qt::CheckStateRole ).toBool();
    std::get< CountingMethod::CountingFormula >( v ) = model.index( row, 0 ).data( Qt::EditRole ).toString().toStdString();
    std::get< CountingMethod::CountingRange >( v ).first = model.index( row, 2 ).data( Qt::EditRole ).toDouble() / std::micro::den;
    std::get< CountingMethod::CountingRange >( v ).second = model.index( row, 3 ).data( Qt::EditRole ).toDouble() / std::micro::den;
    std::get< CountingMethod::CountingProtocol >( v ) = model.index( row, 4 ).data( Qt::EditRole ).toInt();

    return true;
}

bool
CountingHelper::setRow( int row, const adcontrols::CountingMethod::value_type& v, QStandardItemModel& model )
{
    using adcontrols::CountingMethod;

    model.setData( model.index( row, 0 ), QString::fromStdString( std::get< CountingMethod::CountingFormula >( v ) ) );
    model.setData( model.index( row, 0 ), std::get< CountingMethod::CountingEnable >( v ) ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    if ( auto item = model.item( row, 0 ) )
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
     
    model.setData( model.index( row, 2 ), std::get< CountingMethod::CountingRange >( v ).first * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, 3 ), std::get< CountingMethod::CountingRange >( v ).second * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, 4 ), std::get< CountingMethod::CountingProtocol >( v ), Qt::EditRole );

    return true;
}

bool
CountingHelper::setTime( int row, double mass, std::shared_ptr< const adcontrols::MassSpectrometer > sp, QStandardItemModel& model )
{
    if ( sp ) {
        if ( auto scanlaw = sp->scanLaw() ) {
            double tof = scanlaw->getTime( mass, model.data( model.index( row, c_protocol ) ).toInt() );
            model.setData( model.index( row, c_time ), tof * std::micro::den, Qt::EditRole );
        }
    }
}
    