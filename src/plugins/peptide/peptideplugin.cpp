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

#include "peptideplugin.hpp"
#include "peptideconstants.hpp"
#include "mainwindow.hpp"
#include "peptidemode.hpp"

#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/id.h>
#include <coreplugin/modemanager.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#include <adportable/debug.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem/path.hpp>

using namespace peptide;

peptideplugin::peptideplugin() : mainWindow_( new MainWindow() )
                               , mode_( std::make_shared< PeptideMode >(this) )
{
    // Create your members
}

peptideplugin::~peptideplugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
	if ( mode_ )
        removeObject( mode_.get() );
    delete mainWindow_;
}

bool peptideplugin::initialize(const QStringList &arguments, QString *errorString)
{
    (void)arguments;
    (void)errorString;

    mainWindow_->activateWindow();
    mainWindow_->createActions();

    // Core::Context gc( (Core::Id( Core::Constants::C_GLOBAL )) );
    // mode_->setContext( gc );
    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;
}

void peptideplugin::extensionsInitialized()
{
    mainWindow_->onInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag peptideplugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)

#if ! defined NDEBUG
    ADDEBUG() << "\t## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif

    return SynchronousShutdown;
}

void
peptideplugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from peptide."));
}

Q_EXPORT_PLUGIN2(peptide, peptideplugin)

