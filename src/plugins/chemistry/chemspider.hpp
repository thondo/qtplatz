/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <string>
#include <vector>

namespace chemistry {

    class ChemSpider {
    public:
        ChemSpider( const std::string& token );
        ~ChemSpider();

        bool AsyncSimpleSearch( const std::string& stmt );
        bool GetCompoundInfo( int, std::string& smiles, std::string& InChI, std::string& InChIKey );

        const std::string& rid() const;
        const std::vector< int >& csids() const;
        
    private:
        bool getAsyncSearchResult( const std::string& rid );
        std::vector< int > csids_;
        std::string rid_;
        std::string token_;
    };
    
}
