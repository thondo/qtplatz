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

#ifndef MSPEAKS_HPP
#define MSPEAKS_HPP

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class MSPeak;

    class MSPeaks {
    public:
        ~MSPeaks();
        MSPeaks();
        MSPeaks( const MSPeaks& );

        typedef MSPeak value_type;
        typedef std::vector< value_type >::iterator iterator_type;
        typedef std::vector< value_type >::const_iterator const_iterator_type;

        iterator_type begin();
        iterator_type end();
        const_iterator_type begin() const;
        const_iterator_type end() const;
        iterator_type erase( iterator_type );
        iterator_type erase( iterator_type first, iterator_type last );
        MSPeaks& operator << ( const MSPeak& );

    private:
        std::vector< value_type > vec_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            (void)(version)
                ar & vec_;
        }
        
    };

}

#endif // MSPEAKS_HPP
