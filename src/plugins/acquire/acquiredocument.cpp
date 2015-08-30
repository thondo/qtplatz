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
#include <adportable/date_string.hpp>
#include <adportable/utf.hpp>
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
#include <boost/date_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>
#include <atomic>

namespace acquire {

    namespace fsm {
        // events
        namespace msm = boost::msm;
        namespace mpl = boost::mpl;
        using boost::msm::front::Row;
        using boost::msm::front::none;
        
        struct prepare {};
        struct start_run {};
        struct stop {};
        struct inject {};

        // front-end
        struct acquire_ : public msm::front::state_machine_def< acquire_ > {
            template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: peripheral"; }
            template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: peripheral"; }

            struct Empty : public msm::front::state<> {
                // Empty method and sample
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Empty"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Empty"; }
            };
            struct Preparing : public msm::front::state<> {
                // Preparing state; we have a method and sample data to be processed.  Equilibrating instrument(s) for the method
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Preparing"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Preparing"; }
            };
            struct Waiting : public msm::front::state<> {
                // Instrument is waiting for inject trigger; This is actually a part of running state
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: WaitForContactClosure"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: WaitForContactClosure"; }
            };
            struct Running : public msm::front::state<> {
                // Method and sample is in progress
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Running"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Running"; }
            };
            struct Dormant : public msm::front::state<> {
                // Method and sample has been completed.  Keep instrument running at the last step of method
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Dormant"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Dormant"; }
            };

            void action( prepare const& ) {}
            void action( start_run const& ) {}
            void action( stop const& ) {}
            void action( inject const& ) {}

            typedef acquire_ p;
            // Transition table
            struct transition_table : mpl::vector<
                //    Start          Event         Next      Action				 Guard
                //  +---------+-------------+---------+---------------------+----------------------+
                // 0 = Empty
                a_row < Empty,       prepare,   Preparing, &p::action >
                // 1 = Preparing
                , a_row < Preparing, start_run, Waiting,   &p::action >
                , a_row < Preparing, stop,      Dormant,   &p::action >
                , a_row < Preparing, inject,    Running,   &p::action >
                // 2 = WaitingForContactClosure
                , a_row < Waiting,   inject,    Running,   &p::action >
                , a_row < Waiting,   stop,      Dormant,   &p::action >
                // 3 = Running
                , a_row < Running,   stop,      Dormant,   &p::action >
                // 4 = Dormant
                , a_row < Dormant,   stop,      Empty,     &p::action >
                , a_row < Dormant,   prepare,   Preparing, &p::action >
                , a_row < Dormant,   start_run, Waiting,   &p::action >
                > {};
            template<class FSM, class Event>
            void no_transition( Event const& e, FSM&, int state ) {
                ADDEBUG() << "no transition from state " << state << " on event " << typeid( e ).name();
            }
            typedef Empty initial_state;
            internal::MainWindow * mainWindow_;
        };
        
        // back-end
        typedef msm::back::state_machine< acquire_ > acquire;
        
    } // namespace fsm

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "acquire";
        }
    };

    class document::impl {
    public:
        impl() {
            fsm_.mainWindow_ = 0;
        }

        ~impl() {
        }

        inline fsm::acquire& fsm() { return fsm_; }

        void setMainWindow( internal::MainWindow * w ) {
            fsm_.mainWindow_ = w;
        }
    private:
        fsm::acquire fsm_;
    };

}

using namespace acquire;

std::atomic< document * > document::instance_(0);
std::mutex document::mutex_;

document::~document()
{
    delete impl_;
}

document::document(QObject *parent) : QObject(parent)
                                    , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                                , QLatin1String( "acquire" ) ) )
                                    , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
                                    , sampleRun_( std::make_shared< adcontrols::SampleRun >() )
                                    , impl_( new impl() )
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
    //Core::DocumentManager::setProjectsDirectory( path ); -- Quan plugin may set already
    Core::DocumentManager::setUseProjectsDirectory( true );

    boost::filesystem::path mfile( dir / "default.cmth" );
    adcontrols::ControlMethod::Method cm;
    if ( load( QString::fromStdWString( mfile.wstring() ), cm ) )
        setControlMethod( cm, QString() ); // don't save default name

    boost::filesystem::path sfile( dir / "samplerun.sequ" );
    adcontrols::SampleRun sr;
    if ( load( QString::fromStdWString( sfile.wstring() ), sr ) ) {
        //boost::filesystem::path path( sr.dataDirectory() );
        boost::filesystem::path path( adportable::profile::user_data_dir< char >() );
        path /= "data";
        path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
        sr.dataDirectory( path.normalize().wstring().c_str() );

        setSampleRun( sr, QString() ); // don't save default name
    }
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

std::shared_ptr< adcontrols::ControlMethod::Method >
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
document::setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        cm_ = std::make_shared< adcontrols::ControlMethod::Method >( m );
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
    sampleRun_ = std::make_shared< adcontrols::SampleRun >( m );

    if ( ! filename.isEmpty() ) {
        samplerun_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_SEQUENCE_FILES, constants::KEY_FILES, filename );
    }
    emit onSampleRunChanged( QString::fromWCharArray( sampleRun_->filePrefix() ), QString::fromWCharArray( sampleRun_->dataDirectory() ) );
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
document::load( const QString& filename, adcontrols::ControlMethod::Method& m )
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
                    QMessageBox::information( 0, "acquire -- Open default control method"
                                              , (boost::format( "Failed to open file: %1% by reason of %2% @ %3% #%4%" )
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
document::save( const QString& filename, const adcontrols::ControlMethod::Method& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }
    
    adfs::folder folder = file.addFolder( L"/ControlMethod" );
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::Method::dataClass() );
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
        adfile.dataClass( adcontrols::ControlMethod::Method::dataClass() );
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
document::fsmStart()
{
    impl_->fsm().start();
}

void
document::fsmStop()
{
    impl_->fsm().start();
}

void
document::fsmSetMainWindow( internal::MainWindow * w )
{
    impl_->setMainWindow( w );
}

void
document::fsmActPrepareForRun()
{
    adcontrols::SampleRun run;

    if ( impl_->fsm().mainWindow_ && impl_->fsm().mainWindow_->getSampleRun( run ) )
        setSampleRun( run, QString() );

    impl_->fsm().process_event( fsm::prepare() );
}

void
document::fsmActRun()
{
    impl_->fsm().process_event( fsm::start_run() );
}

void
document::fsmActInject()
{
    impl_->fsm().process_event( fsm::inject() );
}

void
document::fsmActStop()
{
    impl_->fsm().process_event( fsm::stop() );
}

// See also AcquirePlugin::handle_controller_message
void
document::notify_ready_for_run( const char * xml )
{
    if ( xml ) {
        std::wstring wxml( adportable::utf::to_wstring( xml ) );
        std::wistringstream is( wxml );

        adcontrols::SampleRun run;
        adcontrols::SampleRun::xml_restore( is, run );

        emit onSampleRunChanged( QString::fromWCharArray( run.filePrefix() ), QString::fromWCharArray( run.dataDirectory() ) );
        double length = run.methodTime();
        emit onSampleRunLength( QString( "%1 min" ).arg( QString::number( length / 60, 'f', 1 ) ) );
    }
}
