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

#include "quanplugin.hpp"
#include "quanconstants.hpp"
#include "mainwindow.hpp"
#include "quanmode.hpp"
#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace quan;

QuanPlugin::QuanPlugin() : mainWindow_( new MainWindow() )
                         , mode_( std::make_shared< QuanMode >( this ) )
{
    // Create your members
}

QuanPlugin::~QuanPlugin()
{
	if ( mode_ )
        removeObject( mode_.get() );
    delete mainWindow_;
}

bool
QuanPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    mainWindow_->activateWindow();
    mainWindow_->createActions();

	const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;
    mode_->setContext( gc );

    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );

    addObject( mode_.get() );

    return true;
}

void QuanPlugin::extensionsInitialized()
{
    mainWindow_->onInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag QuanPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

Q_EXPORT_PLUGIN2(Quan, QuanPlugin)
