/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <memory>

namespace boost { namespace serialization { class access; } }

namespace aqdrv4 {

    struct trigger_method {
        uint32_t trigClass;
        uint32_t trigPattern;
        uint32_t trigCoupling;
        uint32_t trigSlope;
        double trigLevel1;
        double trigLevel2;
        trigger_method() : trigClass( 0 ) // edge trigger
                         , trigPattern( 0x80000000 ) // Ext 1
                         , trigCoupling( 0 ) // DC
                         , trigSlope( 0 ) // positive
                         , trigLevel1( 1000.0 ) // mV for Ext, %FS for CHn
                         , trigLevel2( 0.0 )    // only if window for trigSlope (3)
            {}
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    
    struct horizontal_method {
        double sampInterval;
        uint32_t mode;  // configMode, 0: normal, 2: averaging
        uint32_t flags; // configMode, if mode == 0, 0: normal, 1: start on trigger
        uint32_t nbrAvgWaveforms;
        int32_t  nStartDelay; // digitizer mode can be negative
        uint32_t nbrSamples;
        horizontal_method() : sampInterval( 0.5e-9 )
                            , mode( 0 )
                            , flags( 0 )
                            , nbrAvgWaveforms( 1 )
                            , nStartDelay( 0 )
                            , nbrSamples( 0 ) // filled when apply to device
            {}
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

    struct vertical_method {
        double fullScale;
        double offset;
        uint32_t coupling;
        uint32_t bandwidth;
        bool invertData;
        bool autoScale;
        vertical_method() : fullScale( 5.0 )
                          , offset( 0.0 )
                          , coupling( 3 )
                          , bandwidth( 2 )
                          , invertData( false )
                          , autoScale( true )
            {}
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    

    class acqiris_method {
    public:
        acqiris_method();
        acqiris_method( const acqiris_method& t );
        
        static const char * modelClass() { return "AqDrv4"; };
        static const char * itemLabel() { return "AqDrv4"; };
        static const boost::uuids::uuid& clsid();
        
        enum class DigiMode : uint32_t { Digitizer = 0, Averager = 2 };

        std::shared_ptr< trigger_method > mutable_trig();
        std::shared_ptr< horizontal_method > mutable_hor();
        std::shared_ptr< vertical_method > mutable_ch1();
        std::shared_ptr< vertical_method > mutable_ch2();

        DigiMode mode;
        boost::uuids::uuid clsid_;
        std::shared_ptr< trigger_method > trig;
        std::shared_ptr< horizontal_method > hor;
        std::shared_ptr< vertical_method > ch1;
        std::shared_ptr< vertical_method > ch2;
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
}

BOOST_CLASS_VERSION( aqdrv4::trigger_method, 1 )
BOOST_CLASS_VERSION( aqdrv4::horizontal_method, 1 )
BOOST_CLASS_VERSION( aqdrv4::vertical_method, 1 )
BOOST_CLASS_VERSION( aqdrv4::acqiris_method, 1 )
