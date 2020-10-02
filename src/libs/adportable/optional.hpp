/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#if __cplusplus >= 201703L
#include <optional>
#else
#include <boost/optional.hpp>
#endif

// check __cplusplus value with: g++ -x c++ -std=c++14 -dM -E - </dev/null

namespace adportable {

#if __cplusplus >= 201703L
    template< typename T > using optional = std::optional< T >;
    typedef std::nullopt_t nullopt;
#else
    template< typename T > using optional = boost::optional< T >;
    typedef boost::none nullopt; 
#endif

}
