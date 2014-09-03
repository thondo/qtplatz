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

#include "dataproc_document.hpp"
#include "dataprocessor.hpp"
#include "mainwindow.hpp"
#include <adcontrols/msqpeaks.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/processmethod.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/settings.hpp>
#include <xmlparser/pugixml.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/all.hpp>
#include <atomic>

namespace dataproc {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "dataproc";
        }
    };

}

using namespace dataproc;

std::atomic<dataproc_document * > dataproc_document::instance_(0);
std::mutex dataproc_document::mutex_;

dataproc_document::dataproc_document(QObject *parent) : QObject(parent)
                                    , quant_( std::make_shared< adcontrols::MSQPeaks >() )
                                    , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                                , QLatin1String( "dataproc" ) ) )
{
}

dataproc_document * 
dataproc_document::instance()
{
    typedef dataproc_document T;

    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

std::shared_ptr< adcontrols::ProcessMethod >
dataproc_document::processMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return pm_;
}

void
dataproc_document::setProcessMethod( const adcontrols::ProcessMethod& m )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    pm_ = std::make_shared< adcontrols::ProcessMethod >( m );
    emit onProcessMethodChanged();
}

void
dataproc_document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, filename );
}

void
dataproc_document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::dataproc_document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = qtwrapper::settings(*settings_).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
    if ( path.isEmpty() ) {
        auto path = QString::fromStdWString( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }
    // fake project directory for help initial openfiledialog location
    Core::DocumentManager::setProjectsDirectory( path );
    Core::DocumentManager::setUseProjectsDirectory( true );

    boost::filesystem::path mfile( dir / "default.pmth" );
    if ( boost::filesystem::exists( mfile ) ) {
        
        adfs::filesystem fs;
        if ( fs.mount( mfile.wstring().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/ProcessMethod" );

            auto files = folder.files();
            if ( !files.empty() ) {
                auto file = files.back();
                pm_ = std::make_shared< adcontrols::ProcessMethod >();
                try {
                    file.fetch( *pm_ );
                }
                catch ( std::exception& ex ) {
                    QMessageBox::information( 0, "dataproc -- Open default process method"
                                              , (boost::format( "Failed to open last used process method file: %1% by reason of %2% @ %3% #%4%" )
                                              % mfile.string() % ex.what() % __FILE__ % __LINE__).str().c_str() );
                    return;
                }
                procmethod_filename_ = mfile.generic_wstring();
            }
        }
    }
}

void
dataproc_document::finalClose()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::dataproc_document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
    
    boost::filesystem::path fname( dir / "default.pmth" );
    adfs::filesystem file;
    if ( !file.create( fname.wstring().c_str() ) ) {
        ADTRACE() << "Error in dataproc_document::finalClose: \"" << fname.string() << "\" can't be created";
        return;
    }
    
    adfs::folder folder = file.addFolder( L"/ProcessMethod" );
    adfs::file adfile = folder.addFile( fname.wstring() );
    try {
        adfile.save( *pm_ );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception in dataproc_document::finalClose: " << boost::diagnostic_information( ex );
    }
    adfile.dataClass( adcontrols::ProcessMethod::dataClass() );
    adfile.commit();

    boost::filesystem::path xmlfile( dir / "default.pmth.xml" );
    if ( boost::filesystem::exists( xmlfile ) ) 
        boost::filesystem::remove( xmlfile );

    std::wstringstream o;
    try {
        adcontrols::ProcessMethod::xml_archive( o, *pm_ );
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
        ADTRACE() << "Exception in dataproc_document::finalClose: " << boost::diagnostic_information( ex );
    }
    pugi::xml_document doc;
    doc.load( o );
    // todo: add style sheet
    doc.save_file( xmlfile.string().c_str() );
}

adcontrols::MSQPeaks *
dataproc_document::msQuanTable()
{
    return quant_.get();
}

const adcontrols::MSQPeaks *
dataproc_document::msQuanTable() const
{
    return quant_.get();
}

void
dataproc_document::setMSQuanTable( const adcontrols::MSQPeaks& v )
{
    quant_ = std::make_shared< adcontrols::MSQPeaks >( v );
}

// static
size_t
dataproc_document::findCheckedTICs( Dataprocessor * dp, std::set< int >& vfcn )
{
    vfcn.clear();
    if ( dp ) {
        auto cfolder = dp->portfolio().findFolder( L"Chromatograms" );
        for ( auto& folium: cfolder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                const std::wstring& name = folium.name();
                auto found = name.find( std::wstring( L"TIC/TIC.") );
                if ( found != std::wstring::npos ) {
                    auto dot = name.find_last_of( L'.' );
                    if ( dot != std::wstring::npos ) {
                        int fcn = std::stoi( name.substr( dot + 1 ) );
                        vfcn.insert( fcn - 1 );
                    }
                }
            }
        }
    }
    return vfcn.size();
}

//static
const std::shared_ptr< adcontrols::Chromatogram >
dataproc_document::findTIC( Dataprocessor * dp, int fcn )
{
    if ( dp ) {
        auto cfolder = dp->portfolio().findFolder( L"Chromatograms" );
        std::wstring name = ( boost::wformat( L"TIC/TIC.%d" ) % ( fcn + 1 ) ).str();
        if ( auto folium = cfolder.findFoliumByName( name ) ) {
            auto cptr = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium );
            return cptr;
        }
    }
    return 0;
}

