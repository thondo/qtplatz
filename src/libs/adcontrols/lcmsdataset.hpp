// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include "acquireddataset.hpp"
#include <functional>
#include <vector>

namespace adcontrols {

    class Chromatogram;
    class MassSpectrum;

    class LCMSDataset : public AcquiredDataset {
    public:
        // LCMSDataset();
        virtual size_t getFunctionCount() const = 0;
        virtual size_t getSpectrumCount( int fcn ) const = 0;
        virtual size_t getChromatogramCount() const = 0;
        virtual bool getTIC( int fcn, adcontrols::Chromatogram& ) const = 0;
        virtual bool getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ) const = 0;
		virtual size_t posFromTime( double x ) const = 0;
		virtual bool getChromatograms( int fcn
			                         , const std::vector< std::pair<double, double> >&
			                         , std::vector< adcontrols::Chromatogram >&
									 , std::function< bool (long curr, long total ) > progress
									 , int begPos = 0
									 , int endPos = (-1) ) const = 0;
	};

}


