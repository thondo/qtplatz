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
    struct ISADPBaselines;
}

namespace adwidgets {
    namespace ui {

        class Baseline;
        
        class Baselines  {
        public:
            ~Baselines();
            Baselines( SAGRAPHICSLib::ISADPBaselines * pi = 0 );
            Baselines( const Baselines& );
            //

            Baseline operator [] ( int idx );

            size_t size() const;
            
            bool visible() const;
            void visible( bool );
            
            short LineWidth() const;
            void LineWidth( short );
            
            enum LineStyle lineStyle() const;
            void lineStyle( LineStyle );
            
            Baseline add();
            bool remove ( int idx );
            bool clear ();

        private:
            SAGRAPHICSLib::ISADPBaselines * pi_;
        };
        
    }
}

