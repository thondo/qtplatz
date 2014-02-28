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

#include "u5303amethodtable.hpp"
#include <u5303a/digitizer.hpp>
#include <qtwrapper/font.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QHeaderView>

namespace u5303a {
    
    class u5303AMethodDelegate : public QStyledItemDelegate {
    public:
    };

}

using namespace u5303a;

u5303AMethodTable::~u5303AMethodTable()
{
    delete model_;
}

u5303AMethodTable::u5303AMethodTable(QWidget *parent) : QTableView(parent)
                                                      , model_( new QStandardItemModel )
{
    setModel( model_ );
	setItemDelegate( new u5303AMethodDelegate );
    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );
}
    
void
u5303AMethodTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    model.setColumnCount( 3 );
    model.setRowCount( 8 );
    
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "parameter" ) );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "value" ) );
    model.setHeaderData( 3, Qt::Horizontal, QObject::tr( "description" ) );

    u5303a::method m; // constructor default method for initial display
    int row = 0;
    model.setData( model.index( row, 0 ), "front end range" );
    model.setData( model.index( row, 1 ), m.front_end_range );
    model.setData( model.index( row, 2 ), "1V/2V range" );
    ++row;
    model.setData( model.index( row, 0 ), "front end offset" );
    model.setData( model.index( row, 1 ), m.front_end_offset );
    model.setData( model.index( row, 2 ), "[-0.5V,0.5V], [-1V,1V] offset" );
    ++row;
    model.setData( model.index( row, 0 ), "sampling rage" );
    model.setData( model.index( row, 1 ), m.samp_rate );
    model.setData( model.index( row, 2 ), "sampling rate (3.2GS/s)" );
    ++row;
    model.setData( model.index( row, 0 ), "ext. trigger level" );
    model.setData( model.index( row, 1 ), m.ext_trigger_level );
    model.setData( model.index( row, 2 ), "external trigger threshold" );
    ++row;
    model.setData( model.index( row, 0 ), "number of samples" );
    model.setData( model.index( row, 1 ), m.nbr_of_s_to_acquire );
    model.setData( model.index( row, 2 ), "number of sample points in a spectrum" );
    ++row;
    model.setData( model.index( row, 0 ), "number of average" );
    model.setData( model.index( row, 1 ), m.nbr_of_averages );
    model.setData( model.index( row, 2 ), "number of averages ninus one." );
    ++row;
    model.setData( model.index( row, 0 ), "delay to first sample" );
    model.setData( model.index( row, 1 ), m.delay_to_first_s );
    model.setData( model.index( row, 2 ), "delay to first sample" );
    ++row;
    model.setData( model.index( row, 0 ), "invert signal" );
    model.setData( model.index( row, 1 ), m.invert_signal ? true : false );
    model.setData( model.index( row, 2 ), "inversion" );
    ++row;
    model.setData( model.index( row, 0 ), "nsa" );
    model.setData( model.index( row, 1 ), m.nsa );
    model.setData( model.index( row, 2 ), "bit[31]->enable, bit[11:0]->threshold" );

    resizeColumnsToContents();
	resizeRowsToContents();

    for ( int row = 0; row < model.rowCount(); ++row ) {
        model.item( row, 0 )->setEditable( false );
        model.item( row, 1 )->setEditable( true );
        model.item( row, 2 )->setEditable( false );
    }
}

bool
u5303AMethodTable::setContents( const u5303a::method& m )
{
    QStandardItemModel& model = *model_;

    int row = 0;
    model.setData( model.index( row, 1 ), m.front_end_range );
    ++row;
    model.setData( model.index( row, 1 ), m.front_end_offset );
    ++row;
    model.setData( model.index( row, 1 ), m.ext_trigger_level );
    ++row;
    model.setData( model.index( row, 1 ), m.samp_rate );
    ++row;
    model.setData( model.index( row, 1 ), m.nbr_of_s_to_acquire );
    ++row;
    model.setData( model.index( row, 1 ), m.nbr_of_averages );
    ++row;
    model.setData( model.index( row, 1 ), m.delay_to_first_s );
    ++row;
    model.setData( model.index( row, 1 ), m.invert_signal ? true : false );
    ++row;
    model.setData( model.index( row, 1 ), m.nsa );
	
	return true;
}

bool
u5303AMethodTable::getContents( u5303a::method& m )
{
    QStandardItemModel& model = *model_;

    int row = 0;
	m.front_end_range = model.index( row, 1 ).data().toDouble();
    ++row;
	m.front_end_offset = model.index( row, 1 ).data().toDouble();
    ++row;
	m.ext_trigger_level = model.index( row, 1 ).data().toDouble();
    ++row;
	m.samp_rate = model.index( row, 1 ).data().toDouble();
    ++row;
	m.nbr_of_s_to_acquire = model.index( row, 1 ).data().toInt();
    ++row;
	m.nbr_of_averages = model.index( row, 1 ).data().toInt();
    ++row;
	m.delay_to_first_s = model.index( row, 1 ).data().toInt();
    ++row;
    m.invert_signal = model.index( row, 1 ).data().toBool() ? 1 : 0;
    ++row;
    m.nsa = model.index( row, 1 ).data().toInt();
	
	return true;
}