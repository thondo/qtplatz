/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "mode.hpp"
#include "constants.hpp"
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/ieditor.h>

using namespace sequence;

Mode::Mode(QObject *parent) :  Core::IMode(parent)
{
    setId( Constants::C_SEQUENCE_MODE );
    setContext( Core::Context( Constants::C_SEQUENCE_MODE ) );
    setDisplayName( tr( "Sequence" ) );
    setIcon(QIcon(":/sequence/images/DefineControls.png"));
    setPriority( 70 );
    
    connect( dynamic_cast<const Core::ModeManager *>(Core::ModeManager::instance()), &Core::ModeManager::currentModeChanged, this, &Mode::grabEditorManager );
}

Mode::~Mode()
{
    //Core::EditorManager::instance()->setParent( 0 );
}

void
Mode::grabEditorManager( Core::IMode * mode )
{
    if ( mode != this )
        return;

    if ( auto cmd = Core::ActionManager::instance()->command( Core::Constants::OPEN ) )
        cmd->action()->setText( tr( "Open Sequence..." ) );
}
