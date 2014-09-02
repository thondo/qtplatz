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

#ifndef IDAUDIT_HPP
#define IDAUDIT_HPP

#include "adcontrols_global.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <compiler/deprecated_register.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <string>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT idAudit  {
    public:
        idAudit();
        idAudit( const idAudit& );
        const char * digest() const { return digest_.c_str(); }
        const char * dateCreated() const { return dateCreated_.c_str(); }
        const wchar_t * idComputer() const { return idComputer_.c_str(); }
        const wchar_t * idCreatedBy() const { return idCreatedBy_.c_str(); }
        const wchar_t * nameCreatedBy() const { return nameCreatedBy_.c_str(); }

        const boost::uuids::uuid& uuid() const { return uuid_; }

        static bool xml_archive( std::wostream&, const idAudit& );
        static bool xml_restore( std::wistream&, idAudit& );

    private:
        boost::uuids::uuid uuid_;
        
        std::string digest_;
        std::string dateCreated_;
        std::wstring idComputer_;
        std::wstring idCreatedBy_;
        std::wstring nameCreatedBy_;
        
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( uuid_ )
                & BOOST_SERIALIZATION_NVP( digest_ )
                & BOOST_SERIALIZATION_NVP( dateCreated_ )
                & BOOST_SERIALIZATION_NVP( idComputer_ )
                & BOOST_SERIALIZATION_NVP( idCreatedBy_ )
                & BOOST_SERIALIZATION_NVP( nameCreatedBy_ )
                ;
        };
    };

}

#endif // IDAUDIT_HPP
