// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "adfs.hpp"
#include "file.hpp"
#include "folder.hpp"
#include "filesystem.hpp"
#include "fs.hpp"
#include "sqlite.hpp"
#include "attributes.hpp"
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>

namespace adfs {
    template<> ADFSSHARED_EXPORT const std::basic_string< char >
    file::name() const
    {
        return adportable::utf::to_utf8( name_ );
    }

    template<> ADFSSHARED_EXPORT const std::basic_string< wchar_t >
    file::name() const
    {
        return name_;
    }
}

using namespace adfs;

file::~file()
{
}

file::file() : db_(0)
             , rowid_(0)  // directory rowid
             , fileid_(0) // file rowid
             , is_attachment_(false)
{
}


file::file( const file& t ) : attributes( t )
                            , db_( t.db_ )
                            , name_( t.name_ )  
                            , rowid_( t.rowid_ )
                            , fileid_( t.fileid_ )
                            , is_attachment_( t.is_attachment_ )
{
}

file::file( sqlite& db
            , boost::int64_t rowid
            , const std::wstring& name
            , bool is_attachment ) : db_( &db )
                                   , name_( name )  
                                   , rowid_( rowid )
                                   , fileid_( 0 )
                                   , is_attachment_( is_attachment ) 
{
    attributes::fetch();
	if ( attributes::id() != name_ )
		attributes::id( name_ );
    fileid_ = internal::fs::rowid_from_fileid( *db_, rowid_ );
}

file::file( sqlite& db
            , boost::int64_t rowid
            , const boost::uuids::uuid& name
            , bool is_attachment ) : db_( &db )
                                   , name_( adfs::to_string< wchar_t >( name ) )  
                                   , rowid_( rowid )
                                   , fileid_( 0 )
                                   , is_attachment_( is_attachment ) 
{
    attributes::fetch();
	if ( attributes::id() != name_ )
		attributes::id( name_ );
    fileid_ = internal::fs::rowid_from_fileid( *db_, rowid_ );
}

files
file::attachments()
{
    files attachments;

    adfs::stmt sql( *db_ );
    sql.prepare( "SELECT rowid, name FROM directory WHERE type = 3 AND parent_id = :parent_id" );
    sql.bind( 1 ) = rowid_;

    while ( sql.step() == adfs::sqlite_row ) {
        boost::int64_t rowid = sql.get_column_value< int64_t >( 0 );
        std::wstring name = sql.get_column_value< std::wstring>( 1 );
        attachments.push_back( adfs::file( *db_, rowid, name ) );
    }
    return attachments;
}

const files
file::attachments() const
{
    return const_cast<adfs::file&>(*this).attachments();
}

file
file::addAttachment( const std::wstring& name )
{
    return internal::fs::add_attachment( *this, name );
}

file
file::addAttachment( const boost::uuids::uuid& uuid )
{
    return internal::fs::add_attachment( *this, uuid );
}

folder
file::getParentFolder()
{
    return internal::fs::get_parent_folder( *db_, rowid_ );
}

std::size_t
file::write( std::size_t size, const char_t * p )
{
    if ( internal::fs::write( *db_, rowid_, size, p ) ) {
        attributes::commit();
        return size;
    }
    return 0;
}

std::size_t
file::read( std::size_t size, char_t * p )
{
    int64_t fid = fileid_ > 0 ? fileid_ : internal::fs::rowid_from_fileid( *db_, rowid_ );
    if ( internal::fs::read( *db_, fid, size, p ) )
        return size;
    return 0;
}

std::size_t
file::size() const
{
    int64_t fid = fileid_ > 0 ? fileid_ : internal::fs::rowid_from_fileid( *db_, rowid_ );
    return internal::fs::size( *db_, fid );
}
