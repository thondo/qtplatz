// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "servantplugin.hpp"
#include "servantmode.hpp"
#include "outputwindow.hpp"
#include "servantpluginimpl.hpp"
#include <adbroker/orbbroker.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <extensionsystem/pluginmanager.h>

#include <acewrapper/constants.hpp>

#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrometer_factory.hpp>

#include <adplugin/loader.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbfactory.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/constants.hpp>

#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include <adportable/configloader.hpp>

#include <qtwrapper/qstring.hpp>
#include <qtwrapper/application.hpp>

#include <QMessageBox>
#include <QtCore/qplugin.h>
#include <QtCore>
#include <qdebug.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <chrono>
#include <thread>

using namespace servant;
using namespace servant::internal;

ServantPlugin * ServantPlugin::instance_ = 0;

ServantPlugin::~ServantPlugin()
{
    final_close();
    instance_ = 0;
}

ServantPlugin::ServantPlugin() 
{
    instance_ = this;
    // ACE::init();
}

ServantPlugin *
ServantPlugin::instance()
{
    return instance_;
}

bool
ServantPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);

    do { adportable::debug(__FILE__, __LINE__) << "<----- ServantPlugin::initialize() ..."; } while(0);

    OutputWindow * outputWindow = new OutputWindow;
    addAutoReleasedObject( outputWindow );
    //pImpl_ = new internal::ServantPluginImpl( outputWindow );

    ///////////////////////////////////
    Core::ICore * core = Core::ICore::instance();
    QList<int> context;
    if ( core ) {
        Core::UniqueIDManager * uidm = core->uniqueIDManager();
        if ( uidm ) {
            context.append( uidm->uniqueIdentifier( QLatin1String("Servant.MainView") ) );
            context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
        }
    } else
        return false;
    
	boost::filesystem::path plugindir;
    do {
        std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
		std::wstring configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/does-not-exist.adplugin" );
		plugindir = boost::filesystem::path( configFile ).branch_path();
    } while(0);

	// populate .adplugin files under given folder.
	adplugin::loader::populate( plugindir.generic_wstring().c_str() );

	std::vector< adplugin::plugin_ptr > spectrometers;
	if ( adplugin::loader::select_iids( ".*\\.adplugins\\.massSpectrometer\\..*", spectrometers ) ) {
		std::for_each( spectrometers.begin(), spectrometers.end(), []( const adplugin::plugin_ptr& d ){ 
			adcontrols::massspectrometer_factory * factory = d->query_interface< adcontrols::massspectrometer_factory >();
			if ( factory )
				adcontrols::massSpectrometerBroker::register_factory( factory, factory->name() );
		});
	}

	if ( adplugin::plugin_ptr adbroker_plugin = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" ) ) {

        adplugin::orbServant * adBroker = 0;
        if ( adbroker::orbBroker * orbBroker = adbroker_plugin->query_interface< adbroker::orbBroker >() ) {

            orbBroker->orbmgr_init( 0, 0 );

            adbroker::orbBroker& orbCreator = *orbBroker;

            try { 

                adBroker = orbBroker->create_instance();
                orbServants_.push_back( adBroker );

            } catch ( boost::exception& ex ) {
                *error_message = QString::fromStdString( boost::diagnostic_information( ex ) );
                return false;
            }

            // ----------------------- initialize corba servants ------------------------------
            std::vector< adplugin::plugin_ptr > factories;
            adplugin::loader::select_iids( ".*\\.adplugins\\.orbfactory\\..*", factories );
            for ( const adplugin::plugin_ptr& plugin: factories ) {
                
                if ( plugin->iid() == adbroker_plugin->iid() )
                    continue;
                
                if ( adplugin::orbServant * servant = orbCreator( plugin.get() ) ) {
                    orbServants_.push_back( servant );
                }
            }
        }
    }
    do { adportable::debug() << "----> ServantPlugin::initialize() completed."; } while(0);
    return true;
}

void
ServantPlugin::extensionsInitialized()
{
    do { adportable::debug(__FILE__, __LINE__) << "ServantPlugin::extensionsInitialized() entered."; } while(0);
    OutputWindow * outputWindow = ExtensionSystem::PluginManager::instance()->getObject< servant::OutputWindow >();
    if ( outputWindow )
        outputWindow->appendLog( L"ServantPlugin::extensionsInitialized()" );
}

ExtensionSystem::IPlugin::ShutdownFlag
ServantPlugin::aboutToShutdown()
{ 
	return SynchronousShutdown;
}

void
ServantPlugin::final_close()
{
    adportable::debug() << "====== ServantPlugin::final_close servants shutdown... =======";
    // destriction must be reverse order
    for ( orbservant_vector_type::reverse_iterator it = orbServants_.rbegin(); it != orbServants_.rend(); ++it )
        (*it)->deactivate();

	if ( adplugin::plugin_ptr adbroker_plugin = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" ) ) {
        if ( adbroker::orbBroker * orbBroker = adbroker_plugin->query_interface< adbroker::orbBroker >() ) {

            try {

                orbBroker->orbmgr_shutdown();
                orbBroker->orbmgr_fini();
                orbBroker->orbmgr_wait();

            } catch ( boost::exception& ex ) {
                ADDEBUG() << boost::diagnostic_information( ex );
            }
            
        }
    }

}

Q_EXPORT_PLUGIN( ServantPlugin )
