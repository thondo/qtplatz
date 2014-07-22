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

#ifndef QUANSAMPLEPROCESSOR_HPP
#define QUANSAMPLEPROCESSOR_HPP

#include <adcontrols/datasubscriber.hpp>
//#include <adcontrols/quansample.hpp>
#include <string>
#include <vector>
#include <memory>


namespace adcontrols { class datafile; class QuanSample; class LCMSDataset; }
namespace portfolio { class Portfolio; class Folium;  }


namespace quan {

    class QuanDataWriter;

    class QuanSampleProcessor : public adcontrols::dataSubscriber {
        QuanSampleProcessor( const QuanSampleProcessor& ) = delete;
    public:
        ~QuanSampleProcessor();
        QuanSampleProcessor( const std::wstring& path, std::vector< adcontrols::QuanSample >& samples );
        bool operator()( std::shared_ptr< QuanDataWriter > writer );
        
    private:
        std::wstring path_;
        const adcontrols::LCMSDataset * raw_;
        std::vector< adcontrols::QuanSample > samples_;
        std::shared_ptr< adcontrols::datafile > datafile_;
        std::shared_ptr< portfolio::Portfolio > portfolio_;

        void open();
        bool subscribe( const adcontrols::LCMSDataset& d ) override;
        bool subscribe( const adcontrols::ProcessedDataset& d ) override;
        bool fetch( portfolio::Folium& folium );
    };

}

#endif // QUANSAMPLEPROCESSOR_HPP
