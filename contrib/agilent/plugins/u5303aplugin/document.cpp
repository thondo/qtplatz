/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "constants.hpp"
#include "mainwindow.hpp"
#include "resultwriter.hpp"
#include "task.hpp"
#include "u5303a_constants.hpp"
#include "icontrollerimpl.hpp"
#include <acqrscontrols/u5303a/histogram.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/metadata.hpp>
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/timedigitalmethod.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adextension/icontrollerimpl.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adicontroller/masterobserver.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <extensionsystem/pluginmanager.h>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QMetaType>
#include <chrono>
#include <future>
#include <string>
#include <fstream>


Q_DECLARE_METATYPE( boost::uuids::uuid );

using namespace u5303a;

namespace u5303a {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "u5303a";
        }
    };

    namespace so = adicontroller::SignalObserver;

    //..........................................
    class document::impl {
    public:
        static document * instance_; // workaround
        static std::mutex mutex_;
        static const std::chrono::steady_clock::time_point uptime_;
        static const uint64_t tp0_;

        std::shared_ptr< acqrscontrols::u5303a::tdcdoc > tdcdoc_;
        std::shared_ptr< adcontrols::SampleRun > nextSampleRun_;
        std::shared_ptr< ::u5303a::iControllerImpl > iControllerImpl_;
        std::shared_ptr< adextension::iSequenceImpl > iSequenceImpl_;
        std::vector< std::shared_ptr< adextension::iController > > iControllers_;
        std::vector< std::shared_ptr< adextension::iController > > activeControllers_;
        std::shared_ptr< adicontroller::MasterObserver > masterObserver_;
        bool isMethodDirty_;

        std::deque< std::shared_ptr< const acqrscontrols::u5303a::waveform > > que_;
        std::shared_ptr< adcontrols::ControlMethod::Method > cm_;
        std::shared_ptr< acqrscontrols::u5303a::method > method_;
        std::shared_ptr< adcontrols::TimeDigitalMethod > tdm_;
        std::shared_ptr< ResultWriter > resultWriter_;

        int32_t device_status_;
        double triggers_per_second_;
        
        std::shared_ptr< QSettings > settings_;  // user scope settings
        QString ctrlmethod_filename_;

        std::map< QString, bool > moduleStates_;

        // display data
        std::map< boost::uuids::uuid, std::array< std::shared_ptr< adcontrols::MassSpectrum >, acqrscontrols::u5303a::nchannels > > spectra_;

        impl() : tdcdoc_( std::make_shared< acqrscontrols::u5303a::tdcdoc >() )
               , nextSampleRun_( std::make_shared< adcontrols::SampleRun >() )
               , iControllerImpl_( std::make_shared< u5303a::iControllerImpl >() )
               , iSequenceImpl_( std::make_shared< adextension::iSequenceImpl >( "U5303A" ) )
               , isMethodDirty_( true )
               , device_status_( 0 )
               , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
               , method_( std::make_shared< acqrscontrols::u5303a::method >() )
               , tdm_( std::make_shared< adcontrols::TimeDigitalMethod>() )
               , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                           , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                           , QLatin1String( "u5303a" ) ) )
               , triggers_per_second_(0)
               , resultWriter_( std::make_shared< ResultWriter >() )  {
            
        }

        void addInstController( std::shared_ptr< adextension::iController > p );
        void handleConnected( adextension::iController * controller );
        void handleLog( adextension::iController *, const QString& );
        void handleDataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos );
        void setControllerState( const QString&, bool enable );
        void loadControllerState();
        void takeSnapshot();
        std::shared_ptr< adcontrols::MassSpectrum > getHistogram( double resolution ) const;
    };

    std::mutex document::impl::mutex_;
    document * document::impl::instance_( 0 );
    const std::chrono::steady_clock::time_point document::impl::uptime_ = std::chrono::steady_clock::now();
    const uint64_t document::impl::tp0_ = std::chrono::duration_cast<std::chrono::nanoseconds>( document::impl::uptime_.time_since_epoch() ).count();
}
    
document::document() : impl_( new impl() )
{
    // diagnostic
    QVariant v;
    v.setValue( boost::uuids::uuid() );
}

document::~document()
{
    delete impl_;
    ADDEBUG() << "=====> document dtor";    
}

document *
document::instance()
{
    static std::once_flag flag;

    std::call_once( flag, [=] () { impl::instance_ = new document(); } );
    return impl::instance_;
}

void
document::actionConnect()
{
    if ( !impl_->iControllers_.empty() ) {

        std::vector< std::future<bool> > futures;

        std::vector< std::shared_ptr< adextension::iController > > activeControllers;

        for ( auto& iController : impl_->iControllers_ ) {

            if ( isControllerEnabled( iController->module_name() ) ) {

                ADDEBUG() << "u5303a actionConnect connecting to " << iController->module_name().toStdString();

                activeControllers.push_back( iController );
                
                futures.push_back( std::async( [iController] () {
                            return iController->connect() && iController->wait_for_connection_ready(); } ) );

            }
        }

        size_t i = 0;
        for ( auto& future : futures ) {
            if ( future.get() )
                impl_->activeControllers_.push_back( activeControllers[ i ] );
            ++i;
        }

        auto cm = MainWindow::instance()->getControlMethod();
        setControlMethod( *cm, QString() );
        tdc()->set_threshold_method( 0, impl_->tdm_->threshold( 0 ) );

        futures.clear();
        for ( auto& iController : impl_->iControllers_ ) {
            if ( auto session = iController->getInstrumentSession() ) {
                session->initialize();
                session->prepare_for_run( cm );
            }
        }

        // task::instance()->post( futures );

        // setup observer hiralchey
        impl_->masterObserver_ = std::make_shared< adicontroller::MasterObserver >();
        for ( auto& iController : impl_->iControllers_ ) {
            if ( auto session = iController->getInstrumentSession() ) {
                if ( auto observer = session->getObserver() )
                    impl_->masterObserver_->addSibling( observer );
            }
        }
        
        task::instance()->prepare_next_sample( impl_->masterObserver_.get(), impl_->nextSampleRun_, *cm );
    }
}

void
document::actionRec( bool onoff )
{
    task::instance()->setRecording( onoff );

    for ( auto inst : impl_->iControllers_ ) {
        if ( auto session = inst->getInstrumentSession() )
            session->recording( onoff );
    }
}

void
document::actionSyncTrig()
{
    tdc()->clear_histogram();
}

void
document::actionRun( bool run )
{
    ADDEBUG() << "### actionRun " << run << " ###";
    for ( auto inst : impl_->iControllers_ ) {
        if ( auto session = inst->getInstrumentSession() ) {
            if ( run )
                session->start_run();
            else
                session->stop_run();
        }
    }
}

void
document::addInstController( adextension::iController * p )
{
    try {

        if ( auto ptr = p->pThis() )
            impl_->addInstController( p->pThis() );

    } catch ( std::bad_weak_ptr& ) {
        
        QMessageBox::warning( MainWindow::instance(), "U5303A plugin"
                              , QString( tr( "Instrument controller %1 has no shared_ptr; ignored." ) ).arg( p->module_name() ) );

    }
}

void
document::impl::addInstController( std::shared_ptr< adextension::iController > p )
{

    using adextension::iController;
    using adicontroller::SignalObserver::Observer;

    if ( p->module_name() == "u5303a" ) { // handle only this device

        for ( auto it = iControllers_.begin(); it != iControllers_.end(); ++it )
            if ( ( *it )->module_name() == "u5303a" )
                return;

        iControllers_.push_back( p );

        // switch to UI thread
        connect( p.get(), &iController::message, document::instance()
                 , [] ( iController * p, unsigned int code, unsigned int value ) { document::instance()->handleMessage( p, code, value ); } );

        // non UI thread
        connect( p.get(), &iController::connected, [this] ( iController * p ) { handleConnected( p ); } );
        connect( p.get(), &iController::log, [this] ( iController * p, const QString& log ) { handleLog( p, log ); } );
        
        p->dataChangedHandler( [] ( Observer *o, unsigned int pos ) { task::instance()->onDataChanged( o, pos ); } );
    }

}

void
document::prepare_for_run()
{
    using adcontrols::ControlMethod::MethodItem;

    auto cm = MainWindow::instance()->getControlMethod();
    setControlMethod( *cm, QString() );
    impl_->isMethodDirty_ = false;

    ADDEBUG() << "### prepare_for_run ###";
    
    std::vector< std::future< bool > > futures;
    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            futures.push_back( std::async( [=] () { return session->prepare_for_run( cm ); } ) );
        }
    }
    if ( !futures.empty() )
        task::instance()->post( futures );
}

void
document::start_run()
{
    ADDEBUG() << "### start run ###";    
    prepare_for_run();
}

void
document::stop()
{
    ADDEBUG() << "### stop ###";

    std::vector< std::future< bool > > futures;
    
    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            futures.push_back( std::async( [=] () { return session->stop_run(); } ) );
        }
    }
    if ( !futures.empty() )
        task::instance()->post( futures );
    
}

int32_t
document::device_status() const
{
    return impl_->device_status_;
}

std::shared_ptr< const acqrscontrols::u5303a::method >
document::method() const
{
    return impl_->method_;
}

// static
bool
document::appendOnFile( const boost::filesystem::path& path
                        , const QString& title
                        , const adcontrols::MassSpectrum& ms
                        , QString& id )
{
    adfs::filesystem fs;
	
	if ( ! boost::filesystem::exists( path ) ) {
		if ( ! fs.create( path.c_str() ) )
			return false;
	} else {
		if ( ! fs.mount( path.c_str() ) )
			return false;
	}
	adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );

    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), title.toStdWString() );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = QString::fromStdWString( file.id() );
            if ( file.save( ms ) )
				file.commit();
        }
	}
    return true;
    
}

void
document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "u5303a::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( Constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }

    do {
        boost::filesystem::path mfile( dir / "u5303a.cmth.xml" );
        acqrscontrols::u5303a::method m;
        if ( load( QString::fromStdWString( mfile.wstring() ), m ) )
            *impl_->method_ = m;

        if ( impl_->cm_ ) {
            impl_->cm_->append<>( m );
        }

    } while ( 0 );

    impl_->loadControllerState();

}

void
document::finalClose()
{
    ADTRACE() << "=====> document::finalClose()";

    task::instance()->finalize();

    if ( auto session = impl_->iControllerImpl_->getInstrumentSession() )
        session->shutdown();

    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "u5303a::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }

    auto cm = MainWindow::instance()->getControlMethod();
    auto it = cm->find( cm->begin(), cm->end(), acqrscontrols::u5303a::method::clsid() );
    if ( it != cm->end() ) {
        acqrscontrols::u5303a::method x;
        if ( it->get<>( *it, x ) ) {
            boost::filesystem::path fname( dir / "u5303a.cmth.xml" );
            save( QString::fromStdWString( fname.wstring() ), x );
        }
        impl_->cm_ = cm;
    }

    if ( auto settings = impl_->settings_ ) {
        settings->beginGroup( "u5303a" );
        
        settings->beginWriteArray( "ControlModule" );

        int i = 0;
        for ( auto& state : impl_->moduleStates_ ) {
            settings->setArrayIndex( i++ );
            settings->setValue( "module_name", state.first );
            settings->setValue( "enable", state.second );
        }

        settings->endArray();
        settings->endGroup();
    }
    ADTRACE() << "=====> document finalClose completed.";
}

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings( *impl_->settings_ ).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, filename );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = Constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *impl_->settings_ ).recentFile( group, Constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *impl_->settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );

        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

bool
document::load( const QString& filename, acqrscontrols::u5303a::method& m )
{
    try {

        std::wifstream inf( filename.toStdString() );
        boost::archive::xml_wiarchive ar( inf );
        
        ar >> boost::serialization::make_nvp( "u5303a_method", m );

        return true;

    } catch( ... ) {
        std::cout << "############# acqrscontrols::u5303a::method load failed" << std::endl;
    }
    return false;
}

bool
document::save( const QString& filename, const acqrscontrols::u5303a::method& m )
{
    std::wofstream outf( filename.toStdString() );

    boost::archive::xml_woarchive ar( outf );
    ar << boost::serialization::make_nvp( "u5303a_method", m );
    return true;
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

#if 0 // adcontrols can't archive into xml format
    QFileInfo xmlfile( filename + ".xml" );
    if ( xmlfile.exists() )
        QFile::remove( xmlfile.absoluteFilePath() );

    std::wstringstream o;
    try {
        adcontrols::ControlMethod::xml_archive( o, m );
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
    }
    pugi::xml_document doc;
    doc.load( o );
    doc.save_file( xmlfile.absoluteFilePath().toStdString().c_str() );
#endif

    return true;
}

std::shared_ptr< adcontrols::ControlMethod::Method >
document::controlMethod() const
{
    return impl_->cm_;
}

void
document::setControlMethod( std::shared_ptr< adcontrols::ControlMethod::Method > ptr, const QString& filename )
{
    do {
        auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::u5303a::method::clsid() );
        if ( it != ptr->end() ) {
            acqrscontrols::u5303a::method m;
            if ( it->get( *it, m ) ) {
                *impl_->method_ = m;
            }
            impl_->cm_ = ptr;
        }
    } while(0);

    do {
        auto it = ptr->find( ptr->begin(), ptr->end(), adcontrols::TimeDigitalMethod::clsid() );
        if ( it != ptr->end() ) {
            adcontrols::TimeDigitalMethod tdm;
            if ( it->get( *it, tdm ) )
                *impl_->tdm_;
        }
    } while ( 0 );

    if ( ! filename.isEmpty() ) {
        impl_->ctrlmethod_filename_ = filename;
        qtwrapper::settings( *impl_->settings_ ).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

void
document::setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename )
{
    auto ptr = std::make_shared< adcontrols::ControlMethod::Method >( m );
    setControlMethod( ptr, filename );
}

double
document::triggers_per_second() const
{
    return impl_->triggers_per_second_;
}

size_t
document::unprocessed_trigger_counts() const
{
    return 0; //impl_->unprocessed_trigger_counts();
}

void
document::save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist )
{
#if 0
    std::ofstream of( impl_->hist_datafile(), std::ios_base::out | std::ios_base::app );

    const double * times = hist.getTimeArray();
    const double * intens = hist.getIntensityArray();
    const adcontrols::MSProperty& prop = hist.getMSProperty();

    of << boost::format("\n%d, %.8lf, %.14le") % tickCount % prop.timeSinceInjection() % prop.instTimeRange().first;
    for ( size_t i = 0; i < hist.size(); ++i )
        of << boost::format(", %.14le, %d" ) % times[ i ] % uint32_t( intens[ i ] );
#endif
}

void
document::set_threshold_method( int ch, const adcontrols::threshold_method& m )
{
    tdc()->set_threshold_method( ch, m );

    auto mp = MainWindow::instance()->getControlMethod();
    setControlMethod( mp );

    emit on_threshold_method_changed( ch );
}

void
document::set_method( const acqrscontrols::u5303a::method& )
{
    if ( auto cm = MainWindow::instance()->getControlMethod() ) {
        setControlMethod( cm );

        auto it = std::find_if( impl_->iControllers_.begin(), impl_->iControllers_.end(), [] ( std::shared_ptr<adextension::iController> ic ) {
                return ic->module_name() == "u5303a";
            } );
        if ( it != impl_->iControllers_.end() ) {
            if ( auto session = ( *it )->getInstrumentSession() )
                session->prepare_for_run( cm );
        }
    }
}

const adcontrols::SampleRun *
document::sampleRun() const
{
    return impl_->nextSampleRun_.get();
}

void
document::setSampleRun( std::shared_ptr< adcontrols::SampleRun > sr )
{
    impl_->nextSampleRun_ = sr;
}

bool
document::isRecording() const
{
    return task::instance()->isRecording();
}

void
document::impl::handleConnected( adextension::iController * controller )
{
    ADDEBUG() << controller->module_name().toStdString();
    task::instance()->initialize();
}

void
document::handleMessage( adextension::iController * ic, uint32_t code, uint32_t value )
{
    if ( code == adicontroller::Receiver::CLIENT_ATTACHED ) {

        emit document::instance()->instStateChanged( int( adicontroller::Instrument::eStandBy ) ); // --> enable FSM UI

    } else if ( code == adicontroller::Receiver::STATE_CHANGED ) {

        if ( value & adicontroller::Instrument::eErrorFlag ) {
            QMessageBox::warning( MainWindow::instance(), "U5303A Error"
                                  , QString( "Module %1 error with code %2" ).arg( ic->module_name(), QString::number( value, 16 ) ) );
        } else {

            emit document::instance()->instStateChanged( value );
            
            if ( value == adicontroller::Instrument::eStandBy && impl_->cm_ && ic ) {
                if ( auto session = ic->getInstrumentSession() )
                    session->prepare_for_run( impl_->cm_ );
            }
        }
    }
}

void
document::impl::handleLog( adextension::iController *, const QString& )
{
}

void
document::impl::handleDataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos )
{
}

acqrscontrols::u5303a::tdcdoc *
document::tdc()
{
    return impl_->tdcdoc_.get();
}

void
document::setData( const boost::uuids::uuid& objid, std::shared_ptr< adcontrols::MassSpectrum > ms, unsigned idx )
{
    assert( idx < acqrscontrols::u5303a::nchannels );

    if ( idx >= acqrscontrols::u5303a::nchannels )
        return;

    do {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );    
        impl_->spectra_[ objid ][ idx ] = ms;
    } while( 0 );

    emit dataChanged( objid, idx );
    
    if ( objid == u5303a_observer ) {
        if ( auto hgrm = tdc()->longTermHistogram() ) {

            double resolution = 0;
            if ( auto tm = tdc()->threshold_method( idx ) )
                resolution = tm->time_resolution;

            if ( resolution > hgrm->xIncrement() ) {
                std::vector< std::pair<double, uint32_t > > time_merged;
                adcontrols::TimeDigitalHistogram::average_time( hgrm->histogram(), resolution, time_merged );
                hgrm = hgrm->clone( time_merged );
            }

            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            adcontrols::TimeDigitalHistogram::translate( *ms, *hgrm );

            impl_->triggers_per_second_ = hgrm->triggers_per_second();
            
            impl_->spectra_ [ histogram_observer ] [ idx ] = ms; // histogram
            
            emit dataChanged( histogram_observer, idx );
        }
    }
}


void
document::commitData()
{
    // save time data
    impl_->resultWriter_->commitData();

#if 0
    // save histogram
    double resolution = 0;
    const int channel = 0;
    if ( auto tm = tdc()->threshold_method( channel ) ) // CH-1 
        resolution = tm->time_resolution;

    size_t trigCount(0);
    std::pair< uint64_t, uint64_t > timeSinceEpoch( 0,0 );
    
    if ( auto histogram = tdc()->getHistogram( resolution, channel, trigCount, timeSinceEpoch ) ) {

        impl_->resultWriter_->writeHistogram( trigCount, timeSinceEpoch, histogram );
    
    }
#endif
}

std::shared_ptr< adcontrols::MassSpectrum >
document::recentSpectrum( const boost::uuids::uuid& uuid, int idx )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );    

    auto it = impl_->spectra_.find( uuid );
    if ( it != impl_->spectra_.end() ) {
        if ( it->second.size() > idx )
            return it->second.at( idx );
    }
    return 0;
}

QSettings *
document::settings()
{
    return impl_->settings_.get();
}

void
document::takeSnapshot()
{
    std::vector< std::future<bool> > futures;
    futures.push_back( std::async( std::launch::async, [this](){ impl_->takeSnapshot(); return true; } ) );
    task::instance()->post( futures );
}

std::shared_ptr< adcontrols::MassSpectrum >
document::impl::getHistogram( double resolution ) const
{
    if ( auto hgrm = tdcdoc_->longTermHistogram() ) {

        double resolution = 0;
        if ( auto tm = tdcdoc_->threshold_method( 0 ) )
            resolution = tm->time_resolution;
        
        if ( resolution > hgrm->xIncrement() ) {
            std::vector< std::pair<double, uint32_t > > time_merged;
            adcontrols::TimeDigitalHistogram::average_time( hgrm->histogram(), resolution, time_merged );
            hgrm = hgrm->clone( time_merged );
        }

        auto ms = std::make_shared< adcontrols::MassSpectrum >();
        adcontrols::TimeDigitalHistogram::translate( *ms, *hgrm );
        return ms;
    }
    return nullptr;
}

void
document::impl::takeSnapshot()
{
    boost::filesystem::path dir( nextSampleRun_->dataDirectory() );
    boost::filesystem::path file( std::wstring( nextSampleRun_->filePrefix() ) + L".adfs~" );

    // debug -->
    resultWriter_->dump_waveform();
    // <-- debug
    
    if ( ! boost::filesystem::exists( dir ) ) {
        boost::system::error_code ec;
        boost::filesystem::create_directories( dir, ec );
    }
    
    boost::filesystem::path path( dir / file );
    if ( ! boost::filesystem::exists( path ) )
        path = dir / ( std::wstring( nextSampleRun_->filePrefix() ) + L"_snapshots.adfs" );
    
    unsigned idx = 0;

    // get histogram
    double resolution = 0.0;
    if ( auto tm = tdcdoc_->threshold_method( idx ) )
        resolution = tm->time_resolution;

    //size_t trigCount;
    //std::pair< uint64_t, uint64_t > timeSinceEpoch;
    auto histogram = getHistogram( resolution ); // , , trigCount, timeSinceEpoch );

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now(); 
    std::string date = adportable::date_string::logformat( tp );

    uint32_t serialnumber(0);
    

    // get waveform(s)
    auto spectra = spectra_[ u5303a_observer ];
    
    int ch = 1;
    for ( auto ms: spectra ) {
        if ( ms ) {
            serialnumber = ms->getMSProperty().trigNumber();
            QString title = QString( "Spectrum %1 CH-%2" ).arg( QString::fromStdString( date ), QString::number( ch ) );
            QString folderId;
            if ( appendOnFile( path, title, *ms, folderId ) ) {
                auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                for ( auto handler: vec )
                    handler->folium_added( path.string().c_str(), "/Processed/Spectra", folderId );
            }
        }
        ++ch;
    }

    // save histogram
    ch = 1;
    if ( histogram ) {
        QString title = QString( "Histogram %1 CH-%2" ).arg( QString::fromStdString( date ), QString::number( ch ) );
        QString folderId;
        if ( document::appendOnFile( path, title, *histogram, folderId ) ) {
            auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
            for ( auto handler: vec )
                handler->folium_added( path.string().c_str(), "/Processed/Spectra", folderId );
        }
    }

}

u5303a::iControllerImpl *
document::iController()
{
    return impl_->iControllerImpl_.get();
}

adextension::iSequenceImpl *
document::iSequence()
{
    return impl_->iSequenceImpl_.get();
}

void
document::result_to_file( std::shared_ptr< acqrscontrols::u5303a::threshold_result > ch1 )
{
    *( impl_->resultWriter_ ) << ch1;
}

void
document::setControllerState( const QString& module, bool enable )
{
    impl_->setControllerState( module, enable );
}

bool
document::isControllerEnabled( const QString& module ) const
{
    auto it = impl_->moduleStates_.find( module );

    if ( it != impl_->moduleStates_.end() )
        return it->second;

    return false;
}

void
document::impl::setControllerState( const QString& module, bool enable )
{
    moduleStates_[ module ] = enable;

    // update settings
    settings_->beginGroup( "u5303a" );
        
    settings_->beginWriteArray( "Controllers" );

    int i = 0;
    for ( auto& state : moduleStates_ ) {
        settings_->setArrayIndex( i++ );
        settings_->setValue( "module_name", state.first );
        settings_->setValue( "enable", state.second );
    }

    settings_->endArray();
    settings_->endGroup();
}

void
document::impl::loadControllerState()
{
    settings_->beginGroup( "u5303a" );
        
    int size = settings_->beginReadArray( "Controllers" );
    
    for ( int i = 0; i < size; ++i ) {
        settings_->setArrayIndex( i );
        QString module_name = settings_->value("module_name").toString();
        bool enable = settings_->value( "enable" ).toBool();
        moduleStates_[ module_name ] = enable;
    }
    
    settings_->endArray();
    settings_->endGroup();

    auto it = this->moduleStates_.find( "Acquire" );
    if ( it == moduleStates_.end() )
        moduleStates_[ "Acquire" ] = true;

    setControllerState( "u5303a", true );
}
