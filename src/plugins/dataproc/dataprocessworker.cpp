/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "dataprocessworker.hpp"
#include "dataprocessor.hpp"
#include "sessionmanager.hpp"
#include <adportable/debug.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processmethod.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>

using namespace dataproc;

DataprocessWorker * DataprocessWorker::instance_ = 0;
std::mutex DataprocessWorker::mutex_;

DataprocessWorker::DataprocessWorker()
{
}

DataprocessWorker::~DataprocessWorker()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    instance_ = 0;
}

DataprocessWorker *
DataprocessWorker::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new DataprocessWorker;
    }
    return instance_;
}

void
DataprocessWorker::createChromatograms( Dataprocessor *, std::shared_ptr< adcontrols::MassSpectrum >&, double lMass, double hMass )
{
	(void)lMass;
	(void)hMass;
}

void
DataprocessWorker::createChromatograms( Dataprocessor* processor
                                        , const std::vector< std::tuple< int, double, double > >& ranges )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    threads_.push_back( std::thread( [=] { handleCreateChromatograms( processor, ranges ); } ) );
}

void
DataprocessWorker::joinMe( const std::thread::id&)
{
    std::lock_guard< std::mutex > lock( mutex_ );

	// auto& it = std::find( threads_.begin(), threads_.end(), [&]( std::thread& t ) { return t.get_id() == id; } );

    // if ( it != threads_.end() ) {
    // it->join();
        // threads_.erase( it );
}

void
DataprocessWorker::handleCreateChromatograms( Dataprocessor* processor
                                              , const std::vector< std::tuple< int, double, double > >& ranges )
{
    for ( auto range: ranges )
        adportable::debug(__FILE__, __LINE__ ) << "createChromatogram: " << std::get<1>(range) << ", " << std::get<2>(range);
    
    std::vector< adcontrols::Chromatogram > vec;

    if ( const adcontrols::LCMSDataset * dset = processor->getLCMSDataset() ) {
        dset->getChromatograms( ranges, vec, [](long curr, long total)->bool{
                adportable::debug(__FILE__, __LINE__) << curr << "/" << total;
				return true;
            } );
    }

    adcontrols::ProcessMethod method;
    adcontrols::PeakMethod peakmethod;
	method.appendMethod( peakmethod );

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( c, method );
	SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );

	// auto it = std::find( threads_.begin(), threads_.end(), [=]( std::thread& t ) { return t.get_id() == std::this_thread::get_id(); } );
    // threads_.push_back( std::thread( [&] { joinMe( std::this_thread::get_id() ); } ) );
}
