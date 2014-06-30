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

#include "msspectrawnd.hpp"
#include "mainwindow.hpp"
#include "dataprocessor.hpp"
#include "selchanged.hpp"
#include <adcontrols/description.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adutils/processeddata.hpp>
#include <adwplot/spectrogramwidget.hpp>
#include <adwidgets/msmergedtable.hpp>
#include <portfolio/folium.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

#include <qtwrapper/qstring.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adportable/configuration.hpp>

#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/widget_factory.hpp>

#include <qtwidgets/peakresultwidget.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include "qtwidgets_name.hpp"

using namespace dataproc;

namespace dataproc {

    // class ChromatogramWndImpl {
    // public:
    //     ~ChromatogramWndImpl() {
    //         delete chroWidget_;
    //         delete peakWidget_;
    //     }
    //     ChromatogramWndImpl() : chroWidget_(0)
    //                           , peakWidget_(0) {
    //     }
    //     void setData( const adcontrols::Chromatogram&, const QString& );
    //     adwplot::ChromatogramWidget * chroWidget_;
    //     QWidget * peakWidget_; // adplutin::manager::widget_factory will make a widget
    // };

    // //----------------------------//
    // template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
    //     selProcessed( Wnd& wnd ) : wnd_(wnd) {}
    //     template<typename T> void operator ()( T& ) const {
    //     }
    //     void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
    //         wnd_.draw2( ptr );
    //     }
    //     void operator () ( adutils::ChromatogramPtr& ptr ) const {
    //         wnd_.draw( ptr );
    //     }
    //     void operator () ( adutils::PeakResultPtr& ptr ) const {
    //         wnd_.draw( ptr );
    //     }
    //     Wnd& wnd_;
    // };

}

MSSpectraWnd::~MSSpectraWnd()
{
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) :  QWidget(parent)
                                              , plot_( new adwplot::SpectrumWidget )
                                              , table_( new adwidgets::MSMergedTable )
{
    init();
}

void
MSSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( adplugin::LifeCycle * p = dynamic_cast<adplugin::LifeCycle *>(table_.get()) ) {
            p->OnInitialUpdate();
            //connect( this, SIGNAL( fireSetData( const adcontrols::PeakResult& ) ),
            //    pImpl_->peakWidget_, SLOT( setData( const adcontrols::PeakResult& ) ) );
        }
        splitter->addWidget( plot_.get() );
        splitter->addWidget( table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSSpectraWnd::draw1( adutils::MassSpectrumPtr& )
{
}

void
MSSpectraWnd::draw2( adutils::MassSpectrumPtr& )
{
}

#if 0
void
MSSpectraWnd::draw( adutils::ChromatogramPtr& ptr )
{
    adcontrols::Chromatogram& c = *ptr;
    pImpl_->chroWidget_->setData( c );
	if ( c.peaks().size() ) {
		adcontrols::PeakResult r( c.baselines(), c.peaks() );
		emit fireSetData( r );
	}
}
#endif

void
MSSpectraWnd::handleSessionAdded( Dataprocessor * )
{
	/*
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            c.addDescription( adcontrols::Description( L"filename", file.filename() ) );
            //pImpl_->setData( c, filename );
        }
    }
	*/
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor* , portfolio::Folium& folium )
{
#if 0
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    boost::apply_visitor( selChanged<MSSpectraWnd>(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<MSSpectraWnd>( *this ), contents );
    }
#endif
}

void
MSSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSSpectraWnd::handleAxisChanged( int )
{
}

void
MSSpectraWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
}
///////////////////////////

// void
// MSSpectraWndImpl::setData( const adcontrols::Chromatogram& c, const QString& )
// {
//     chroWidget_->setData( c );
// }

