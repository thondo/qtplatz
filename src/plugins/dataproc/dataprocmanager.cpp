// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dataprocmanager.h"
#include "sessionmanager.h"
#include "dataprocessor.h"
#include <adportable/Configuration.h>
#include <adcontrols/datafilebroker.h>
#include <adcontrols/datafile.h>
#include <adplugin/adplugin.h>
#include <adplugin/lifecycle.h>
#include <qtwrapper/qstring.h>
#include <boost/noncopyable.hpp>

#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QtCore/QHash>
#include <QString>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>

#include <vector>

using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class DataprocManagerImpl : boost::noncopyable {
        public:
            ~DataprocManagerImpl();
            DataprocManagerImpl();

            Utils::FancyMainWindow * mainWindow_;
            void init();

        public:
            //std::vector< QDockWidget * > dockWidgetVec_;
    };
  }
}

DataprocManager::DataprocManager(QObject *parent) :
    QObject(parent)
    , pImpl_( new DataprocManagerImpl() )
{
}

QMainWindow *
DataprocManager::mainWindow() const
{
    return pImpl_->mainWindow_;
}

void
DataprocManager::init( const adportable::Configuration& config, const std::wstring& apppath )
{
    pImpl_->init();

    DataprocManagerImpl& m = *pImpl_;

    const adportable::Configuration * pTab = adportable::Configuration::find( config, L"ProcessMethodEditors" );
    if ( pTab ) {
        using namespace adportable;
        using namespace adplugin;
            
        // std::wstring loadpath = qtwrapper::wstring( dir.path() );
        // tab pages
        for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {

            const std::wstring name = it->name();

            if ( it->isPlugin() ) {
                QWidget * pWidget = manager::widget_factory( *it, apppath.c_str(), 0 );
                if ( pWidget ) {
                    // receive client event
                    connect( pWidget, SIGNAL( signalApplyMethod() ), this, SLOT( handleApplyMethod() ) );

                    // send to client
                    connect( this, SIGNAL( signalUpdateFile( adcontrols::datafile* ) ), pWidget, SLOT( handleUpdateFile( adcontrols::datafile* ) ) );

                    pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    m.mainWindow_->addDockForWidget( pWidget );
                }
            } else {
                QWidget * edit = new QTextEdit( "Edit" );
                edit->setWindowTitle( qtwrapper::qstring( it->title() ) );
                m.mainWindow_->addDockForWidget( edit );
            }
        }
    }       

    ////
    const adportable::Configuration * provider = adportable::Configuration::find( config, L"dataproviders" );
    if ( provider ) {
        using namespace adportable;
        using namespace adplugin;

        for ( Configuration::vector_type::const_iterator it = provider->begin(); it != provider->end(); ++it ) {
            const std::wstring name = apppath + it->module().library_filename();
            adcontrols::datafileBroker::register_library( name );
        }
    }
}

void
DataprocManager::OnInitialUpdate()
{
    DataprocManagerImpl& m = *pImpl_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle ) {
                pLifeCycle->OnInitialUpdate();
            }
        }
    }
}

void
DataprocManager::OnFinalClose()
{
    DataprocManagerImpl& m = *pImpl_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle ) {
                pLifeCycle->OnFinalClose();
            }
        }
    }
}

///////////////////////
DataprocManagerImpl::~DataprocManagerImpl()
{
}

DataprocManagerImpl::DataprocManagerImpl() : mainWindow_(0)
{
}

void
DataprocManagerImpl::init()
{
    mainWindow_ = new Utils::FancyMainWindow;
    if ( mainWindow_ ) {
        mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
    }
}

void
DataprocManager::setSimpleDockWidgetArrangement()
{
    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
    public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };

    DataprocManagerImpl& m = *pImpl_;
    setTrackingEnabled lock( *m.mainWindow_ );
  
    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating( false );
        m.mainWindow_->removeDockWidget( dockWidget );
    }

    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        m.mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
    }

    for ( int i = 1; i < dockWidgets.size(); ++i )
        m.mainWindow_->tabifyDockWidget( dockWidgets[0], dockWidgets[i] );
}

void
DataprocManager::handleSessionAdded( dataproc::Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    emit signalUpdateFile( &file );
}

void
DataprocManager::handleApplyMethod()
{
    // connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), pWidget, SLOT( handleSessionAdded( Dataprocessor* ) ) );
}

