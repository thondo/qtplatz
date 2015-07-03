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

#include "ap240_global.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

namespace adcontrols { class ControlMethod; }
namespace adportable { class TimeSquaredScanLaw; }

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace ap240 {

    namespace detail { class task; }

    class AP240SHARED_EXPORT identify {
    public:
        identify();
        identify( const identify& );
        std::string Identifier;
        std::string Revision;
        std::string Vendor;
        std::string Description;
        std::string InstrumentModel;
        std::string FirmwareRevision;
        std::string SerialNumber;
        std::string Options;
        std::string IOVersion;
        uint32_t    NbrADCBits;
        uint32_t bus_number_;
        uint32_t slot_number_;
        uint32_t serial_number_;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( Identifier );
            ar & BOOST_SERIALIZATION_NVP( Revision );
            ar & BOOST_SERIALIZATION_NVP( Vendor );
            ar & BOOST_SERIALIZATION_NVP( Description );
            ar & BOOST_SERIALIZATION_NVP( InstrumentModel );
            ar & BOOST_SERIALIZATION_NVP( FirmwareRevision );
            if ( version >= 1 ) {
                ar & BOOST_SERIALIZATION_NVP( SerialNumber );
                ar & BOOST_SERIALIZATION_NVP( Options );
                ar & BOOST_SERIALIZATION_NVP( IOVersion );
                ar & BOOST_SERIALIZATION_NVP( NbrADCBits );                
            }
        }
    };

	class /* AP240SHARED_EXPORT */ method {
    public:
        method()
            : front_end_range( 5.0 )        // 1V,2V range
            , front_end_offset( 0.0 )       // [-0.5V,0.5V], [-1V,1V] offset
            , ext_trigger_level( 0.5 )      // external trigger threshold
			, samp_rate( 1/0.5e-9 )			// sampling rate (2.0GS/s)
            , nbr_of_s_to_acquire( 16000 ) // from 1 to 480,000 samples
            , nbr_of_averages( 128 )		// number of averages minus one. >From 0 to 519,999 averages in steps of 8. For instance 0,7,15
            , delay_to_first_sample( 1.024e-6 )  // delay from trigger (seconds)
            , invert_signal( 0 )                  // 0-> no inversion , 1-> signal inverted
            , nsa( 0x0 )
            , ext_trigger_slope( 0 )
            , ext_trigger_range( 1.0 )
            , ext_trigger_offset( 0.0 )
            , ext_trigger_bandwidth( 2 )
            , bandwidth{ 2, 2 }
            , average_mode( 2 )
            , digitizer_ndelay_( 1024 ) // 512ns
            { }
        double front_end_range;
        double front_end_offset;
        double ext_trigger_level;
        double samp_rate; // HZ
        int32_t nbr_of_s_to_acquire;
        int32_t nbr_of_averages;
        double delay_to_first_sample;
        int32_t invert_signal;
        int32_t nsa;
        // double digitizer_delay_to_first_sample; // actual delay set to ap240
        // uint32_t digitizer_nbr_of_s_to_acquire; // actual number of samples per waveform
        uint32_t ext_trigger_polarity; // 0(pos)|1(neg)
        double ext_trigger_range;
        double ext_trigger_offset;
        uint32_t ext_trigger_slope;
        uint32_t ext_trigger_bandwidth;
        uint32_t bandwidth[2];
        uint32_t average_mode;
        uint32_t digitizer_ndelay_;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( front_end_range );
            ar & BOOST_SERIALIZATION_NVP( front_end_offset );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_level );
            ar & BOOST_SERIALIZATION_NVP( samp_rate );
            ar & BOOST_SERIALIZATION_NVP( nbr_of_s_to_acquire );
            ar & BOOST_SERIALIZATION_NVP( nbr_of_averages );
            ar & BOOST_SERIALIZATION_NVP( delay_to_first_sample );
            ar & BOOST_SERIALIZATION_NVP( invert_signal );
            ar & BOOST_SERIALIZATION_NVP( nsa );
            //ar & BOOST_SERIALIZATION_NVP( digitizer_delay_to_first_sample );
            //ar & BOOST_SERIALIZATION_NVP( digitizer_nbr_of_s_to_acquire );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_polarity );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_range );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_offset );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_slope );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_bandwidth );
            ar & BOOST_SERIALIZATION_NVP( bandwidth );
            ar & BOOST_SERIALIZATION_NVP( average_mode );
            ar & BOOST_SERIALIZATION_NVP( digitizer_ndelay_ );            
        }
    };

    class /* AP240SHARED_EXPORT */ metadata {
    public:
        metadata() : initialXTimeSeconds( 0 )
			, actualPoints( 0 )
            , flags( 0 )
            , actualAverages( 0 )
            , actualRecords( 0 )
            , initialXOffset( 0 )
            , scaleFactor( 0 )
            , scaleOffset(0) {
        }
        double initialXTimeSeconds; 
        int64_t actualPoints;
        int32_t flags;
        int32_t actualAverages;
        int64_t actualRecords;
        double initialXOffset;
        double xIncrement;
        double scaleFactor;
        double scaleOffset;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( initialXTimeSeconds );
            ar & BOOST_SERIALIZATION_NVP( actualPoints );
            if ( version == 0 ) {
                int64_t firstValidPoint;
                ar & BOOST_SERIALIZATION_NVP( firstValidPoint );
            } else {
                ar & BOOST_SERIALIZATION_NVP( flags );
            }
            ar & BOOST_SERIALIZATION_NVP( actualAverages );
            ar & BOOST_SERIALIZATION_NVP( actualRecords );
            ar & BOOST_SERIALIZATION_NVP( initialXOffset );
            ar & BOOST_SERIALIZATION_NVP( xIncrement );
            ar & BOOST_SERIALIZATION_NVP( scaleFactor );
            ar & BOOST_SERIALIZATION_NVP( scaleOffset );
        }
    };
    
	class AP240SHARED_EXPORT waveform : public std::enable_shared_from_this< waveform > {
		waveform( const waveform& ); // = delete;
		void operator = ( const waveform& ); // = delete;
	public:
        waveform( std::shared_ptr< identify >& id ) : ident_( id ), wellKnownEvents_( 0 ), serialnumber_( 0 ) {
        }
        
        const int32_t * trim( metadata&, uint32_t& ) const;
        
        method method_;
        metadata meta_;
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        std::vector< int32_t > d_;
        std::shared_ptr< identify > ident_;
    private:
        
    };

	class AP240SHARED_EXPORT device_data {
    public:
        identify ident;
        metadata meta;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident );
            ar & BOOST_SERIALIZATION_NVP( meta );           
        }
    };
    

    class AP240SHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const adcontrols::ControlMethod& );
        bool peripheral_prepare_for_run( const ap240::method& );
        bool peripheral_run();
        bool peripheral_stop();
        bool peripheral_trigger_inject();
        bool peripheral_terminate();
        void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > );

        typedef std::function< void( const std::string, const std::string ) > command_reply_type;
        typedef std::function< bool( const waveform *, ap240::method& ) > waveform_reply_type;

        void connect_reply( command_reply_type ); // method,reply
        void disconnect_reply( command_reply_type );

        void connect_waveform( waveform_reply_type );
        void disconnect_waveform( waveform_reply_type );
        //-------
        bool findDevice();
        bool initial_setup( int nDelay, int nSamples, int nAverage );
        bool acquire();
        bool stop();
        enum result_code { success, error_timeout, error_overload, error_io_read, error_stopped };
        result_code waitForEndOfAcquisition( size_t timeout );
    };

}

BOOST_CLASS_VERSION( ap240::method, 3 )
BOOST_CLASS_VERSION( ap240::metadata, 1 )
BOOST_CLASS_VERSION( ap240::identify, 1 )

#endif
