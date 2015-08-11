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

#include "threshold_result.hpp"
#include <ap240/digitizer.hpp>
#include <boost/format.hpp>

using namespace ap240;

threshold_result::threshold_result()
{
}

threshold_result::threshold_result( std::shared_ptr< const waveform > d ) : data_( d )
{
}

threshold_result::threshold_result( const threshold_result& t ) : indecies_( t.indecies_ )
                                                                , data_( t.data_ )
                                                                , processed_( t.processed_ )
{
}

namespace ap240 {
    
    std::ostream& operator << ( std::ostream& os, const threshold_result& t ) {

        os << boost::format("\n%d, %.8lf, ") % t.data_->serialnumber_ % t.data_->meta_.initialXTimeSeconds
           << t.data_->timeSinceEpoch_
           << boost::format(", %.8e, %.8e" ) % t.data_->meta_.scaleFactor % t.data_->meta_.scaleOffset
           << boost::format(", %.8e" ) % t.data_->meta_.initialXOffset;
        
        for ( auto& idx : t.indecies_ ) {
            auto v = (*t.data_)[ idx ];
            os << boost::format(", %.14le, %d" ) % v.first % v.second;
        }
        
        return os;
    }
}
