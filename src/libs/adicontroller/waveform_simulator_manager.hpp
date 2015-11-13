// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "adicontroller_global.hpp"
#include <functional>

namespace adicontroller {

    class waveform_simulator;

    class ADICONTROLLERSHARED_EXPORT waveform_simulator_manager {

        class impl;

        waveform_simulator_manager();
        ~waveform_simulator_manager();
        waveform_simulator_manager( const waveform_simulator_manager& ) = delete;
        waveform_simulator_manager& operator = ( const waveform_simulator_manager& ) = delete;

    public:
        static waveform_simulator_manager& instance();

        void install_factory( std::function< std::shared_ptr< adicontroller::waveform_simulator >(*)(double, double, uint32_t, uint32_t) > f);

        std::shared_ptr< adicontroller::waveform_simulator > waveform_simulator() const;

    };

}
