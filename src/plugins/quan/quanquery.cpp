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

#include "quanquery.hpp"
#include <adfs/sqlite.hpp>
#include <adfs/sqlite3.h>
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <QObject>

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

using namespace quan;

QuanQuery::QuanQuery( adfs::sqlite& db ) : sql_( db )
{
}

QuanQuery::QuanQuery( const QuanQuery& t ) : std::enable_shared_from_this< QuanQuery >( t )
                                           , sql_( t.sql_ )
{
}

bool
QuanQuery::prepare( const std::wstring& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

bool
QuanQuery::prepare( const std::string& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

adfs::sqlite_state
QuanQuery::step()
{
    state_ = sql_.step();
    return state_;
}

size_t
QuanQuery::column_count() const
{
    return sql_.column_count();
}

QString
QuanQuery::column_name( size_t idx ) const
{
    std::string name = sql_.column_name( int( idx ) );
    return QString::fromStdString( name );
}

bool
QuanQuery::hasColumn( const std::string& column, const std::string& table )
{
    sql_.reset();
    sql_.prepare( "PRAGMA table_info('" + table + "')" );
    while ( sql_.step() == adfs::sqlite_row ) {
        std::string a = sql_.get_column_value< std::string >( 1 );
        if ( strcasecmp(a.c_str(), column.c_str() ) ) {
            sql_.reset();
            return true;
        }
    }
    return false;
}

QString
QuanQuery::column_name_tr( const QString& d )
{
    struct column_name {
        const QString loc_c;
        const QString i10n;
    };
    column_name names[] = {
            { "dataSource", QObject::tr( "dataSource" ) }
            , { "formula", QObject::tr( "formula" ) }
            , { "mass", QObject::tr( "mass" ) }
            , { "exact mass", QObject::tr( "exact mass" ) }
            , { "error(Da)", QObject::tr( "error(Da)" ) }
            , { "row", QObject::tr( "row" ) }
            , { "level", QObject::tr( "level" ) }
            , { "intensity", QObject::tr( "intensity" ) }
            , { "sampleType", QObject::tr( "sample type" ) }
            , { "name", QObject::tr( "name" ) }
            , { "description", QObject::tr( "description" ) }
            , { "amount", QObject::tr( "amount" ) }
            , { "idCompound", QObject::tr( "id" ) }
    };
    
    for ( auto& t : names ) {
        if ( t.loc_c == d )
            return t.i10n;
    }
    return d;
}


QVariant
QuanQuery::column_value( size_t idx ) const
{
    switch ( sql_.column_type( int( idx ) ) ) {
    case SQLITE_INTEGER: return QVariant( static_cast< qlonglong >( sql_.get_column_value< int64_t >( int( idx ) ) ) );
    case SQLITE_FLOAT:   return QVariant( sql_.get_column_value< double >( int( idx ) ) );
    case SQLITE_TEXT:    return QVariant( QString( sql_.get_column_value< std::string >( int( idx ) ).c_str() ) );
    case SQLITE_BLOB: {
        try {
            auto uuid = sql_.get_column_value< boost::uuids::uuid >( int( idx ) );
            return QVariant( QString( boost::lexical_cast<std::string>(uuid).c_str() ) );
        }
        catch ( boost::bad_lexical_cast& ) {
        }
    }
    case SQLITE_NULL:    return QVariant();
    }
    return QVariant();
}

