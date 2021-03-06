/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/version.hpp>
#include <memory>
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class moltable;

    class ADCONTROLSSHARED_EXPORT MSSimulatorMethod {
    public:
        ~MSSimulatorMethod();
        MSSimulatorMethod();
        MSSimulatorMethod( const MSSimulatorMethod& );
        MSSimulatorMethod& operator = ( const MSSimulatorMethod& );

        static const wchar_t * dataClass() { return L"adcontrols::MSSimulatorMethod"; }

        uint32_t chargeStateMin() const;
        uint32_t chargeStateMax() const;

        void setChargeStateMin( uint32_t );
        void setChargeStateMax( uint32_t );

        void setResolvingPower( double );
        double resolvingPower() const;

        bool isPositivePolarity() const;
        void setIsPositivePolarity( bool );

        bool isTof() const;
        void setIsTof( bool );
        double length() const;
        void setLength( double );
        double acceleratorVoltage() const;
        void setAcceleratorVoltage( double );
        double tDelay() const;
        void setTDelay( double );

        void setProtocol( int );
        int protocol() const;
        void setMode( int );
        int mode() const;

        const moltable& molecules() const;
        moltable& molecules();
        void setMolecules( const moltable& );

        void setAbundanceLowLimit( double );
        double abundanceLowLimit() const;

    private:
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

}
