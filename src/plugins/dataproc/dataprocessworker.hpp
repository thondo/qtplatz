/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#ifndef DATAPROCESSWORKER_HPP
#define DATAPROCESSWORKER_HPP

#include <workaround/boost/asio.hpp>
#include <adportable/asio/thread.hpp>
#include <mutex>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>

namespace adcontrols { class MassSpectrum; class ProcessMethod; }
namespace adprot { class digestedPeptides; }
namespace qtwrapper { class ProgressBar; }

namespace dataproc {

	class Dataprocessor;

    class DataprocessWorker {
        DataprocessWorker();
        static DataprocessWorker * instance_;
        static std::mutex mutex_;
        std::vector< adportable::asio::thread > threads_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
    public:
        ~DataprocessWorker();

        static DataprocessWorker * instance();
        static void dispose();
        
        void createChromatograms( Dataprocessor *, const std::vector< std::tuple< int, double, double > >& );
        void createChromatograms( Dataprocessor *, std::shared_ptr< adcontrols::MassSpectrum >&, double lMass, double hMass );
        void createSpectrogram( Dataprocessor * );
		void clusterSpectrogram( Dataprocessor * );
        void findPeptide( Dataprocessor *, const adprot::digestedPeptides& );

    private:
        void terminate();
        void handleCreateChromatograms( Dataprocessor *
                                        , const std::shared_ptr< adcontrols::ProcessMethod >
                                        , const std::vector< std::tuple< int, double, double > >&
                                        , qtwrapper::ProgressBar* );

        void handleCreateSpectrogram( Dataprocessor *
                                      , const std::shared_ptr< adcontrols::ProcessMethod >
                                      , qtwrapper::ProgressBar* );
        
        void handleClusterSpectrogram( Dataprocessor *
                                       , const std::shared_ptr< adcontrols::ProcessMethod >
                                       , qtwrapper::ProgressBar* );

        void handleFindPeptide( Dataprocessor *
                                , const std::shared_ptr< adcontrols::ProcessMethod >
                                , qtwrapper::ProgressBar* );

        void join( const adportable::asio::thread::id& );
    };

}

#endif // DATAPROCESSWORKER_HPP
