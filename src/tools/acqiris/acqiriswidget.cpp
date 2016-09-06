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

#include "acqiriswidget.hpp"
#include "acqiris_method.hpp"
#include "document.hpp"
#include <QBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QHeaderView>
#include <QMetaType>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QTreeView>
#include <iostream>
#include <ratio>

enum { r_trig, r_hor, r_ver };
// enum { r_trig_class, r_trig_source, r_trig_coupling, r_trig_level1, r_trig_level2 };
// enum { r_hor_delay, r_hor_width, r_hor_rate, r_hor_mode };
// enum { r_ver_fs, r_ver_offset, r_ver_coupling, r_ver_bandwidth };

namespace acqiriswidget {

    class column_type {
    public:
        enum data_type { numeric_type, enum_type };
        data_type type_;
        int precision_;
        size_t den_;
        std::vector< std::pair< QString, uint32_t > > choice_;

        column_type( data_type t = enum_type ) : type_( t ), precision_( -1 ), den_( 1 )
            {}

        column_type( int precision, size_t den = 1, data_type t = numeric_type ) : type_( numeric_type )
                                                                                 , den_( den )
                                                                                 , precision_( precision )
            {}

        column_type& operator << ( std::pair< QString, uint32_t >&& t ) {
            choice_.emplace_back( t );
            return *this;
        }

        QVariant operator()() const {
            return QVariant::fromValue( *this );
        }

        QVariant variant() const {
            return QVariant::fromValue( *this );
        }
    };
}

Q_DECLARE_METATYPE( acqiriswidget::column_type );

using namespace acqiriswidget;

class AcqirisWidget::delegate : public QStyledItemDelegate {

    AcqirisWidget * this_;

public:
    delegate( AcqirisWidget * owner ) : this_( owner )
        {}

    void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            
        QStyleOptionViewItem opt(option);
        initStyleOption( &opt, index );

        auto v = index.data( Qt::UserRole + 1 );
        
        if ( index.column() == 1 && !v.isNull() ) {
            auto state = v.value< column_type >();

            if ( state.type_ == column_type::enum_type ) {

                auto value = state.den_ == 1 ? index.data( Qt::EditRole ).toUInt()
                    : uint32_t ( index.data( Qt::EditRole ).toDouble() * state.den_ );

                auto it = std::find_if( state.choice_.begin(), state.choice_.end()
                                        , [&]( const std::pair< QString, uint32_t >& t ){ return t.second == value; } );

                if ( it != state.choice_.end() )
                    painter->drawText( opt.rect, opt.displayAlignment, it->first );

            } else if ( state.type_ == column_type::numeric_type ) {

                if ( index.data( Qt::EditRole ).canConvert< double >() ) {
                    painter->drawText( option.rect
                                       , option.displayAlignment
                                       , QString::number( index.data( Qt::EditRole ).toDouble() * state.den_, 'f', state.precision_ ) );
                }

            }
        } else {
            QStyledItemDelegate::paint( painter, opt, index );
        }
    }
    
    QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {

        if ( index.column() == 1 && !index.data( Qt::UserRole + 1 ).isNull() ) {

            auto state = index.data( Qt::UserRole + 1 ).value< column_type >();

            if ( state.type_ == column_type::enum_type ) {

                auto combo = new QComboBox( parent );
                for ( auto& x : state.choice_ )
                    combo->addItem( x.first );
                return combo;
                
            } else if ( state.type_ == column_type::numeric_type ) {

                auto spin = new QDoubleSpinBox( parent );
                spin->setDecimals( state.precision_ );
                spin->setMaximum( 1000.0 );
                spin->setMinimum( -1000.0 );
                spin->setSingleStep( std::pow( 10, -state.precision_ ) );

                return spin;
            }

        } else {
            return QStyledItemDelegate::createEditor( parent, option, index );
        }
    }

    void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {

        bool handled( false );
        
        if ( index.column() == 1 && !index.data( Qt::UserRole + 1 ).isNull() ) {
            
            auto state = index.data( Qt::UserRole + 1 ).value< column_type >();
            if ( state.type_ == column_type::enum_type ) {
                if ( auto combo = qobject_cast<QComboBox *>( editor ) ) {
                    int idx = combo->currentIndex();
                    if ( idx < state.choice_.size() ) {
                        if ( state.den_ == 1 )
                            model->setData( index, state.choice_[ idx ].second, Qt::EditRole );
                        else
                            model->setData( index, double( state.choice_[ idx ].second ) / state.den_, Qt::EditRole );
                        handled = true;
                    }
                }
            } else if ( state.type_ == column_type::numeric_type ) {
                if ( auto spin = qobject_cast< QDoubleSpinBox * >( editor ) ) {
                    model->setData( index, spin->value() / state.den_, Qt::EditRole );
                }
            }
        }

        if ( !handled )
            QStyledItemDelegate::setModelData( editor, model, index );
    }
    
};

AcqirisWidget::AcqirisWidget( QWidget * parent ) : QWidget( parent )
                                                 , model_( new QStandardItemModel() )
                                                 , tree_( new QTreeView() )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        layout->addWidget( tree_ );
        tree_->setModel( model_.get() );
        tree_->setItemDelegate( new delegate( this ) );
        initialUpdate( *model_ );
    }

    connect( model_.get(), &QStandardItemModel::itemChanged, this, [this]( const QStandardItem * item ){
            static aqdrv4::SubMethodType types [] = { aqdrv4::triggerMethod, aqdrv4::horizontalMethod, aqdrv4::ch1VerticalMethod, aqdrv4::extVerticalMethod };
            if ( auto parent = item->parent() ) {
                int row = parent->index().row();
                if ( row < sizeof( types )/sizeof( types[0] ))
                    emit dataChanged( this, int( types[ row ] ) );
            }
        });
}

AcqirisWidget::~AcqirisWidget()
{
}

void
AcqirisWidget::initialUpdate( QStandardItemModel& model )
{
    QSignalBlocker block( &model );
    model.setColumnCount( 2 );
    model.setRowCount( 4 );

    model.setData( model.index( 0, 0 ), "Trigger Config", Qt::EditRole );
    model.setData( model.index( 1, 0 ), "Horizontal", Qt::EditRole );
    model.setData( model.index( 2, 0 ), "Channel 1", Qt::EditRole );
    model.setData( model.index( 3, 0 ), "Ext. Trig", Qt::EditRole );

    auto m = std::make_shared< aqdrv4::acqiris_method >( *document::instance()->acqiris_method() );

    // trigger
    if ( auto parent = model.item( r_trig, 0 ) ) {
        for ( auto label: { "Class", "Source", "Coupling", "Slope", "Level1(mV)", "Level2" } )
            parent->appendRow( QList< QStandardItem * >() << new QStandardItem( label ) << new QStandardItem() );

        if ( auto trig = m->mutable_trig() ) {
            int row(0);
            {
                column_type state;
                state << std::make_pair( "Edge", 0 ) << std::make_pair( "Level", 1 );
                model.setData( model.index( row, 1, parent->index() ), state.variant(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), trig->trigClass, Qt::EditRole );
            }
            {
                column_type state;
                state << std::make_pair( "Ext. trig", 0x80000000 )
                      << std::make_pair( "channel 1", 1 )
                      << std::make_pair( "channel 2", 2 );
                model.setData( model.index( row, 1, parent->index() ), state.variant(), Qt::UserRole + 1 ); // source
                model.setData( model.index( row++, 1, parent->index() ), trig->trigPattern, Qt::EditRole ); // source
            }
            {
                column_type state;
                state << std::make_pair( "DC", 0 ) << std::make_pair( "AC", 1 )
                      << std::make_pair( "HF Reject", 2 ) << std::make_pair( "DC, 50W", 3 ) << std::make_pair( "AC, 50W", 4 );
                model.setData( model.index( row, 1, parent->index() ), state.variant(), Qt::UserRole + 1 );                
                model.setData( model.index( row++, 1, parent->index() ), trig->trigCoupling, Qt::EditRole );
            }
            {
                column_type state;
                state << std::make_pair( "Positive", 0 ) << std::make_pair( "Negative", 1 );
                model.setData( model.index( row, 1, parent->index() ), state.variant(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), trig->trigSlope, Qt::EditRole );
            }
            {
                // trig level1
                model.setData( model.index( row, 1, parent->index() ), column_type( 0 )(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), trig->trigLevel1, Qt::EditRole );
            }
            {
                // trig level2
                model.setData( model.index( row, 1, parent->index() ), column_type( 0 )(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), trig->trigLevel2, Qt::EditRole );
            }
        }
    }

    if ( auto parent = model.item( r_hor, 0 ) ) {

        for ( auto label: { "Delay", "Width", "Rate" } )
            parent->appendRow( QList< QStandardItem * >() << new QStandardItem( label ) << new QStandardItem() );

        if ( auto hor = m->mutable_hor() ) {
            int row(0);
            {
                // delay
                double delay = hor->delayTime;
                model.setData( model.index( row, 1, parent->index() ), column_type( 3 )(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), delay * std::micro::den, Qt::EditRole );
            }            
            {
                // width
                double width = hor->nbrSamples * hor->sampInterval;
                model.setData( model.index( row, 1, parent->index() ), column_type( 3 )(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), width * std::micro::den, Qt::EditRole );
            }
            {
                // rate
                column_type t;
                t << std::make_pair( "4.0GS/s", 250 )   // 0.25e-9
                  << std::make_pair( "2.0GS/s", 500 )   // 0.50e-9
                  << std::make_pair( "1.0GS/s", 1000 )  // 1.00e-9
                  << std::make_pair( "500MS/s", 2000 ); // 2.00e-9
                t.den_ = std::pico::den; //1000000000000LL;
                model.setData( model.index( row, 1, parent->index() ), t.variant(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), hor->sampInterval, Qt::EditRole );
            }
        }
    }

    // vertical (Channel 1, Ext. Trig)
    int nid(0);
    for ( auto ver : { m->mutable_ch1(), m->mutable_ext() } ) {

        if ( auto parent = model.item( r_ver + nid++, 0 ) ) {

            for ( auto label: { "Full scale", "Offset", "Coupling", "Band width" } )
                parent->appendRow( QList< QStandardItem * >() << new QStandardItem( label ) << new QStandardItem() );
            
            int row(0);
            {
                column_type t;
                t << std::make_pair( "5.0V", 5000 )
                  << std::make_pair( "2.0V", 2000 )
                  << std::make_pair( "1.0V", 1000 )
                  << std::make_pair( "0.5V", 500 )
                  << std::make_pair( "0.2V", 200 )
                  << std::make_pair( "0.1V", 100 )
                  << std::make_pair( "0.05V", 50 );
                t.den_ = 1000; // mV -> V
                model.setData( model.index( row, 1, parent->index() ), t.variant(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), ver->fullScale, Qt::EditRole );
            }
            {
                // offset
                model.setData( model.index( row, 1, parent->index() ), column_type( 2 )(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), ver->offset, Qt::EditRole );
            }
            {   // coupling
                column_type t;
                t << std::make_pair( "DC, 1Mohm", 1 )
                  << std::make_pair( "AC, 1Mohm", 2 )
                  << std::make_pair( "DC, 50ohm", 3 )
                  << std::make_pair( "AC, 50ohm", 4 );
                model.setData( model.index( row, 1, parent->index() ), t.variant(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), ver->coupling, Qt::EditRole );
            }
            {   // bandwidth
                column_type t;
                t << std::make_pair( "no limit", 0 )
                  << std::make_pair( " 25MHz", 1 )
                  << std::make_pair( "700MHz", 2 )
                  << std::make_pair( "200MHZ", 3 )
                  << std::make_pair( " 20MHz", 4 )
                  << std::make_pair( " 35MHz", 5 );
                model.setData( model.index( row, 1, parent->index() ), t.variant(), Qt::UserRole + 1 );
                model.setData( model.index( row++, 1, parent->index() ), ver->bandwidth, Qt::EditRole );
            }
        }
    }

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( auto parent = model.item( row, 0 ) ) {
            parent->setEditable( false );
            for ( int childRow = 0; childRow < parent->rowCount(); ++childRow ) {
                if ( auto child = parent->child( childRow, 0 ) )
                    child->setEditable( false );
            }
        }
    }
    
    setContents( m );
    
    tree_->expandAll();    
    tree_->resizeColumnToContents( 0 );
    tree_->resizeColumnToContents( 1 );
    tree_->header()->hide();
}

void
AcqirisWidget::setContents( std::shared_ptr< const aqdrv4::acqiris_method > m )
{
    QSignalBlocker block( model_.get() );
    
    // trigger
    if ( auto parent = model_->item( r_trig, 0 ) ) {

        if ( auto trig = m->trig() ) {
            int row(0);
            model_->setData( model_->index( row++, 1, parent->index() ), trig->trigClass, Qt::EditRole );
            model_->setData( model_->index( row++, 1, parent->index() ), trig->trigPattern, Qt::EditRole ); // source
            model_->setData( model_->index( row++, 1, parent->index() ), trig->trigCoupling, Qt::EditRole );
            model_->setData( model_->index( row++, 1, parent->index() ), trig->trigSlope, Qt::EditRole );
            model_->setData( model_->index( row++, 1, parent->index() ), trig->trigLevel1, Qt::EditRole );
            model_->setData( model_->index( row++, 1, parent->index() ), trig->trigLevel2, Qt::EditRole );
        }
    }

    // horizontal
    if ( auto parent = model_->item( r_hor, 0 ) ) {

        if ( auto hor = m->hor() ) {
            int row(0);
            model_->setData( model_->index( row++, 1, parent->index() ), hor->delayTime * std::micro::den, Qt::EditRole );
            model_->setData( model_->index( row++, 1, parent->index() ), ( hor->sampInterval * hor->nbrSamples ) * std::micro::den, Qt::EditRole );
            model_->setData( model_->index( row++, 1, parent->index() ), hor->sampInterval, Qt::EditRole );
        }
        
    }

    // vertical (Channel 1, Ext. Trig)
    int nid(0);
    for ( auto ver : { m->ch1(), m->ext() } ) {
        if ( ver ) {
            if ( auto parent = model_->item( r_ver + nid++, 0 ) ) {

                int row(0);
                model_->setData( model_->index( row++, 1, parent->index() ), ver->fullScale, Qt::EditRole );
                model_->setData( model_->index( row++, 1, parent->index() ), ver->offset, Qt::EditRole );
                model_->setData( model_->index( row++, 1, parent->index() ), ver->coupling, Qt::EditRole );
                model_->setData( model_->index( row++, 1, parent->index() ), ver->bandwidth, Qt::EditRole );
            }
        }
    }
}

void
AcqirisWidget::getContents( std::shared_ptr< aqdrv4::acqiris_method > m ) const
{
    // trigger
    if ( auto parent = model_->item( r_trig, 0 ) ) {

        if ( auto trig = m->mutable_trig() ) {
            int row(0);
            trig->trigClass = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toInt();
            trig->trigPattern = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toUInt();
            trig->trigCoupling = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toUInt();
            trig->trigSlope = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toUInt();
            trig->trigLevel1 = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble();
            trig->trigLevel2 = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble();
        }
    }

    // horizontal
    if ( auto parent = model_->item( r_hor, 0 ) ) {

        if ( auto hor = m->mutable_hor() ) {
            int row(0);
            hor->delayTime = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble() / std::micro::den;
            double width = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble() / std::micro::den;
            hor->sampInterval = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble();
            hor->nbrSamples = width / hor->sampInterval + 0.5;
        }
        
    }

    // vertical (Channel 1, Ext. Trig)
    int nid(0);
    for ( auto ver : { m->mutable_ch1(), m->mutable_ext() } ) {
        if ( auto parent = model_->item( r_ver + nid++, 0 ) ) {

            int row(0);
            ver->fullScale = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble();
            ver->offset = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toDouble();
            ver->coupling = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toUInt();
            ver->bandwidth = model_->index( row++, 1, parent->index() ).data( Qt::EditRole ).toUInt();

        }
    }
}
