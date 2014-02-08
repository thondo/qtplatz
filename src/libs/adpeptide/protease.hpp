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

#ifndef PROTEASE_HPP
#define PROTEASE_HPP

#include "adpeptide_global.hpp"
#include <string>
#include <vector>

namespace adpeptide {

    class ADPEPTIDESHARED_EXPORT protease {
    public:

        protease();

        const std::string& name() const;
        void name( const std::string& ) const;
        const std::vector< std::string >& aliases() const;
        std::vector< std::string >& aliases();
        
    private:
        std::string name_;
        std::vector< std::string > aliases_;

    };

}
#endif // PROTEASE_HPP
