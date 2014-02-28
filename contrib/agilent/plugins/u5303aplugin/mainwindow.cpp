/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "mainwindow.hpp"
#include "waveformwnd.hpp"
#include "document.hpp"
#include "u5303a_constants.hpp"
#include "u5303amethodwidget.hpp"
#include <u5303a/digitizer.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adinterface/controlserver.hpp>
#include <adportable/serializer.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/imode.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>

#include <QDockWidget>
#include <QMenu>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QIcon>
#include <qdebug.h>

using namespace u5303a;

MainWindow * MainWindow::instance_ = 0;

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
{
    instance_ = this;
}

MainWindow::~MainWindow()
{
}

MainWindow *
MainWindow::instance()
{
    return instance_;
}

void
MainWindow::OnInitialUpdate()
{
    setSimpleDockWidgetArrangement();

    QList< QDockWidget *> widgets = dockWidgets();
    for ( auto w: widgets ) {
        if ( auto t = w->findChild< u5303AMethodWidget * >() )
            t->onInitialUpdate();
    }

    connect( document::instance(), SIGNAL( on_reply(const QString&, const QString&) ), this, SLOT( handle_reply( const QString&, const QString& ) ) );
    // connect( document::instance(), SIGNAL( on_waveform_received() ), this, SLOT( handle_waveform() ) );
    connect( document::instance(), SIGNAL( on_status(int) ), this, SLOT( handle_status(int) ) );
    for ( auto action: actions_ )
        action->setEnabled( false );
    actions_[ idActConnect ]->setEnabled( true );

	if ( WaveformWnd * wnd = centralWidget()->findChild<WaveformWnd *>() ) {
		wnd->onInitialUpdate();
        connect( document::instance(), SIGNAL( on_waveform_received() ), wnd, SLOT( handle_waveform() ) );
    }

}

void
MainWindow::activateLayout()
{
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::West );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );
	    
    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( editorHolderLayout );
        editorHolderLayout->addWidget( new QTextEdit() );

        Utils::StyledBar * toolBar1 = createTopStyledToolbar();
        Utils::StyledBar * toolBar2 = createMidStyledToolbar();
        
        //---------- central widget ------------
        QWidget * centralWidget = new QWidget;
        if ( centralWidget ) {
            setCentralWidget( centralWidget );
            
            QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
            // centralWidget->setLayout( centralLayout );
            centralLayout->setMargin( 0 );
            centralLayout->setSpacing( 0 );
            // ----------------- top tool bar -------------------
            centralLayout->addWidget( toolBar1 );              // [1]
            // ----------------------------------------------------
            centralLayout->addWidget( new WaveformWnd );
            // ----------------------------------------------------
            centralLayout->setStretch( 0, 1 );
            centralLayout->setStretch( 1, 0 );
            
            // ----------------- mid tool bar -------------------
            centralLayout->addWidget( toolBar2 );              // [1]
        }
    }

	// Right-side window with editor, output etc.
	Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
    if ( mainWindowSplitter ) {
        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
        outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );
        mainWindowSplitter->addWidget( this );
        mainWindowSplitter->addWidget( outputPane );
        mainWindowSplitter->setStretchFactor( 0, 10 );
        mainWindowSplitter->setStretchFactor( 1, 0 );
        mainWindowSplitter->setOrientation( Qt::Vertical );
    }

	// Navigation and right-side window
	Core::MiniSplitter * splitter = new Core::MiniSplitter;               // entier this view
    if ( splitter ) {
        splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
        splitter->addWidget( mainWindowSplitter );                            // *this + ontput
        splitter->setStretchFactor( 0, 0 );
        splitter->setStretchFactor( 1, 1 );
        splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );
    }

    createDockWidgets();

	return splitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    qtwrapper::TrackingEnabled< Utils::FancyMainWindow > x( *this );

    QList< QDockWidget *> widgets = dockWidgets();

    for ( auto widget: widgets ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }
  
    size_t npos = 0;
    for ( auto widget: widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos && npos++ < widgets.size() - 1 ) // last item is not on the tab
            tabifyDockWidget( widgets[0], widget );
    }
    // update();
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& page )
{
    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( page.isEmpty() ? widget->objectName() : page );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
}

void
MainWindow::createDockWidgets()
{
    createDockWidget( new u5303AMethodWidget(), "U5303A Parameter", "U5303AMethodWidget" );
    createDockWidget( new QTextEdit(), "Log", "Log" );
}

// static
QToolButton * 
MainWindow::toolButton( QAction * action )
{
    QToolButton * button = new QToolButton;
    if ( button )
        button->setDefaultAction( action );
    return button;
}

// static
QToolButton * 
MainWindow::toolButton( const char * id )
{
    Core::ActionManager * mgr = Core::ICore::instance()->actionManager();
    return toolButton( mgr->command(id)->action() );
}

void
MainWindow::setData( const adcontrols::MassSpectrum& ms )
{
    // if ( monitorView_ )
    //     monitorView_->setData( ms );
}

void
MainWindow::setData( const adcontrols::Trace& trace, const std::wstring& traceId )
{
    // if ( monitorView_ )
    //     monitorView_->setData( trace, traceId );
}

Utils::StyledBar *
MainWindow::createTopStyledToolbar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_CONNECT)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_INITRUN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_RUN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_STOP)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_INJECT)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_SNAPSHOT)->action()));
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            //toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledToolbar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            // print, method file open & save buttons
            //toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            //toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_OPEN)->action()));
            // [file open] button
            toolBarLayout->addWidget(toolButton(am->command(Constants::FILE_OPEN)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            QList<int> context;
            context << Core::Constants::C_GLOBAL_ID;
            
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::createActions()
{
    // enum idActions { idActConnect, idActInitRun, idActRun, idActStop, idActSnapshot, idActInject, idActFileOpen, numActions };

    actions_[ idActConnect ] = createAction( Constants::ICON_CONNECT,  tr("Connect"), this );    
    actions_[ idActInitRun ] = createAction( Constants::ICON_INITRUN,  tr("Initial run"), this );    
    actions_[ idActRun ]     = createAction( Constants::ICON_RUN,      tr("Run"), this );    
    actions_[ idActStop ]    = createAction( Constants::ICON_STOP,     tr("Stop"), this );    
    actions_[ idActSnapshot ]= createAction( Constants::ICON_SNAPSHOT, tr("Snapshot"), this );    
    actions_[ idActInject ]  = createAction( Constants::ICON_INJECT,   tr("INJECT"), this );
    actions_[ idActFileOpen ]= createAction( Constants::ICON_FILE_OPEN,tr("Open protain file..."), this );
    connect( actions_[ idActConnect ], SIGNAL( triggered() ), this, SLOT( actConnect() ) );
    connect( actions_[ idActInitRun ], SIGNAL( triggered() ), this, SLOT( actInitRun() ) );
    connect( actions_[ idActRun ], SIGNAL( triggered() ), this, SLOT( actRun() ) );
    connect( actions_[ idActStop ], SIGNAL( triggered() ), this, SLOT( actStop() ) );
    connect( actions_[ idActInject ], SIGNAL( triggered() ), this, SLOT( actInject() ) );
    connect( actions_[ idActSnapshot ], SIGNAL( triggered() ), this, SLOT( actSnapshot() ) );
    connect( actions_[ idActFileOpen ], SIGNAL( triggered() ), this, SLOT( actFileOpen() ) );

    const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;

    if ( Core::ActionManager * am = Core::ICore::instance()->actionManager() ) {

        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( "U5303A" );

        Core::Command * cmd = 0;
        cmd = am->registerAction( actions_[ idActConnect ], Constants::ACT_CONNECT, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActInitRun ], Constants::ACT_INITRUN, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActRun ], Constants::ACT_RUN, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActStop ], Constants::ACT_STOP, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActInject ], Constants::ACT_INJECT, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActSnapshot ], Constants::ACT_SNAPSHOT, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActFileOpen ], Constants::FILE_OPEN, gc );
        menu->addAction( cmd );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

QAction *
MainWindow::createAction( const QString& iconname, const QString& msg, QObject * parent )
{
    QIcon icon;
    icon.addFile( iconname );
    return new QAction( icon, msg, parent );
}

void
MainWindow::actConnect()
{
    document::instance()->u5303a_connect();
}

void
MainWindow::actInitRun()
{
    document::instance()->prepare_for_run();
}

void
MainWindow::actRun()
{
    document::instance()->u5303a_start_run();
}

void
MainWindow::actStop()
{
    document::instance()->u5303a_stop();
}

void
MainWindow::actSnapshot()
{
    if ( auto waveform = document::instance()->findWaveform() ) {
        adcontrols::MassSpectrum ms;
        if ( document::toMassSpectrum( ms, *waveform ) ) {

            boost::filesystem::path path( adportable::profile::user_data_dir<char>() );
            path /= "data";
            path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
            if ( ! boost::filesystem::exists( path ) ) {
                boost::system::error_code ec;
                boost::filesystem::create_directories( path, ec );
            }
            path /= "u5303a.adfs";
            std::wstring title = ( boost::wformat( L"Spectrum %1%" ) % waveform->serialnumber_ ).str();
            std::wstring folderId;
			if ( document::appendOnFile( path.wstring(), title, ms, folderId ) ) {
                auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                for ( auto handler: vec )
                    handler->folium_added( path.string().c_str(), "/Processed/Spectra", QString::fromStdWString( folderId ) );
            }
        }
    }
}

void
MainWindow::actInject()
{
    document::instance()->u5303a_trigger_inject();
}

void
MainWindow::actFileOpen()
{
}

void
MainWindow::handle_reply( const QString& method, const QString& reply )
{
	auto docks = dockWidgets();
    auto it = std::find_if( docks.begin(), docks.end(), []( const QDockWidget * w ){ return w->objectName() == "Log"; });
    if ( it != docks.end() ) {
		if ( auto edit = dynamic_cast< QTextEdit * >( (*it)->widget() ) )
			edit->append( QString("%1: %2").arg( method, reply ) );
	}
}

void
MainWindow::handle_status( int status )
{
    if ( status == controlserver::eStandBy ) {
        for ( auto action: actions_ )
            action->setEnabled( true );
        actions_[ idActConnect ]->setEnabled( false );
        actInitRun();
        if ( auto mw = findChild< u5303AMethodWidget * >() )
            mw->onStatus( status );
    }
}
