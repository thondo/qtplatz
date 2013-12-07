/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "chemistryplugin.hpp"
#include "chemistryconstants.hpp"
#include "chemistrymode.hpp"
#include "mainwindow.hpp"
#include "constants.hpp"
#include <adportable/debug.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedWidget>
#else
#include <QtGui/QBoxLayout>
#include <QtGui/QStackedWidget>
#endif

#include <qtextedit.h>
#include <qwidget.h>

using namespace chemistry;

ChemistryPlugin::ChemistryPlugin() : mode_( std::make_shared< ChemistryMode >( this ) )
                                   , mainWindow_( std::make_shared< MainWindow >() )
{
    // Create your members
}

ChemistryPlugin::~ChemistryPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    if ( mode_ )
		removeObject( mode_.get() );
}

bool
ChemistryPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    QAction *action = new QAction(tr("chemistry action"), this);
	const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;
	Core::Command * cmd = Core::ICore::instance()->actionManager()->registerAction( action, Constants::ACTION_ID, gc );

    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

	Core::ActionContainer *menu = Core::ICore::instance()->actionManager()->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("Chemistry"));
    menu->addAction(cmd);
	Core::ICore::instance()->actionManager()->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->activateWindow();
    mainWindow_->createActions();

    mode_->setContext( gc );
    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;
}

void
ChemistryPlugin::extensionsInitialized()
{
	Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
ChemistryPlugin::aboutToShutdown()
{ 
	return SynchronousShutdown;
}

void
ChemistryPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}

Q_EXPORT_PLUGIN2(Chemistry, ChemistryPlugin)

