/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "massspectrometer.hpp"
#include "constants.hpp"
#include "scanlaw.hpp"
#include <accutofcontrols/method.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adcontrols/datareader.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include "constants.hpp"
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <mutex>

using namespace accutof::spectrometer;

MassSpectrometer::~MassSpectrometer()
{
}

MassSpectrometer::MassSpectrometer() : scanLaw_( std::make_unique< ScanLaw >() )
{
}

const wchar_t *
MassSpectrometer::name() const
{
    return accutof::spectrometer::dataInterpreter::spectrometer::name();
}

std::shared_ptr< adcontrols::ScanLaw >
MassSpectrometer::scanLaw( const adcontrols::MSProperty& prop ) const
{
    if ( scanLaw_ ) {
        auto ptr = std::make_shared< ScanLaw >( *scanLaw_ );
        ptr->setAcceleratorVoltage( 7000 );
        ptr->setTDelay( 0 );
        return ptr;
    } else {
        return std::make_shared< ScanLaw >( prop.acceleratorVoltage(), prop.tDelay() );
    }
}

void
MassSpectrometer::setAcceleratorVoltage( double acclVoltage, double tDelay )
{
    if ( !scanLaw_ ) {
        scanLaw_ = std::make_unique< ScanLaw >( acclVoltage, tDelay );
    } else {
        scanLaw_->setAcceleratorVoltage( acclVoltage );
        scanLaw_->setTDelay( tDelay );
    }
}

const char *
MassSpectrometer::objtext() const
{
    return class_name; // 'Accutof'
}

const adcontrols::ScanLaw *
MassSpectrometer::scanLaw() const
{
    return scanLaw_.get();
}

void
MassSpectrometer::setMethod( const adcontrols::ControlMethod::Method& m )
{
    method_ = std::make_unique< adcontrols::ControlMethod::Method >( m );
}

const adcontrols::ControlMethod::Method *
MassSpectrometer::method() const
{
    return method_.get();
}

int
MassSpectrometer::mode( uint32_t protocolNumber ) const
{
    return 0;
}

bool
MassSpectrometer::setMSProperty( adcontrols::MassSpectrum& ms, const adcontrols::ControlMethod::Method& m, int proto ) const
{
    auto it = m.find( m.begin(), m.end(), accutofcontrols::method::clsid() );
    if ( it != m.end() ) {
        accutofcontrols::method im;
        if ( adcontrols::ControlMethod::MethodItem::get<>( *it, im ) ) {
            // if ( im.tof().protocols.size() > proto ) {
            //     ms.setAcquisitionMassRange( im.tof().protocols[proto].lower_mass, im.tof().protocols[proto].upper_mass );

            //     std::vector< adcontrols::TofProtocol > v;
            //     accutofcontrols::method::copy_protocols( im.tof(), v );
            //     if ( v.size() > proto ) {
            //         auto& prop = ms.getMSProperty();
            //         prop.setTofProtocol( v[ proto ] );
            //     }
            //     return true;
            // }
        }
    }
    return false;
}

void
MassSpectrometer::initialSetup( adfs::sqlite& dbf, const boost::uuids::uuid& objuuid )
{
    adfs::stmt sql( dbf );

    sql.prepare( "SELECT acclVoltage,tDelay FROM ScanLaw WHERE objuuid=?" );
    sql.bind( 1 ) = objuuid;

    if ( sql.step() == adfs::sqlite_row ) {
        acceleratorVoltage_ = sql.get_column_value< double >( 0 );
        tDelay_             = sql.get_column_value< double >( 1 );
    }
}


bool
MassSpectrometer::estimateScanLaw( const adcontrols::MSPeaks& peaks, double& va, double& t0 ) const
{
    if ( auto law = scanLaw() ) {

        if ( peaks.size() == 1 ) {

            const adcontrols::MSPeak& pk = peaks[ 0 ];
            va = scanLaw_->acceleratorVoltage( pk.exact_mass(), pk.time(), pk.mode(), 0.0 );
            t0 = 0.0;
            ADDEBUG() << "scanlaw: " << va << ", " << t0;
            return true;

        } else if ( peaks.size() >= 2 ) {

            std::vector<double> x, y, coeffs;

            for ( auto& pk : peaks ) {
                x.push_back( std::sqrt( pk.exact_mass() ) * law->fLength( pk.mode() ) );
                y.push_back( pk.time() );
            }

            if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {

                t0 = coeffs[ 0 ];
                double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, 1mL
                va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

                return true;
            }
        }
    }
    return false;
}

const adcontrols::ScanLaw *
MassSpectrometer::scanLaw( int64_t rowid ) const
{
    return scanLaw_.get();
}

bool
MassSpectrometer::assignMasses( adcontrols::MassSpectrum& ms, int64_t rowid ) const
{
    //auto mode = ms.mode();
    auto scanlaw = scanLaw( rowid );
    return ms.assign_masses( [&]( double time, int mode ) { return scanlaw->getMass( time, mode ); } );
}

const char *
MassSpectrometer::dataInterpreterText() const
{
    return names::objtext_datainterpreter; // "AccuTOF"
}

const boost::uuids::uuid&
MassSpectrometer::dataInterpreterUuid() const
{
    return iids::uuid_datainterpreter;
}

const boost::uuids::uuid&
MassSpectrometer::objclsid() const
{
    return iids::uuid_massspectrometer; // 9568b15d-73b6-48ed-a1c7-ac56a308f712 defined in accutofcontrols/constants.hpp
}