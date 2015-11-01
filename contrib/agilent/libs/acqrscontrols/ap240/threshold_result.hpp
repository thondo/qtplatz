/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "../acqrscontrols_global.hpp"
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>

namespace acqrscontrols {
    namespace ap240 {

        class waveform;

        namespace ap240x = acqrscontrols::ap240;

        class ACQRSCONTROLSSHARED_EXPORT threshold_result {

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
            std::shared_ptr< const ap240x::waveform > data_;
            std::vector< uint32_t > indecies_;
            std::vector< double > processed_;
            uint32_t foundIndex_;
            std::pair< uint32_t, uint32_t > findRange_;

#if defined _MSC_VER
#pragma warning(pop)
#endif
        public:
            std::shared_ptr< const ap240x::waveform >& data();
            std::vector< uint32_t >& indecies();
            std::vector< double >& processed();
            std::shared_ptr< const ap240x::waveform > data() const;
            const std::vector< uint32_t >& indecies() const;
            const std::vector< double >& processed() const;
            const std::pair<uint32_t, uint32_t >& findRange() const;
            uint32_t foundIndex() const;
            void setFoundAction( uint32_t index, const std::pair< uint32_t, uint32_t >& );
#if defined _MSC_VER
            static const uint32_t npos = (-1);
#else
            static constexpr uint32_t npos = (-1);
#endif
            bool hasFoundIndex() const { return foundIndex_ != npos; }

            threshold_result();
            threshold_result( std::shared_ptr< const ap240x::waveform > d );
            threshold_result( const threshold_result& t );
        };

        ACQRSCONTROLSSHARED_EXPORT std::ostream& operator << ( std::ostream&, const threshold_result& );

    }
}
