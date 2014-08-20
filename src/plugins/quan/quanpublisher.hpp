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

#ifndef QUANPUBLISHER_HPP
#define QUANPUBLISHER_HPP

#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid.hpp>
#include <map>
#include <memory>
#include <QVector>
#include <QPointF>

namespace pugi { class xml_document; class xml_node; }
namespace adcontrols { class MassSpectrum; }
namespace adfs { class stmt; }

namespace quan {

    class QuanDocument;
    class QuanConnection;

    class QuanPublisher {
        QuanPublisher( const QuanPublisher& ) = delete;
    public:
        QuanPublisher();
        bool operator()( QuanConnection * );
        const boost::filesystem::path& filepath() const;

        struct resp_data {
            boost::uuids::uuid cmpId;
            int64_t respId;
            std::string name;
            int sampType;
            std::string formula;
            double mass;
            double mass_error;
            double intensity;
            double amount;
            std::string dataSource;
            std::string dataGuid;
            int fcn;
            int idx;
            int level;
            resp_data() : respId(0), sampType(0), mass(0), mass_error(0), intensity(0), amount(0), fcn(0), idx(0), level(0) {}
            resp_data( const resp_data& t ) = delete;
        };

        struct calib_curve {
            boost::uuids::uuid uuid;     // cmpdId
            std::string formula;
            std::wstring description;
            size_t n;
            double min_x;
            double max_x;
            std::string date;
            std::vector< double > coeffs;
            std::map< int, double > std_amounts;
            std::vector< std::pair< int, int64_t > > respIds;
            calib_curve() {}
            calib_curve( const calib_curve& t ) = delete;
        };

        struct calib_data {
            std::string formula;
            QVector< QPointF > xy;
            calib_data() {}
            calib_data( const calib_data& t ) = delete;
        };

    private:
        std::shared_ptr< QuanConnection > conn_;
        std::shared_ptr< pugi::xml_document > xmldoc_;
        boost::filesystem::path filepath_;

        std::map< boost::uuids::uuid, std::shared_ptr< calib_curve > > calib_curves_; // cmpdId, curve
        std::map< boost::uuids::uuid, std::shared_ptr< calib_data > > calib_data_;    /// cmpdId, data
        std::map< int64_t, std::shared_ptr< resp_data > > resp_data_;
        
        bool appendSampleSequence( pugi::xml_node& );
        bool appendProcessMethod( pugi::xml_node& );
        bool appendQuanResponseUnk( pugi::xml_node& );
        bool appendQuanResponseStd( pugi::xml_node& );
        bool appendQuanCalib( pugi::xml_node& );
    };

}

#endif // QUANPUBLISHER_HPP
