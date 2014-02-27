/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include <cstdint>
#include <vector>
#include <utility>
#include <memory>

namespace u5303a {

    class waveform_generator : public std::enable_shared_from_this< waveform_generator > {
    public:
        waveform_generator( uint32_t nStartDelay = 0
                            , uint32_t nbrSamples = 65536
                            , double exitDelay = 0
                            , uint32_t nbrWavefoms = 1
                            , uint32_t sampInterval_ = 500 );

        void addIons( const std::vector< std::pair<double, double> >& ); // pair<mass, intensity>
        void onTriggered(); // ns
        const std::vector< int32_t >& waveform() const;
        uint64_t timestamp() const;
        uint32_t startDelay() const;
        uint32_t nbrWaveforms() const;
        uint32_t nbrSamples() const;
        uint32_t exitDelay() const;
        uint32_t sampInterval() const;
        
    private:
        double exitDelay_;    // seconds 

        uint32_t nStartDelay_;
        uint32_t nbrSamples_;
        uint32_t sampInterval_; // ps
        uint64_t timeStamp_;    // ps
        uint32_t nbrWaveforms_;

        std::vector< int32_t > waveform_;

        struct ion_t {
            double mass;
            double width;
            double height;
        };

        std::vector< ion_t > ions_; // pair<time, intensity>
        static double mass_to_time( double mass, int nTurn = 0 );
    };

}

