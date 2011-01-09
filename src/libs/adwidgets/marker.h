// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

namespace SAGRAPHICSLib {
    struct ISADPMarker;
}

namespace adwidgets {
    namespace ui {

        enum MarkerStyle {
            MS_None = 0,
            MS_Point = 1,
            MS_Circle = 2,
            MS_Dot = 3,
            MS_Box = 4,
            MS_Square = 5,
            MS_X = 6,
            MS_Plus = 7,
            MS_Diamond = 8,
            MS_FilledDiamond = 9
        };
        
        class Marker  {
        public:
            ~Marker();
            Marker( SAGRAPHICSLib::ISADPMarker * pi = 0 );
            Marker( const Marker& );

            MarkerStyle style() const;
            void style( MarkerStyle );

            short colorIndex() const;
            void colorIndex( short );

        private:
            SAGRAPHICSLib::ISADPMarker * pi_;
        };
        
    }
}

