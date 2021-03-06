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

#include "dataproceditor.hpp"
#include "document.hpp"
#include "dataprocessor.hpp"
#include "dataprocessorfactory.hpp"
#include "dataprocconstants.hpp"
#include "mainwindow.hpp"
#include "msprocessingwnd.hpp"
#include "sessionmanager.hpp"
#include <coreplugin/id.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <qtwrapper/waitcursor.hpp>
#include <QTextEdit>
#include <QEvent>

using namespace dataproc;

DataprocEditor::DataprocEditor( Core::IEditorFactory * factory ) : Core::IEditor( 0 )
                                                                 , widget_( new QWidget )
                                                                 , factory_(factory)
{
    widget_->installEventFilter( this );
    setWidget( widget_ );
    context_.add( Constants::C_DATAPROCESSOR );
}

DataprocEditor::~DataprocEditor()
{
    SessionManager::instance()->removeEditor( this );
    delete widget_;
}

void
DataprocEditor::setDataprocessor( Dataprocessor * processor )
{
    processor_ = std::static_pointer_cast< Dataprocessor >( processor->shared_from_this() );
}

bool
DataprocEditor::portfolio_create( const QString& filename )
{
    if ( processor_ && processor_->create( filename ) ) {
        SessionManager::instance()->addDataprocessor( processor_, this );
        return true;
    }
    return false;
}

bool
DataprocEditor::open( QString* errorMessage, const QString &filename, const QString& )
{
	qtwrapper::waitCursor wait;

    QString emsg;
    if ( processor_ && processor_->open( filename,  emsg) ) {
        SessionManager::instance()->addDataprocessor( processor_, this );

        Core::DocumentManager::addDocument( processor_->document() );
        Core::DocumentManager::addToRecentFiles( filename );
        document::instance()->addToRecentFiles( filename );

        return true;
    }
    *errorMessage = QString( "DataprocEditor:\nfile %1 could not be opend.\nReason: %2." ).arg( filename, emsg );

    return false;
}

Core::IDocument *
DataprocEditor::document()
{
    return processor_ ? processor_->document() : 0;
}

void
DataprocEditor::handleTitleChanged( const QString & /* title */ )
{
}

QByteArray
DataprocEditor::saveState() const
{
    return QByteArray();
}

bool
DataprocEditor::restoreState(const QByteArray & /* state */ )
{
    return true;
}

QWidget *
DataprocEditor::toolBar()
{
    return 0;
}

Core::Context
DataprocEditor::context() const
{
    return context_;
}

bool
DataprocEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == widget_ ) {
        if ( event->type() == QEvent::ShowToParent ) {
            int mode(0);
            if ( processor_ && (mode = processor_->mode() ) )
                Core::ModeManager::activateMode( Core::Id( mode ) ); // Constants::C_DATAPROCESSOR ) );
        }
    }
    return false;
}
