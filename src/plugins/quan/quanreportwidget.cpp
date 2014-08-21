/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanreportwidget.hpp"
#include "quanconnection.hpp"
#include "quanconstants.hpp"
#include "quandocument.hpp"
#include "quanmethodcomplex.hpp"
#include "quanpublisher.hpp"
#include "quanquery.hpp"
#include "quanqueryform.hpp"
#include "quanresulttable.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adportable/profile.hpp>
#include <adportable/xml_serializer.hpp>
#include <adpublisher/doceditor.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/xmlhelper.hpp>
#include <xmlparser/xmlencode.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <QCoreApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/spirit/include/classic.hpp>
#include <fstream>
#include <algorithm>

#ifdef Q_OS_MAC
const QString qrcpath = ":/adpublisher/images/mac";
#else
const QString qrcpath = ":/adpublisher/images/win";
#endif

using namespace quan;

QuanReportWidget::~QuanReportWidget()
{
}

QuanReportWidget::QuanReportWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QVBoxLayout( this ) )
                                                    , docEditor_( new adpublisher::docEditor )
{
    if ( auto am = Core::ActionManager::instance() ) {

        if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_FILE_MENU ) ) {
            menuContainer->menu()->setTitle( tr( "Publisher" ) );
            // docEditor_->setupFileActions( menuContainer->menu() );
            setupFileActions( menuContainer->menu() );
            am->actionContainer( Core::Constants::M_FILE )->addMenu( menuContainer );
        }

        if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_EDIT_MENU ) ) {
            menuContainer->menu()->setTitle( tr( "Publisher" ) );
            docEditor_->setupEditActions( menuContainer->menu() );
            am->actionContainer( Core::Constants::M_EDIT )->addMenu( menuContainer );
        }

        if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_TEXT_MENU ) ) {
            menuContainer->menu()->setTitle( tr( "Format" ) );
            docEditor_->setupTextActions( menuContainer->menu() );
            am->actionContainer( Core::Constants::MENU_BAR )->addMenu( menuContainer, Core::Constants::G_VIEW );
        }
        docEditor_->onInitialUpdate();
    }

    layout_->addWidget( docEditor_.get() );
    layout_->setStretch( 1, 10 );
    QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    docEditor_->setSizePolicy( policy );
}

void
QuanReportWidget::importDocTemplate()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Import ReportFormat..." )
                                                 , QuanDocument::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        QuanMethodComplex m;
        QuanDocument::instance()->load( name.toStdWString(), m );
        auto ptr = m.docTemplate();
        QuanDocument::instance()->method( ptr );
    }
}

void
QuanReportWidget::exportDocTemplate()
{
    boost::filesystem::path path( QuanDocument::instance()->lastMethodDir().toStdWString() );
    path.remove_filename();
    path /= "reportTemplate.xml";

    QString name = QFileDialog::getSaveFileName( this
                                                 , tr( "Export doc template..." )
                                                 , QString::fromStdWString( path.wstring() )
                                                 , tr( "XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        auto doc = docEditor_->document();
        doc->save_file( name.toUtf8() );
    }
}

void
QuanReportWidget::setupFileActions( QMenu * menu )
{
    QToolBar *tb = new QToolBar( docEditor_.get() );
    tb->setWindowTitle(tr("File Actions"));
    docEditor_->addToolBar( tb );

    QAction *a;

    a = new QAction( QIcon( ":/quan/images/run.png" ), tr( "Publish" ), this );
    a->setPriority(QAction::LowPriority);
    connect(a, &QAction::triggered, this, &QuanReportWidget::filePublish );
    tb->addAction(a);
    menu->addAction( a );

    QIcon newIcon = QIcon::fromTheme("document-new", QIcon(qrcpath + "/filenew.png"));
    a = new QAction( newIcon, tr("&New"), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileNew()));
    tb->addAction(a);
    menu->addAction( a );

    a = new QAction(QIcon::fromTheme("document-open", QIcon(qrcpath + "/fileopen.png")), tr("&Open..."), docEditor_.get());
    a->setShortcut(QKeySequence::Open);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction( a );

    menu->addSeparator();

    a = new QAction( QIcon::fromTheme( "document-save", QIcon( qrcpath + "/filesave.png" ) ), tr( "&Save" ), docEditor_.get() );
    docEditor_->setAction( adpublisher::docEditor::idActionSave, a );
    a->setShortcut(QKeySequence::Save);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileSave()));
    a->setEnabled(false);
    tb->addAction(a);
    menu->addAction( a );

    a = new QAction(tr("Save &As..."), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    a = new QAction(QIcon::fromTheme("document-print", QIcon(qrcpath + "/fileprint.png")),  tr("&Print..."), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(filePrint()));
    tb->addAction(a);
    menu->addAction( a );

    a = new QAction(QIcon::fromTheme("fileprint", QIcon(qrcpath + "/fileprint.png")), tr("Print Preview..."), docEditor_.get());
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(filePrintPreview()));
    menu->addAction( a );

    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(qrcpath + "/exportpdf.png")), tr("&Export PDF..."), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(filePrintPdf()));
    tb->addAction(a);
    menu->addAction( a );
#endif

}

void
QuanReportWidget::filePublish()
{
    auto conn = QuanDocument::instance()->connection();
    if ( !conn )
        return;

    QuanPublisher publisher;
    if ( publisher( conn ) ) {

        const QString apppath = QCoreApplication::applicationDirPath() + QLatin1String( "/../share/qtplatz/xslt" );  // sibling of /translations
        boost::filesystem::path xsltpath = boost::filesystem::path( apppath.toStdWString() ).normalize();

        boost::filesystem::path path = publisher.filepath(); 

        publisher.save_file( path.string().c_str() ); // save publisher document xml

        QString output;
        adpublisher::document::apply_template( path.string().c_str()
                                               , (xsltpath / "quan-html.xsl").string().c_str()
                                               , output );
        
        path.replace_extension( ".html" );
        boost::filesystem::ofstream o( path );
        o << output.toStdString();
    }
}
