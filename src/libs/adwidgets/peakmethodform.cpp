/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "peakmethodform.hpp"
#include "ui_peakmethodform.h"

#if !defined Q_MOC_RUN
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/configuration.hpp>
#include <adlog/logger.hpp>
#include <adportable/is_type.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#endif

#include <qtwrapper/qstring.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QTextDocument>
#include <QTreeView>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QDebug>

namespace adwidgets {

    enum { c_time, c_function, c_event_value };

    namespace PeakMethodFormPrivate {

        class TimeEventsDelegate : public QStyledItemDelegate {
            Q_OBJECT
        public:
            explicit TimeEventsDelegate( PeakMethodForm *, QObject *parent = 0 );

            QWidget * createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
            void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
            void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
        private:
            PeakMethodForm * form_;

        signals:

            public slots :

        };

        ////////////
        enum { c_header, c_value, c_num_columns };

        enum {
            r_slope
            , r_min_width
            , r_min_height
            , r_drift
            , r_min_area
            , r_doubling_time
            , r_void_time
            , r_pharmacopoeia
            , r_num_rows
        };

        class PeakMethodDelegate : public QStyledItemDelegate {
            Q_OBJECT
        public:
            explicit PeakMethodDelegate( PeakMethodForm *, QObject *parent = 0 );

            QWidget * createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
            void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
            void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
            void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
            QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const override;
            bool editorEvent( QEvent * event, QAbstractItemModel *
                              , const QStyleOptionViewItem&, const QModelIndex& ) override;
        private:
            PeakMethodForm * form_;

        signals:

            public slots :

        };
    }
}

using namespace adwidgets;
using namespace adwidgets::PeakMethodFormPrivate;

PeakMethodForm::PeakMethodForm(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::PeakMethodForm)
                                                , pMethod_( new adcontrols::PeakMethod ) 
                                                , pTimeEventsModel_( new QStandardItemModel )
                                                , pGlobalModel_( new QStandardItemModel )
{
    ui->setupUi(this);

    auto layout = new QVBoxLayout( ui->groupBox );
    layout->setContentsMargins( 2, 2, 2, 2 );
    layout->setSpacing( 0 );

    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins( 2, 2, 2, 2 );
    hLayout->setSpacing( 0 );
        
    if ( auto tree = new QTreeView() ) {
            
        tree->setModel( pGlobalModel_.get() );
        tree->setItemDelegate( new PeakMethodDelegate(this) );
        hLayout->addWidget( tree );

        tree->setStyleSheet( "QTreeView::item::selected{ background-color: #1d3dec; color: white; }" );
            
    }
        
    if ( auto table = new QTableView () ) {
        table->setModel( pTimeEventsModel_.get() );
        table->setItemDelegate( new TimeEventsDelegate(this) );
        table->verticalHeader()->setDefaultSectionSize( 18 );
        hLayout->addWidget( table );
    }
        
    layout->addLayout( hLayout );
        
}

PeakMethodForm::~PeakMethodForm()
{
    delete ui;
}

void
PeakMethodForm::OnCreate( const adportable::Configuration& config )
{
    (void)config;
}

void
PeakMethodForm::OnInitialUpdate()
{
    setContents( *pMethod_ );

    if ( auto table = findChild< QTableView * >() ) {

        QStandardItemModel& model = *pTimeEventsModel_;
        QStandardItem * rootNode = model.invisibleRootItem();
        rootNode->setColumnCount(3);
        model.setHeaderData( c_time, Qt::Horizontal, "Time(min)" );
        model.setHeaderData( c_function, Qt::Horizontal, "Func" );
        model.setHeaderData( c_event_value, Qt::Horizontal, "Value" );
        table->setSortingEnabled( true );

    }

    if ( auto tree = findChild< QTreeView * >() ) {

        QStandardItemModel& model = *pGlobalModel_;

        model.setColumnCount( c_num_columns );
        model.setHeaderData( c_header, Qt::Horizontal, "Function" );
        model.setHeaderData( c_value, Qt::Horizontal, "Value" );

        model.setRowCount( r_num_rows );
        model.setData( model.index( r_slope,         c_header ), tr( "Slope [&mu;V/min]" ) );
        model.setData( model.index( r_min_width,     c_header ), tr( "Minimum width[min]" ) );
        model.setData( model.index( r_min_height,    c_header ), tr( "Minimum height" ) );
        model.setData( model.index( r_drift,         c_header ), tr( "Drift [height/min]" ) );
        model.setData( model.index( r_min_area,      c_header ), tr( "Minumum area[&mu;V&times;s]" ) );
        model.setData( model.index( r_doubling_time, c_header ), tr( "Peak width doubling time[min]" ) );
        model.setData( model.index( r_void_time,     c_header ), tr( "<em>T<sub>0</sub></em>" ) );
        model.setData( model.index( r_pharmacopoeia, c_header ), tr( "Pharmacopoeia" ) );

        setContents( *pMethod_ );

        for ( int row = 0; row < r_num_rows; ++row ) {
            model.item( row, c_header )->setEditable( false );
            model.item( row, c_value )->setEditable( true );
        }

        tree->resizeColumnToContents( 0 );
        tree->resizeColumnToContents( 1 );
		tree->setExpandsOnDoubleClick( false );
		tree->setEditTriggers( QAbstractItemView::AllEditTriggers );
		tree->setTabKeyNavigation( true );
    }

}

void
PeakMethodForm::OnFinalClose()
{
}

bool
PeakMethodForm::getContents( boost::any& any ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {

        adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
        getContents( *pm );

        return true;
    }

    return false;
}

bool
PeakMethodForm::setContents( boost::any& any )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {

        adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
        if ( const adcontrols::PeakMethod *p = pm.find< adcontrols::PeakMethod >() ) {
            *pMethod_ = *p;
            setContents( *p );
            return true;
        }
    }
    return false;
}

void
PeakMethodForm::getContents( adcontrols::ProcessMethod& pm ) const
{
    getContents( *pMethod_ );
	pm.appendMethod< adcontrols::PeakMethod >( *pMethod_ );
}

void
PeakMethodForm::setContents( const adcontrols::PeakMethod& method )
{
    do {
        QStandardItemModel& model = *pGlobalModel_;
        
        model.setData( model.index( r_slope,         c_value ), method.slope() );
        model.setData( model.index( r_min_width,     c_value ), method.minimumWidth() ); 
        model.setData( model.index( r_min_height,    c_value ), method.minimumHeight() );
        model.setData( model.index( r_drift,         c_value ), method.drift() );
        model.setData( model.index( r_min_area,      c_value ), method.minimumArea() );
        model.setData( model.index( r_doubling_time, c_value ), method.doubleWidthTime() );
        model.setData( model.index( r_void_time,     c_value ), method.t0() );
        model.setData( model.index( r_pharmacopoeia, c_value ), method.pharmacopoeia() );
    } while(0);

    do {
        QStandardItemModel& model = *pTimeEventsModel_;
		
		model.setRowCount( static_cast< int >( method.size() + 1 ) ); // add one blank line at end
		int row = 0;
		for ( const auto item: *pMethod_ ) {
			model.setData( model.index( row, c_time ), item.time() );
			model.setData( model.index( row, c_function ), item.peakEvent() );
			if ( item.isBool() )
				model.setData( model.index( row, c_event_value ), item.boolValue() );
			else
				model.setData( model.index( row, c_event_value ), item.doubleValue() );
			++row;
		}
        
    } while(0);
}

void
PeakMethodForm::getContents( adcontrols::PeakMethod& method ) const
{
    do {
        QStandardItemModel& model = *pGlobalModel_;
    
        method.slope( model.index( r_slope, c_value ).data( Qt::EditRole ).toDouble() );
        method.minimumWidth( model.index( r_min_width, c_value ).data( Qt::EditRole ).toDouble() );
        method.minimumHeight( model.index( r_min_height, c_value ).data( Qt::EditRole ).toDouble() );
        method.drift( model.index( r_drift, c_value ).data( Qt::EditRole ).toDouble() );
        method.minimumArea( model.index( r_min_area, c_value ).data( Qt::EditRole ).toDouble() );
        method.doubleWidthTime(model.index( r_doubling_time, c_value ).data( Qt::EditRole ).toDouble() );
        method.t0( model.index( r_void_time, c_value ).data( Qt::EditRole ).toDouble() );
        int value = model.index( r_pharmacopoeia, c_value ).data( Qt::EditRole ).toInt();
        method.pharmacopoeia( static_cast< adcontrols::chromatography::ePharmacopoeia >( value ) );
        
        using adcontrols::chromatography::ePHARMACOPOEIA_USP;
        using adcontrols::chromatography::ePHARMACOPOEIA_EP;
        using adcontrols::chromatography::ePHARMACOPOEIA_JP;
        using adcontrols::chromatography::ePeakWidth_Tangent;
        using adcontrols::chromatography::ePeakWidth_HalfHeight;

        if ( method.pharmacopoeia() == ePHARMACOPOEIA_USP ) {
            method.theoreticalPlateMethod( ePeakWidth_Tangent );
        } else if ( method.pharmacopoeia() == ePHARMACOPOEIA_EP ) {
            method.theoreticalPlateMethod( ePeakWidth_HalfHeight );
            method.peakWidthMethod( ePeakWidth_HalfHeight );
        } else if ( method.pharmacopoeia() == ePHARMACOPOEIA_JP ) {
            method.theoreticalPlateMethod( ePeakWidth_HalfHeight );
            method.peakWidthMethod( ePeakWidth_HalfHeight );
        }
    } while(0);
    do {
        QStandardItemModel& model = *pTimeEventsModel_;

        method.erase( method.begin(), method.end() );
        for ( int row = 0; row < model.rowCount(); ++row ) {
			double minutes = model.data( model.index( row, c_time ) ).toDouble();

			adcontrols::chromatography::ePeakEvent func 
				= static_cast< adcontrols::chromatography::ePeakEvent >( model.data( model.index( row, c_function ) ).toInt() + 1 );
            const QVariant value = model.data( model.index( row, c_event_value ) );
            adcontrols::PeakMethod::TimedEvent e( adcontrols::timeutil::toSeconds( minutes ), func );
            if ( e.isBool() )
                e.setValue( value.toBool() );
            else
                e.setValue( value.toDouble() );
            method << e;
        }
    } while(0);
}

// #include "timeeventsdelegate.hpp"
// #include <adcontrols/peakmethod.hpp>
// #include <boost/format.hpp>
// #include <QComboBox>

namespace adwidgets { namespace internal {

        static const char * functions [] = {
            "Lock"
            , "Forced base"
            , "Shift base"
            , "V-to-V"
            , "Tailing"
            , "Leading"
            , "Shoulder"
            , "Negative peak"
            , "Negative lock"
            , "Horizontal base"
            , "Post horizontal base"
            , "Forced peak"
            , "Slope"
            , "Minimum width"
            , "Minimum height"
            , "Minimum area"
            , "Drift"
            , "Elmination"
            , "Manual"
        };
    }
}

#define countof(x) ( sizeof(x)/sizeof(x[0]) )

TimeEventsDelegate::TimeEventsDelegate(PeakMethodForm * form, QObject *parent) : QStyledItemDelegate(parent), form_( form )
{
}

QWidget *
TimeEventsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        for ( size_t i = 0; i < countof( internal::functions ); ++i )
            list << internal::functions[ i ];
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QStyledItemDelegate::createEditor( parent, option, index );
  }
}

void
TimeEventsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        int idx = index.data().toInt() - 1;
        if ( size_t( idx ) < countof( internal::functions ) )
            painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, internal::functions[ idx ] );
        else
            painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, index.data().toString() );
    } else if ( index.column() == c_time ) {
        double value = index.data().toDouble();
        painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, QString::number( value, 'f', 4 ) );
    } else if ( index.column() == c_event_value ) {
        double value = index.data().toDouble();
        painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, QString::number( value, 'f', 3 ) );
    } else {
        QStyledItemDelegate::paint( painter, option, index );
    }
}

void
TimeEventsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        int value = p->currentIndex();
        adcontrols::chromatography::ePeakEvent func = static_cast< adcontrols::chromatography::ePeakEvent >( value + 1 );
        model->setData( index, func );
        adcontrols::PeakMethod::TimedEvent event( 0, func );
        if ( event.isBool() ) {
			if ( model->index( index.row(), c_event_value ).data( Qt::EditRole ).type() != QVariant::Bool )
                model->setData( model->index( index.row(), c_event_value ), event.boolValue() );
        } else if ( event.isDouble() ) {
            if ( model->index( index.row(), c_event_value ).data( Qt::EditRole ).type() != QVariant::Double )
                model->setData( model->index( index.row(), c_event_value ), event.doubleValue() );
        }
    } else
        QStyledItemDelegate::setModelData( editor, model, index );

    if ( index.row() == model->rowCount() - 1 )
        model->insertRow( index.row() + 1 ); // add last blank line

    emit form_->valueChanged();
}

////////////////////////////////////

PeakMethodDelegate::PeakMethodDelegate(PeakMethodForm * form, QObject *parent) : QStyledItemDelegate(parent), form_( form )
{
}

QWidget *
PeakMethodDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    static QStringList list = { tr( "Not specified" ), tr( "EP" ), tr( "JP" ), tr( "USP" ) };

    if ( index.row() == r_pharmacopoeia ) {

        QComboBox * pCombo = new QComboBox( parent );
        pCombo->addItems( list );
        return pCombo;

    } else if ( index.row() == r_slope ) {

        QDoubleSpinBox * w = new QDoubleSpinBox( parent );
        w->setMinimum( 0.0 );
        w->setMaximum( 1000.0 );
        w->setDecimals( 3 );
        return w;

    } else {
        return QStyledItemDelegate::createEditor( parent, option, index );
  }
}

void
PeakMethodDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_value ) {
        if ( index.row() == r_pharmacopoeia ) {
            static QStringList list = { tr( "Not specified" ), tr( "EP" ), tr( "JP" ), tr( "USP" ) };
            int value = index.data().toInt() < list.size() ? index.data().toInt() : 0;
            painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, list[value] );
        } else {
            painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 3 ) );
        }
    } else if ( index.column() == c_header ) {
        QStyleOptionViewItem op = option;
        painter->save();

        QTextDocument doc;
        doc.setHtml( index.data().toString() );
        op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
        painter->translate( op.rect.left(), op.rect.top() );
        QRect clip( 0, 0, op.rect.width(), op.rect.height() );
        doc.drawContents( painter, clip );

        painter->restore();
    } else {
        QStyledItemDelegate::paint( painter, option, index );
    }
}

QSize
PeakMethodDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( index.column() == c_header ) {
		QTextDocument doc;
		doc.setHtml( index.data().toString() );
		return QSize( doc.size().width(), doc.size().height() );
	} else 
		return QStyledItemDelegate::sizeHint( option, index );
}

void
PeakMethodDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData( editor, index );
}

void
PeakMethodDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.row() == r_pharmacopoeia ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        int value = p->currentIndex();
        model->setData( index, value );
    } else
        QStyledItemDelegate::setModelData( editor, model, index );
    emit form_->valueChanged();
}

bool
PeakMethodDelegate::editorEvent( QEvent * 
                                 , QAbstractItemModel * 
                                 , const QStyleOptionViewItem&
                                 , const QModelIndex& )
{
	return false;
}

#include "peakmethodform.moc"
