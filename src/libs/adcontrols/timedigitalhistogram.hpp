/**************************************************************************
** Copyright (C) 2015-2018 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include "adcontrols_global.h"
#include "tofprotocol.hpp"
#include <boost/serialization/version.hpp>
#include <vector>
#include <cstdint>
#include <iostream>
#include <functional>
#include <memory>

namespace boost {
    namespace serialization { class access; }
}

namespace adcontrols {

    class MassSpectrum;
    class ScanLaw;

    template< typename T > class TimeDigitalHistogram_archive;

	class ADCONTROLSSHARED_EXPORT TimeDigitalHistogram {
	public:
        typedef std::pair< double, uint32_t > value_type; // time(s), intensity
        typedef std::vector< value_type > vector_type;
        typedef std::vector< value_type >::iterator iterator;
        typedef std::vector< value_type >::const_iterator const_iterator;

        TimeDigitalHistogram();
        TimeDigitalHistogram( const TimeDigitalHistogram& );

        TimeDigitalHistogram& operator += ( const TimeDigitalHistogram& );

        size_t size() const;
        const value_type& operator [] ( size_t idx ) const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
#if 0
        double& initialXTimeSeconds();                    // digitizer time stamp, since acquisition started
        double& initialXOffset();                         // digitizer acquisition start offset ( delay )
        double& xIncrement();                             // digitizer sampling interval
        uint64_t& actualPoints();                         // digitizer waveform length (for spectrum display)
        uint64_t& trigger_count();
        uint32_t& wellKnownEvents();
        std::pair< uint64_t, uint64_t >& serialnumber();
        std::pair< uint64_t, uint64_t >& timeSinceEpoch();
#endif
        void setInitialXTimeSeconds( double );                    // digitizer time stamp, since acquisition started
        void setInitialXOffset( double );                         // digitizer acquisition start offset ( delay )
        void setXIncrement( double );                             // digitizer sampling interval
        void setActualPoints( uint64_t );                         // digitizer waveform length (for spectrum display)
        void setTrigger_count( uint64_t );
        void setWellKnownEvents( uint32_t );
        void setSerialnumber( const std::pair< uint64_t, uint64_t >& );
        void setTimeSinceEpoch( const std::pair< uint64_t, uint64_t >& );

        double initialXTimeSeconds() const;
        double initialXOffset() const;
        double xIncrement() const;
        uint64_t actualPoints() const;
        uint64_t trigger_count() const;
        uint32_t wellKnownEvents() const;

        const std::pair< uint64_t, uint64_t >& serialnumber() const;
        const std::pair< uint64_t, uint64_t >& timeSinceEpoch() const;
        vector_type& histogram();
        const vector_type& histogram() const;

        void setThis_protocol( const TofProtocol& );
        const TofProtocol& this_protocol() const;

        uint32_t protocolIndex() const;
        uint32_t nProtocols() const;
        void setProtocolIndex( uint32_t idx, uint32_t count );

        uint32_t accumulate( double tof, double window ) const;

        double triggers_per_second() const;

        std::shared_ptr< TimeDigitalHistogram > clone( const std::vector< std::pair<double, uint32_t > >& ) const;

        std::shared_ptr< TimeDigitalHistogram > merge_peaks( double resolution ) const;

        // this interface causes 'duplicate symbols for std::vector< std::pair< double, uint32_t > > on MSVC
        static bool average_time( const vector_type&, double resolution, vector_type&);

    public:
        typedef double( mass_assign_t )( double time, int mode );
        typedef std::function< mass_assign_t > mass_assignor_t;

        static bool translate( adcontrols::MassSpectrum&, const TimeDigitalHistogram& );
        static bool translate( adcontrols::MassSpectrum&, const TimeDigitalHistogram&, mass_assignor_t );

        static bool archive( std::ostream&, const TimeDigitalHistogram& );
        static bool restore( std::istream&, TimeDigitalHistogram& );

    private:
        double initialXTimeSeconds_;                       // digitizer time stamp, since acquisition started
        double initialXOffset_;                            // digitizer acquisition start offset ( delay )
        double xIncrement_;                                // digitizer sampling interval
        uint64_t actualPoints_;                            // digitizer waveform length (for spectrum display)
        uint64_t trigger_count_;
        uint32_t wellKnownEvents_;
        adcontrols::TofProtocol this_protocol_;
        uint32_t protocolIndex_;
        uint32_t nProtocols_;

        pragma_msvc_warning_push_disable_4251

        std::pair< uint64_t, uint64_t > serialnumber_;     // first, last waveform trigger#
        std::pair< uint64_t, uint64_t > timeSinceEpoch_;   // first waveform acquired time
        std::vector< std::pair< double, uint32_t > > histogram_;

        pragma_msvc_warning_pop

        friend class TimeDigitalHistogram_archive< TimeDigitalHistogram >;
        friend class TimeDigitalHistogram_archive< const TimeDigitalHistogram >;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );

        class device_data;
	};

#if defined _MSC_VER
    //ADCONTROLSSHARED_TEMPLATE_EXPORT template class ADCONTROLSSHARED_EXPORT std::vector< double, uint32_t >;
#endif

}

BOOST_CLASS_VERSION( adcontrols::TimeDigitalHistogram, 2 )

// V1 := + this_protocol
// V2 := + protocolIndex_, nProtocol
