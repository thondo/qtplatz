/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "acquiredocument.hpp"
#include "constants.hpp"
#include "mainwindow.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adinterface/automaton.hpp>
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

namespace acquire {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "acquire";
        }
    };

    class document::fsm : public adinterface::fsm::handler {
    public:
        adinterface::fsm::controller automaton_;
        adinterface::instrument::eInstStatus instStatus_;

        fsm() : automaton_( this )
            , instStatus_( adinterface::instrument::eOff ) {
        }

        // finite automaton handler
        void handle_state( bool entering, adinterface::instrument::eInstStatus stat ) override {
            if ( entering ) {
                instStatus_ = stat;
                emit document::instance()->instStateChanged( stat );
            }
        }

        void action_on( const adinterface::fsm::onoff& ) override {
            automaton_.process_event( adinterface::fsm::prepare( std::string() ) );
            ADDEBUG() << "fsm action_on";
        }

        void action_prepare_for_run( const adinterface::fsm::prepare& ) override {
            ADDEBUG() << "fsm action_prepare_for_run";
        }

        void action_start_run( const adinterface::fsm::run& ) override {
            ADDEBUG() << "fsm action_start_run";
        }

        void action_stop_run( const adinterface::fsm::stop& ) override {
            // stop current run
            ADDEBUG() << "fsm action_stop_run";
        }

        void action_off( const adinterface::fsm::onoff& ) override {
            ADDEBUG() << "fsm action_off";
        }

        void action_diagnostic( const adinterface::fsm::onoff& ) override {
            ADDEBUG() << "fsm action_diagnostic";
        }

        void action_error_detected( const adinterface::fsm::error_detected& ) override {
            ADDEBUG() << "fsm action_error_detected";
        }
    };
}

using namespace acquire;

std::atomic< document * > document::instance_(0);
std::mutex document::mutex_;

document::~document()
{
}

document::document(QObject *parent) : QObject(parent)
                                    , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                                , QLatin1String( "acquire" ) ) )
                                    , cm_( std::make_shared< adcontrols::ControlMethod >() )
                                    , sampleRun_( std::make_shared< adcontrols::SampleRun >() )
                                    , fsm_( new fsm() )
{
}

document * 
document::instance()
{
    typedef document T;

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

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_DATA_FILES, constants::KEY_FILES, filename );
}

void
document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }
    // fake project directory for help initial openfiledialog location
    Core::DocumentManager::setProjectsDirectory( path );
    Core::DocumentManager::setUseProjectsDirectory( true );

    boost::filesystem::path mfile( dir / "default.cmth" );
    adcontrols::ControlMethod cm;
    if ( load( QString::fromStdWString( mfile.wstring() ), cm ) )
        setControlMethod( cm, QString() ); // don't save default name

    boost::filesystem::path sfile( dir / "samplerun.sequ" );
    adcontrols::SampleRun sr;
    if ( load( QString::fromStdWString( sfile.wstring() ), sr ) )
        setSampleRun( sr, QString() ); // don't save default name
}

void
document::finalClose( internal::MainWindow * mainwindow )
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
    mainwindow->getControlMethod( *cm_ );
    boost::filesystem::path fname( dir / "default.cmth" );
    save( QString::fromStdWString( fname.wstring() ), *cm_ );

    mainwindow->getSampleRun( *sampleRun_ );
    boost::filesystem::path sname( dir / "samplerun.sequ" );
    save( QString::fromStdWString( sname.wstring() ), *sampleRun_ );
}

std::shared_ptr< adcontrols::ControlMethod >
document::controlMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return cm_;
}

std::shared_ptr< adcontrols::SampleRun >
document::sampleRun() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return sampleRun_;
}

void
document::setControlMethod( const adcontrols::ControlMethod& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        cm_ = std::make_shared< adcontrols::ControlMethod >( m );
        for ( auto& item : m ) {
            ADDEBUG() << item.modelname() << ", " << item.itemLabel() << " initial: " << item.isInitialCondition() << " time: " << item.time();
        }
    } while(0);

    if ( ! filename.isEmpty() ) {
        ctrlmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_METHOD_FILES, constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

void
document::setSampleRun( const adcontrols::SampleRun& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        sampleRun_ = std::make_shared< adcontrols::SampleRun >( m );
    } while(0);

    if ( ! filename.isEmpty() ) {
        samplerun_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_SEQUENCE_FILES, constants::KEY_FILES, filename );
    }

    emit onSampleRunChanged( filename );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *settings_ ).recentFile( group, constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *settings_ ).recentFile( constants::GRP_DATA_FILES, constants::KEY_FILES );

        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

bool
document::load( const QString& filename, adcontrols::ControlMethod& m )
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem fs;
        if ( fs.mount( filename.toStdWString().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/ControlMethod" );
        
            auto files = folder.files();
            if ( !files.empty() ) {
                auto file = files.back();
                try {
                    file.fetch( m );
                }
                catch ( std::exception& ex ) {
                    QMessageBox::information( 0, "acquire -- Open default process method"
                                              , (boost::format( "Failed to open last used process method file: %1% by reason of %2% @ %3% #%4%" )
                                                 % filename.toStdString() % ex.what() % __FILE__ % __LINE__).str().c_str() );
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

bool
document::save( const QString& filename, const adcontrols::ControlMethod& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }
    
    adfs::folder folder = file.addFolder( L"/ControlMethod" );
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::dataClass() );
        adfile.save( m );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

    return true;
}

bool
document::load( const QString& filename, adcontrols::SampleRun& m )
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem fs;
        if ( fs.mount( filename.toStdWString().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/SampleRun" );
        
            auto files = folder.files();
            if ( !files.empty() ) {
                auto file = files.back();
                try {
                    file.fetch( m );
                }
                catch ( std::exception& ex ) {
                    QMessageBox::information( 0, "acquire -- Open default sample run"
                                              , (boost::format( "Failed to open last used sample run file: %1% by reason of %2% @ %3% #%4%" )
                                                 % filename.toStdString() % ex.what() % __FILE__ % __LINE__).str().c_str() );
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

bool
document::save( const QString& filename, const adcontrols::SampleRun& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }
    
    adfs::folder folder = file.addFolder( L"/SampleRun" );
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::dataClass() );
        adfile.save( m );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

    do {
        boost::filesystem::path xmlfile( filename.toStdWString() );
        xmlfile.replace_extension( ".xml" );
        std::wostringstream os;
        adcontrols::SampleRun::xml_archive( os, m );
        pugi::xml_document dom;
        dom.load( pugi::as_utf8( os.str() ).c_str() );
        dom.save_file( xmlfile.string().c_str() );
    } while(0);

    return true;
}

void
document::handleOnOff()
{
    //fsm_->automaton_.process_event( adinterface::fsm::onoff() );
}

void
document::handlePrepareForRun()
{
    //fsm_->automaton_.process_event( adinterface::fsm::prepare( std::string() ) );
}

void
document::handleRun()
{
    //fsm_->automaton_.process_event( adinterface::fsm::run() );
}

void
document::handleStop()
{
    //fsm_->automaton_.process_event( adinterface::fsm::stop() );
}