// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "datainterpreter.hpp"
#include "massspectrometerbroker.hpp"
#include "massspectrometer_factory.hpp"
#include "adcontrols.hpp"
#include <adportable/utf.hpp>
#include <string>
#include <cmath>

using namespace adcontrols;

namespace adcontrols {
    namespace internal {
		class ScanLaw : public massspectrometer::ScanLaw {
        public:
            virtual ~ScanLaw() {}
            ScanLaw( double timeCoefficient, double timeDelay, double acclVolt )
                : timeCoefficient_(timeCoefficient), timeDelay_(timeDelay), acclVoltage_(acclVolt) {
            }
            inline double getMass( double secs, int nTurn ) const { 
                return getMass( secs, 0.43764 + nTurn * 0.66273 );
            }
            inline double getTime( double mass, int nTurn ) const {
                return getTime( mass, 0.43764 + nTurn * 0.66273 );
            }
            double getMass( double secs, double fLength ) const {
                double t = secs / fLength - timeDelay_;
                double m = ( ( timeCoefficient_ * timeCoefficient_ ) * ( t * t ) ) * acclVoltage_;
                return m;
            }
            double getTime( double mass, double fLength ) const {
                double v = std::sqrt( acclVoltage_ / mass ) * timeCoefficient_; // (m/s)
                return fLength * ( 1.0 / v ) + timeDelay_;
            }
			double fLength( int nTurn ) const {
				return 1.0 * ( 1 + nTurn );
			}
        private:
            double timeCoefficient_;
            double timeDelay_;
            double acclVoltage_;
        };

        class DataInterpreter : public adcontrols::DataInterpreter {
        public:
            DataInterpreter(void) {  }
            virtual ~DataInterpreter(void) {  }

            adcontrols::translate_state translate( MassSpectrum&
                                                   , const char * data, size_t dsize
                                                   , const char * meta, size_t msize 
                                                   , const MassSpectrometer&
                                                   , size_t idData
												   , const MSCalibration * ) const override {
                (void)data; (void)dsize; (void)meta; (void)msize; (void)idData;
                return adcontrols::translate_error;
            }
            
            adcontrols::translate_state translate( TraceAccessor&
                                                   , const char * data, size_t dsize
                                                   , const char * meta, size_t msize
                                                   , unsigned long events ) const override {
                (void)data; (void)dsize; (void)meta; (void)msize; (void)events;
                return adcontrols::translate_error;
            }

        };

        class MassSpectrometerImpl : public MassSpectrometer {
        public:
            ~MassSpectrometerImpl() {
                delete impl_;
                impl_ = 0;
                delete scanLaw_;
                delete interpreter_;
            }
            MassSpectrometerImpl() {
                scanLaw_ = new internal::ScanLaw( 0.01389e6, 0.0, 5000 );
                interpreter_ = new internal::DataInterpreter();
            }
            virtual void accept( adcontrols::Visitor& ) {
            }
            const wchar_t * name() const override {
				return L"default";
            }
            const massspectrometer::ScanLaw& getScanLaw() const override {
                return * scanLaw_;
            }
			std::shared_ptr< massspectrometer::ScanLaw > scanLaw( const MSProperty& ) const override {
				return std::make_shared< internal::ScanLaw >( 0.01389e6, 0.0, 5000 );
			}
            const adcontrols::DataInterpreter& getDataInterpreter() const override {
                return * interpreter_;
            }
        private:
            static adcontrols::MassSpectrometer * instance() {
                if ( ! impl_ )
                    impl_ = new MassSpectrometerImpl;
                return impl_;
            }
            internal::ScanLaw * scanLaw_;
            internal::DataInterpreter * interpreter_;
            static MassSpectrometerImpl * impl_;
        };
    }
}

internal::MassSpectrometerImpl * internal::MassSpectrometerImpl::impl_ = 0;


///////////////////////////////////////////////

const MassSpectrometer&
MassSpectrometer::get( const wchar_t * dataInterpreterClsid )
{
	massspectrometer_factory * factory = massSpectrometerBroker::find( dataInterpreterClsid );
	if ( factory )
		return *factory->get( dataInterpreterClsid );
    static internal::MassSpectrometerImpl dummy;
    return dummy;
}

const MassSpectrometer&
MassSpectrometer::get( const char * dataInterpreterClsid )
{
	std::wstring wstr = adportable::utf::to_wstring( dataInterpreterClsid );
	return get( wstr.c_str() );
}

std::vector< std::wstring > 
MassSpectrometer::get_model_names()
{
    return massSpectrometerBroker::names();
}

//////////////////////////////////////////////////////////////
