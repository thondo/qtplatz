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

#include "quandocument.hpp"
#include "paneldata.hpp"
#include "quandatawriter.hpp"
#include "quansampleprocessor.hpp"
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quansequence.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <QMessageBox>
#include <QApplication>
#include <algorithm>

namespace quan {
    namespace detail {

        template< class T > struct method_writer {
            const char * nvp_;
            std::string error_code_;
            method_writer( const char * nvp ) : nvp_( nvp ) {}
            bool operator()( const boost::filesystem::path& file, const T& m ) {
                try {
                    boost::filesystem::wofstream outf( file );
                    boost::archive::xml_woarchive ar( outf );
                    ar << boost::serialization::make_nvp( nvp_, m );
                } catch ( std::exception& ex ) {
                    error_code_ = boost::diagnostic_information( ex );
                    ADERROR() << error_code_;
                    return false;
                }
                return true;
            }
        };

        template< class T > struct method_reader {
            std::string error_code_;
            bool operator()( const boost::filesystem::path& file, T& m ) {
                try {
                    boost::filesystem::wifstream inf( file );
                    boost::archive::xml_wiarchive ar( inf );
                    ar >> BOOST_SERIALIZATION_NVP( m );
                } catch ( std::exception& ex ) {
                    error_code_ = boost::diagnostic_information( ex );
                    ADERROR() << error_code_;
                    return false;
                }
                return true;
            }
        };

    }
}

using namespace quan;

QuanDocument * QuanDocument::instance_ = 0;
std::mutex QuanDocument::mutex_;

QuanDocument::~QuanDocument()
{
}

QuanDocument::QuanDocument() : quanMethod_( std::make_shared< adcontrols::QuanMethod >() )
                             , quanCompounds_( std::make_shared< adcontrols::QuanCompounds >() )
                             , quanSequence_( std::make_shared< adcontrols::QuanSequence >() )
                             , postCount_(0)
{
    std::fill( dirty_flags_.begin(), dirty_flags_.end(), true );
    connect( this, &QuanDocument::onCompleted, this, &QuanDocument::handle_completed );
}

QuanDocument *
QuanDocument::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 ) 
            instance_ = new QuanDocument();
    }
    return instance_;
}

void
QuanDocument::register_dataChanged( std::function< void( int, bool ) > f )
{
    clients_.push_back( f );
}

PanelData *
QuanDocument::addPanel( int idx, int subIdx, std::shared_ptr< PanelData >& section )
{
    auto& a_chapter = book_[ idx ];
    auto& a_page = a_chapter[ subIdx ];
    a_page.push_back( section );
    return findPanel( idx, subIdx, int( a_page.size() - 1 ) );
}

PanelData *
QuanDocument::findPanel( int idx, int subIdx, int pos )
{
    auto chapter = book_.find( idx );
    if ( chapter != book_.end() ) {
        auto page = chapter->second.find( subIdx );
        if ( page != chapter->second.end() && page->second.size() > pos )
            return page->second[ pos ].get();
    }
    return 0;
}

const adcontrols::QuanMethod&
QuanDocument::quanMethod()
{
    return *quanMethod_;
}

bool
QuanDocument::save_default_methods()
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() + L"/data/.quan" ); // /quansequence_default.xml" );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "QuanDocument"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return false;
        }
    }
    if ( dirty_flags_[ idQuanMethod ] ) {
        if ( save( dir / L"quanconfig_default.xml", *quanMethod_ ) )
            dirty_flags_[ idQuanMethod ] = false;
    }
    if ( dirty_flags_[ idQuanCompounds ] ) {
        if ( save( dir / L"quancompounds_default.xml", *quanCompounds_ ) )
            dirty_flags_[ idQuanCompounds ] = false;
    }
    if ( dirty_flags_[ idQuanSequence ] ) {
        if ( save( dir / L"quansequence_default.xml", *quanSequence_ ) )
            dirty_flags_[ idQuanSequence ] = false;            
    }
    return true;
}

bool
QuanDocument::load_default_methods()
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() + L"/data/.quan" ); // /quansequence_default.xml" );
    if ( dirty_flags_[ idQuanMethod ] ) {
        if ( load( dir / L"quanconfig_default.xml", *quanMethod_ ) )
            dirty_flags_[ idQuanMethod ] = false;
    }
    if ( dirty_flags_[ idQuanCompounds ] ) {
        if ( load( dir / L"quancompounds_default.xml", *quanCompounds_ ) )
            dirty_flags_[ idQuanCompounds ] = false;
    }
    if ( dirty_flags_[ idQuanSequence ] ) {
        if ( load( dir / L"quansequence_default.xml", *quanSequence_ ) )
            dirty_flags_[ idQuanSequence ] = false;
    }
    return std::find( dirty_flags_.begin(), dirty_flags_.end(), true ) == dirty_flags_.end();
}

void
QuanDocument::quanMethod( const adcontrols::QuanMethod& t )
{
    *quanMethod_ = t;
    dirty_flags_[ idQuanMethod ] = true;
    for ( auto& client: clients_ )
        client( idQuanMethod, false );
}

const adcontrols::QuanCompounds&
QuanDocument::quanCompounds()
{
    return *quanCompounds_;
}

void
QuanDocument::quanCompounds( const adcontrols::QuanCompounds& t )
{
    *quanCompounds_ = t;
    dirty_flags_[ idQuanCompounds ] = true;
    for ( auto& client: clients_ )
        client( idQuanCompounds, false );
}

void
QuanDocument::quanSequence( std::shared_ptr< adcontrols::QuanSequence >& ptr )
{
    quanSequence_ = ptr;
    dirty_flags_[ idQuanSequence ] = true;
}

std::shared_ptr< adcontrols::QuanSequence >
QuanDocument::quanSequence()
{
    return quanSequence_;
}

bool
QuanDocument::load( const boost::filesystem::path& file, adcontrols::QuanMethod& m )
{
    return detail::method_reader<adcontrols::QuanMethod>()( file, m );
}

bool
QuanDocument::save( const boost::filesystem::path& file, const adcontrols::QuanMethod& m )
{
    return detail::method_writer<adcontrols::QuanMethod>( "QuanMethod" )(file, m);
}

bool
QuanDocument::load( const boost::filesystem::path& file, adcontrols::QuanCompounds& m )
{
    return detail::method_reader<adcontrols::QuanCompounds>()( file, m );
}

bool
QuanDocument::save( const boost::filesystem::path& file, const adcontrols::QuanCompounds& m )
{
    return detail::method_writer<adcontrols::QuanCompounds>( "QuanCompounds" )( file, m );
}

bool
QuanDocument::load( const boost::filesystem::path& file, adcontrols::QuanSequence& t )
{
    return detail::method_reader<adcontrols::QuanSequence>()(file, t);
}

bool
QuanDocument::save( const boost::filesystem::path& file, const adcontrols::QuanSequence& t )
{
    return detail::method_writer<adcontrols::QuanSequence>( "QuanSequence" )(file, t);
}

void
QuanDocument::run()
{
    if ( quanSequence_ && quanSequence_->size() > 0 ) {
        
        QApplication::setOverrideCursor( Qt::WaitCursor );

        if ( auto writer = std::make_shared< QuanDataWriter >( quanSequence_->outfile() ) ) {
            if ( writer->open() ) {
                std::map< std::wstring, std::vector< adcontrols::QuanSample > > que;
                for ( auto it = quanSequence_->begin(); it != quanSequence_->end(); ++it )
                    que[ it->dataSource() ].push_back( *it );

                for ( auto it = que.begin(); it != que.end(); ++it ) {
                    ++postCount_;
                    threads_.push_back( std::thread( [=] () { QuanSampleProcessor( it->first, it->second )(writer); } ) );
                }
            }
        }
    }
}

void
QuanDocument::stop()
{
}

void
QuanDocument::completed( QuanSampleProcessor * p )
{
    emit onCompleted( p );
}

void
QuanDocument::handle_completed( QuanSampleProcessor * )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( postCount_ && ( --postCount_ == 0 ) ) {
        std::for_each( threads_.begin(), threads_.end(), [] ( std::thread& t ){ t.join(); } );
        threads_.clear();
        QApplication::restoreOverrideCursor();
    }
    // todo: quantitative calculation
}

void
QuanDocument::onInitialUpdate()
{
    if ( ! load_default_methods() )
        ADERROR() << "default method load failed";
    boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() + L"/data" );
    if ( std::wstring( quanMethod_->quanMethodFilename() ).empty() ) {
        quanMethod_->quanMethodFilename( ( dir / L"quanconfig.xml" ).wstring().c_str() );
    }
    if ( std::wstring( quanMethod_->quanCompoundsFilename() ).empty() ) {
        quanMethod_->quanCompoundsFilename( ( dir / L"compounds.xml" ).wstring().c_str() );
    }
    if ( std::wstring( quanMethod_->quanSequenceFilename() ).empty() ) {
        quanMethod_->quanSequenceFilename( ( dir / L"quansequence.xml" ).wstring().c_str() );
    }

    for ( auto& client: clients_ ) {
        client( idQuanMethod, true );
        client( idQuanCompounds, true );
        client( idQuanSequence, true );
    }
}

void
QuanDocument::onFinalClose()
{
    save_default_methods();
}

void
QuanDocument::setMethodFilename( int idx, const std::wstring& filename )
{
    switch ( idx )  {
    case idQuanMethod:
        quanMethod_->quanMethodFilename( filename.c_str() );
        dirty_flags_[ idQuanMethod ] = true;
        break;
    case idQuanCompounds:
        quanMethod_->quanCompoundsFilename( filename.c_str() );
        dirty_flags_[ idQuanCompounds ] = true;
        break;
    case idQuanSequence:
        quanMethod_->quanSequenceFilename( filename.c_str() );
        dirty_flags_[ idQuanSequence ] = true;
        break;
    }
}
