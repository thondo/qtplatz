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
#include <compiler/disable_unused_parameter.h>
#include "datafile.hpp"
#include <adutils/cpio.hpp>
#include "rawdata_v2.hpp"
#include "rawdata_v3.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <adportable/string.hpp>
#include <adportable/posix_path.hpp>
#include <adlog/logger.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adfs/adfs.hpp>
#include <adfs/attributes.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/cpio.hpp>
#include <adutils/fsio.hpp>
#include <algorithm>
#include <iostream>
#include <compiler/make_unique.hpp>

/////////////////
namespace addatafile { namespace detail {

    static adcontrols::datafile * nullfile(0);

        struct folder {
            static bool save( adfs::filesystem& db, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folder& );
            static bool load( portfolio::Folder parent, const adfs::folder& adf );
        };

        struct folium {
            static bool save( adfs::folder&, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& src );
        };

        struct attachment {
            static bool save( adfs::file& parent, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& adf );
        };

        struct import {
            static void attributes( adfs::attributes&, const portfolio::attributes_type& );
            static void attributes( portfolio::Folium&, const adfs::attributes& );
            static void attributes( portfolio::Folder&, const adfs::attributes& );
        };

        template< class T > struct serializer {

            static std::shared_ptr< T > deserialize( const std::vector<char>& obuf ) { //adfs::detail::cpio& obuf ) {
                std::shared_ptr<T> ptr = std::make_shared<T>();
                try {
                    adfs::cpio::deserialize( *ptr, obuf.data(), obuf.size() );
                } catch ( std::exception& e ) {
                    ADERROR() << "exception: " << e.what() << " while deserializing " << typeid(T).name();
                    ptr.reset();
                }
                return ptr;
            }
        };

        struct is_valid_rawdata : public boost::static_visitor < bool > {
            bool operator()( std::unique_ptr< addatafile::v2::rawdata >& t ) {
                return true;
            }
            bool operator()( std::unique_ptr< addatafile::v3::rawdata >& t ) {
                return true;
            }
            //template<typename T> bool operator()( T& t ) const { return t.get() != nullptr; }
        };

        struct subscribe_rawdata : boost::static_visitor < void > {
            template<typename T, typename Subscriber = adcontrols::dataSubscriber > void operator()( T& t, Subscriber& sub ) const {
                t->loadAcquiredConf();
                sub.subscribe( *t );
                t->loadCalibrations();
            }
        };

        struct undefined_spectrometers : boost::static_visitor<void> {
            std::wostringstream& o_;
            template<typename T> void operator()( T& t ) const {
                for ( auto& s : t->undefined_spectrometers() )
                    o_ << s << L"\r";
            }
        };

    }

}

using namespace addatafile;

datafile::~datafile()
{
}

datafile::datafile() : mounted_( false )
{
    //, rawdata_( new rawdata( dbf_, *this ) )
}

void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    if ( mounted_ ) {

        // handle acquired raw data
        
        if ( adutils::AcquiredConf::formatVersion( dbf_.db() ) == adutils::format_v2 ) {
            rawdata_ = std::make_shared< v2::rawdata >( dbf_, *this );
        } else if ( adutils::AcquiredConf::formatVersion( dbf_.db() ) == adutils::format_v3 ) {
            rawdata_ = std::make_shared< v3::rawdata >( dbf_, *this );
        }

        if ( rawdata_.which() == 0 ) {
            auto rawdata = boost::get< std::shared_ptr<v2::rawdata> >( rawdata_ );
            rawdata->loadAcquiredConf();
            sub.subscribe( *rawdata );
            rawdata->loadCalibrations();

            if ( ! rawdata->undefined_spectrometers().empty() ) {
                std::wostringstream o;
                for ( auto& s : rawdata->undefined_spectrometers() )
                    o << s << L"\r";
                sub.notify( adcontrols::dataSubscriber::idUndefinedSpectrometers, o.str().c_str() );                
            }
        }
#if 0        
        detail::is_valid_rawdata x;
        boost::apply_visitor( x( rawdata_ ) );
        boost::apply_visitor( detail::is_valid_rawdata()( rawdata_ ) );

            boost::apply_visitor( detail::subscribe_rawdata()( rawdata_, sub ) );

            std::vector< std::wstring > undefined_spectrometers;
            // publish acquired dataset <LCMSDataset>
        
            //if ( rawdata_->loadAcquiredConf() )
            //sub.subscribe( *rawdata_ );
        
            //rawdata_->loadCalibrations();

            std::wostringstream o;
            boost::apply_visitor( undefined_spectrometers( o )( rawdata_ ) );

            if ( !o.str().empty() )
                sub.notify( adcontrols::dataSubscriber::idUndefinedSpectrometers, o.str().c_str() );
        }
#endif
    }
    
    do {
        // publish processed dataset
        portfolio::Portfolio portfolio;
        if ( loadContents( portfolio, L"/Processed" ) && processedDataset_ ) {
            processedDataset_->xml( portfolio.xml() );
            sub.subscribe( *processedDataset_ );
        } else {
            portfolio.create_with_fullpath( filename_ );
            portfolio.addFolder( L"Chromatograms" );
            portfolio.addFolder( L"Spectra" );
            processedDataset_->xml( portfolio.xml() );
            sub.subscribe( *processedDataset_ );
        }
        
    } while (0);
}

bool
datafile::open( const std::wstring& filename, bool /* readonly */ )
{
    filename_ = filename;
    processedDataset_.reset( new adcontrols::ProcessedDataset );
    
    if ( ( mounted_ = dbf_.mount( filename.c_str() ) ) ) 
        return true;
    
    if ( ( mounted_ = dbf_.create( filename.c_str() ) ) )
        return true;

    return false;
}

boost::any
datafile::fetch( const std::wstring& dataId, const std::wstring& dataType ) const
{
    adfs::stmt sql( dbf_.db() );
    sql.prepare( "SELECT rowid FROM file WHERE fileid = (SELECT rowid FROM directory WHERE name = ?)" );
    sql.bind( 1 ) = dataId;

#if defined DEBUG && 0
    adportable::debug(__FILE__, __LINE__) << "==> fetch(" << dataId << ", " << dataType << ")";
#endif

    boost::any any;
    size_t n = 0;
    while ( sql.step() == adfs::sqlite_row ) {
        ++n;
        assert( n <= 1 );  // should only be one results

        boost::int64_t rowid = sql.get_column_value<int64_t>( 0 );  // file id
        
        adfs::blob blob;
        if ( rowid && blob.open( dbf_.db(), "main", "file", "data", rowid, adfs::readonly ) ) {
            if ( blob.size() ) {
                std::vector< char > obuf( blob.size() );
                if ( blob.read( reinterpret_cast<int8_t *>(obuf.data()), blob.size() ) ) {

					if ( dataType == adcontrols::MassSpectrum::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::MassSpectrum >::deserialize( obuf );
                        
					} else if ( dataType == adcontrols::Chromatogram::dataClass() ) {
						
                        any = detail::serializer< adcontrols::Chromatogram >::deserialize( obuf );
                        
					} else if ( dataType == adcontrols::PeakResult::dataClass() ) {
                        
						any = detail::serializer< adcontrols::PeakResult >::deserialize( obuf );    
                        
					} else if ( dataType == adcontrols::ProcessMethod::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::ProcessMethod >::deserialize( obuf );    
                        
					} else if ( dataType == adcontrols::MSCalibrateResult::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::MSCalibrateResult >::deserialize( obuf );    
                        
					} else if ( dataType == adcontrols::MSPeakInfo::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::MSPeakInfo >::deserialize( obuf );

					} else if ( dataType == adcontrols::MassSpectra::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::MassSpectra >::deserialize( obuf );

					} else if ( dataType == adcontrols::Targeting::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::Targeting >::deserialize( obuf );

					} else if ( dataType == adcontrols::QuanSample::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::QuanSample >::deserialize( obuf );

					} else if ( dataType == adcontrols::QuanSequence::dataClass() ) {
                        
                        any = detail::serializer< adcontrols::QuanSequence >::deserialize( obuf );

                        
                    } else {
                        ADERROR() << "Error: unknown data type in datafile::fetch(" << dataId << ", " << dataType << ")";
                        BOOST_THROW_EXCEPTION( std::bad_typeid() );
                    }
                }
            }
        }
    }
    return any;
}

////////////////////////////////////////////////////
// SaveFileAs come in here
bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio, const adcontrols::datafile& source )
{
    if ( ! mounted_ )
        return false;

    std::vector< std::string > removed;
    if ( portfolio.removed_dataids( removed ) > 0 ) {
        removeContents( removed );
    }

    if ( calibration_modified_ ) {
        for ( auto calib: calibrations_ )
            adutils::fsio::save_mscalibfile( dbf_, *calib.second );
    }

    adfs::stmt sql( dbf_.db() );
    sql.begin();

    dbf_.addFolder( path );

    adportable::path name( path );

    for ( const portfolio::Folder& folder : portfolio.folders() ) {
        detail::folder::save( dbf_, name, source, folder );
    }

    sql.commit();
    return true;
}


// Save comes here. Thus no file name pass to here
bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio )
{
    if ( ! mounted_ )
        return false;

    std::vector< std::string > removed;
    if ( portfolio.removed_dataids( removed ) > 0 )
        removeContents( removed );

    adfs::stmt sql( dbf_.db() );

    sql.begin();

    dbf_.addFolder( path );

    adportable::path name( path );

    for ( const portfolio::Folder& folder: portfolio.folders() )
        detail::folder::save( dbf_, name, *detail::nullfile, folder );

    sql.commit();
    return true;
}

bool
datafile::loadContents( const std::wstring& path, const std::wstring& id, adcontrols::dataSubscriber& sub )
{
    if ( ! mounted_ )
        return false;
    adfs::folder folder = dbf_.findFolder( path );
    if ( folder ) {
        const std::wstring& name = folder.name();
		(void)name;
		adfs::file file = dbf_.findFile( folder, id );
        if ( file ) 
			sub.onFileAdded( path, file );
    }
    return false;
}

bool
datafile::loadContents( portfolio::Portfolio& portfolio, const std::wstring& query )
{
    if ( ! mounted_ )
        return false;

    portfolio.create_with_fullpath( filename_ );
    adfs::folder processed = dbf_.findFolder( query );  // L"/Processed"
    if ( ! processed )
        return false;

    // top folder should be L"Spectra" | L"Chromatograms"
    for ( const adfs::folder& folder: processed.folders() ) {
        const std::wstring& name = folder.name();
        portfolio::Folder xmlfolder = portfolio.addFolder( name );
        detail::folder::load( xmlfolder, folder );
    }

    processedDataset_.reset( new adcontrols::ProcessedDataset );
    std::string xml = portfolio.xml();
    processedDataset_->xml( xml );

    return true;
}

bool
datafile::removeContents( const std::vector< std::string >& dataids )
{
    // not completed yet
    if ( ! mounted_ )
        return false;

    adfs::stmt sql( dbf_.db() );

    for ( auto& dataid : dataids ) {
        sql.prepare( "DELETE FROM file WHERE fileid = (SELECT fileid FROM directory WHERE name = ?)" );
        sql.bind( 1 ) = dataid;
        if ( sql.step() == adfs::sqlite_done ) {
            sql.prepare( "DELETE FROM directory WHERE name = ?" );
            sql.bind( 1 ) = dataid;
            sql.step();
        }
    }
    return true;
}

bool
datafile::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& result )
{
    if ( rawdata_.which() == 0 ) {
        if ( auto rawdata = boost::get< std::shared_ptr< v2::rawdata > >( rawdata_ ) )
            rawdata->applyCalibration( dataInterpreterClsid, result );
    }
    return true;
}

bool
datafile::readCalibration( size_t idx, adcontrols::MSCalibrateResult& result ) const
{
    return false; //rawdata_->readCalibration( idx, result );
}

///-------------------------------------------------------------------------------------

namespace addatafile {
    namespace detail {

        bool
        attachment::save( adfs::file& parent, const boost::filesystem::path& path
                          , const adcontrols::datafile& source, const portfolio::Folium& folium )
        {
            boost::filesystem::path filename = adportable::path::posix( path / folium.id() );

            adfs::file dbThis = parent.addAttachment( folium.id() );
            import::attributes( dbThis, folium.attributes() );

#if defined DEBUG && 0
            const std::wstring& dataclass = folium.dataClass();
            const std::wstring& name = folium.name();
            adportable::debug( __FILE__, __LINE__ ) << "addatafile::detail::attachment::save(" 
                                                    << dataclass << ", " << name << ")";
#endif
            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() && (&source != nullfile ) )
                any = source.fetch( folium.id(), folium.dataClass() );

            if ( ! any.empty() ) {
                try {
                    adutils::cpio::save( dbThis, any );
                } catch ( boost::bad_any_cast& ) {
                    assert( 0 );
                    return false;
                }
                
                for ( const portfolio::Folium& att: folium.attachments() )
                    save( dbThis, filename, source, att );
            }
            return true;
        }
        //------------

        bool
        folium::save( adfs::folder& folder, const boost::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folium& folium )
        {
            boost::filesystem::path filename = adportable::path::posix( path / folium.id() );

            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() && (&source != nullfile ) )
                any = source.fetch( folium.id(), folium.dataClass() );

            if ( folder && !any.empty() ) {
                adfs::file dbf = folder.addFile( folium.id() );

                import::attributes( dbf, folium.attributes() );
                try {
                    adutils::cpio::save( dbf, any );
                }
                catch ( boost::exception& ex ) {
                    ADTRACE() << boost::diagnostic_information( ex );
                }

                for ( const portfolio::Folium& att : folium.attachments() ) {
                    try {
                        detail::attachment::save( dbf, filename, source, att );
                    }
                    catch ( boost::exception& ex) {
                        ADTRACE() << boost::diagnostic_information( ex );
                    }
                }
            }
            return true;
        }

        // struct folder {
        bool
        folder::save( adfs::filesystem& dbf, const boost::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folder& folder )
        {
            boost::filesystem::path pathname = adportable::path::posix( path / folder.name() );

            adfs::folder dbThis = dbf.addFolder( pathname.wstring() );
            import::attributes( dbThis, folder.attributes() );

            // save all files in this folder
            for ( const portfolio::Folium& folium: folder.folio() )
                folium::save( dbThis, pathname, source, folium );
    
            // recursive save sub folders
            for ( const portfolio::Folder& subfolder: folder.folders() )
                folder::save( dbf, pathname, source, subfolder );

            return true;
        }

        bool folder::load( portfolio::Folder parent, const adfs::folder& adfolder )
        {
            for ( const adfs::file& file: adfolder.files() )
                folium::load( parent.addFolium( file.name() ), file );
            return true;
        }

        bool folium::load( portfolio::Folium dst, const adfs::file& src )
        {
#if defined DEBUG && 0
            adportable::debug(__FILE__, __LINE__) 
                << ">> folium::load(" << src.attribute(L"name") << ") " 
                << src.attribute(L"dataType") << ", " << src.attribute(L"dataId");
#endif
            import::attributes( dst, src );
            for ( const adfs::file& att: src.attachments() )
                attachment::load( dst.addAttachment( att.name() ), att );
            return true;
        }

        bool attachment::load( portfolio::Folium dst, const adfs::file& src )
        {
#if defined DEBUG && 0
            adportable::debug(__FILE__, __LINE__)
                << " +++ attachment::load(" << src.attribute(L"name") << ") " 
                << src.attribute(L"dataType") << ", " << src.attribute(L"dataId");
#endif
            import::attributes( dst, src );
            for ( const adfs::file& att: src.attachments() )
                attachment::load( dst.addAttachment( att.name() ), att );
            return true;
        }

        //---
        void import::attributes( portfolio::Folium& d, const adfs::attributes& s )
        {
            for ( adfs::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
                d.setAttribute( it->first, it->second );
        }

        void import::attributes( portfolio::Folder& d, const adfs::attributes& s )
        {
            for ( adfs::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
                d.setAttribute( it->first, it->second );
        }

        void import::attributes( adfs::attributes& d, const portfolio::attributes_type& s )
        {
            for ( const portfolio::attribute_type& a: s ) 
                d.setAttribute( a.first, a.second );
        }
        //---

    }
}

//////////////////////////

