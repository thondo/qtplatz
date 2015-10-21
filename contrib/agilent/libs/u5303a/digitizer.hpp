/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef DIGITIZER_HPP
#define DIGITIZER_HPP

#include "u5303a_global.hpp"
#include <acqrscontrols/u5303a/identify.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } }
namespace adportable { class TimeSquaredScanLaw; }
namespace acqrscontrols { namespace u5303a { class method; class device_method; } }

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace u5303a {

    namespace detail { class task; }

	class U5303ASHARED_EXPORT device_data {
    public:
        acqrscontrols::u5303a::identify ident;
        acqrscontrols::u5303a::metadata meta;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident );
            ar & BOOST_SERIALIZATION_NVP( meta );           
        }
    };
    

    class U5303ASHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const adcontrols::ControlMethod::Method& );
        bool peripheral_prepare_for_run( const acqrscontrols::u5303a::method& );
        bool peripheral_run();
        bool peripheral_stop();
        bool peripheral_trigger_inject();
        bool peripheral_terminate();
        void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > );

        typedef std::function< void( const std::string, const std::string ) > command_reply_type;
        typedef std::function< bool( const acqrscontrols::u5303a::waveform *, const acqrscontrols::u5303a::waveform *, acqrscontrols::u5303a::method& ) > waveform_reply_type;

        void connect_reply( command_reply_type ); // method,reply
        void disconnect_reply( command_reply_type );

        void connect_waveform( waveform_reply_type );
        void disconnect_waveform( waveform_reply_type );
    };

}

//BOOST_CLASS_VERSION( u5303a::method, 3 )
//BOOST_CLASS_VERSION( u5303a::metadata, 1 )
//BOOST_CLASS_VERSION( u5303a::identify, 1 )

#endif
