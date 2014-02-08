/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef PEPTIDE_HPP
#define PEPTIDE_HPP

#include "adpeptide_global.hpp"
#include <compiler/disable_dll_interface.h>
#include <string>
#include <vector>

namespace adpeptide {

    class ADPEPTIDESHARED_EXPORT peptide  {
    public:
        peptide();
        peptide( const peptide& t);
        peptide( const std::string& name, const std::string& sequence );

        const std::string& name() const;
        void name( const std::string& );

        const std::string& sequence() const;
        void sequence( const std::string& );

        enum series {
            a_series
            , b_series
            , y_series
        };

        static std::string formula( const std::string& sequence );
        static bool product_ions( std::vector< std::string >& formulae, const std::string& sequence, series );

    private:
        std::string name_;
        std::string sequence_;
    };

}

#endif // PEPTIDE_HPP
